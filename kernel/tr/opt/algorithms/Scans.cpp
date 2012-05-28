#include "Scans.h"

#include "tr/opt/path/XPathLookup.h"

#include "tr/executor/base/XPath.h"
#include "tr/executor/base/XPathOnSchema.h"
#include "tr/executor/base/PPUtils.h"

#include "tr/models/XmlConstructor.h"

#include "ExecutionContext.h"

using namespace phop;
using namespace opt;

OPINFO_DEF(SchemaScan)
OPINFO_DEF(SchemaValueScan)
OPINFO_DEF(BogusConstSequence)
OPINFO_DEF(CachedNestedLoop)
OPINFO_DEF(NestedEvaluation)

SchemaScan::SchemaScan(schema_node_cptr _snode, unsigned int size, unsigned int idx)
    : ITupleOperator(OPINFO_REF, size),
        _cachePtr(_cache.begin()), _idx(idx), snode(_snode), currentBlock(XNULL)
{

}

void SchemaScan::reset()
{
    currentBlock = snode->bblk;
    _cache.clear();
    _cachePtr = _cache.end();
}

void SchemaScan::scan()
{
    if (currentBlock == XNULL) {
        _cache.clear();
        seteos();
    } else {
        NodeBlockHeader hdr(checkp(currentBlock));
        Node node = hdr.getFirstDescriptor();

        _cache.clear();
        _cache.reserve(hdr.getNodeCount());
        while (!node.isNull()) {
            _cache.push_back(node.getPtr());
            node = node.getNext();
        }

        currentBlock = hdr.getNextBlock();
    };

    _cachePtr = _cache.begin();
}

void SchemaScan::do_next()
{
    do {
        if (_cache.empty() || _cachePtr == _cache.end()) {
            scan();
        } else {
            value().cells[_idx] = tuple_cell::node(*(_cachePtr++));
            return;
        }
    } while (_cachePtr != _cache.end());

    seteos();
}

SchemaValueScan::SchemaValueScan(
    schema_node_cptr _snode,
    const TupleCellComparison& _tcmpop,
    MemoryTupleSequencePtr _sequence,
    unsigned int _size, unsigned int _left, unsigned int _right)

    : ITupleOperator(OPINFO_REF, _size),
      currentNode(XNULL), snode(_snode),
      tcmpop(_tcmpop), sequence(_sequence),
      left(_left), right(_right)
{

}


void SchemaValueScan::do_next()
{
    if (currentNode == XNULL) {
        NodeBlockHeader header(checkp(snode->bblk));
        currentNode = header.getFirstDescriptor();
    } else {
        currentNode = NodeIteratorForeward::nextNode(currentNode.getPtr());
    };

    do {
        for (MemoryTupleSequence::const_iterator it = sequence->begin(); it != sequence->end(); ++it) {
            tuple_cell leftNode = tuple_cell::node(currentNode.getPtr());

            if (tcmpop.satisfy(leftNode, *it)) {
                value().cells[left] = leftNode;

                if (right < _tsize()) {
                    value().cells[right] = *it;
                }

                return;
            }
        };

        currentNode = NodeIteratorForeward::nextNode(currentNode.getPtr());
    } while (!currentNode.isNull());

    seteos();
}

void SchemaValueScan::reset()
{
    phop::ITupleOperator::reset();
    currentNode = XNULL;
    tcmpop.handler = block->context->collation;
}

BogusConstSequence::BogusConstSequence(MemoryTupleSequencePtr _sequence, unsigned _size, unsigned _resultIdx)
  : ITupleOperator(OPINFO_REF, _size), sequence(_sequence), resultIdx(_resultIdx), idx(0)
{
  
}

void BogusConstSequence::reset()
{
    phop::ITupleOperator::reset();
    idx = 0;
}


void BogusConstSequence::do_next()
{
    if (idx < sequence->size()) {
        value().cells[resultIdx] = sequence->at(idx++);
    } else {
        seteos();
    }
}

CachedNestedLoop::CachedNestedLoop(unsigned _size, const MappedTupleIn & _left, const MappedTupleIn & _right, const TupleCellComparison & _tcmpop, CachedNestedLoop::flags_t _flags)
  : BinaryTupleOperator(OPINFO_REF, _size, _left, _right),
    tcmpop(_tcmpop), cacheFilled(false), flags(_flags)
{
  
}


void CachedNestedLoop::do_next()
{
    if (!cacheFilled) {
        right.op->next();
        while (!right.eos()) {
            nestedSequenceCache.push_back(right.get());
            right.op->next();
        };

        nestedIdx = nestedSequenceCache.size();
        cacheFilled = true;

        if (nestedSequenceCache.size() == 0) {
            seteos();
        };
    };

    do {
        if (nestedIdx >= nestedSequenceCache.size()) {
            left->next();

            if (left.eos()) {
                seteos();
                return;
            };

            nestedIdx = 0;
        };

        tuple_cell lTuple = left.get();

        while (nestedIdx < nestedSequenceCache.size()) {
            nestedIdx++;

            if (tcmpop.satisfy(lTuple, nestedSequenceCache.at(nestedIdx-1))) {
                left.assignTo(value());
                right.assignTo(value());

                if ((flags & strict_output) == 0) {
                    nestedIdx = nestedSequenceCache.size();
                };
                
                return;
            }
        };
    } while (!left.eos());
}

void CachedNestedLoop::reset()
{
    BinaryTupleOperator::reset();
    cacheFilled = false;
    nestedIdx = nestedSequenceCache.size();
    tcmpop.handler = block->context->collation;
}

NestedEvaluation::NestedEvaluation(const phop::TupleIn& _in, IValueOperator* _op, unsigned int _size, unsigned int _resultIdx)
  : ITupleOperator(OPINFO_REF, _size), in(_in), nestedOperator(_op), resultIdx(_resultIdx)
{
    nestedOperatorIdx = block->operatorMap.at(_op);
}

void NestedEvaluation::do_next()
{
    nestedOperator->next();

    if (nestedOperator->get().is_eos()) {
        seteos();
        return;
    };

    in.copyTo(value());
    value().cells[resultIdx] = nestedOperator->get();
}

void NestedEvaluation::reset()
{
    phop::ITupleOperator::reset();
    nestedOperator = dynamic_cast<IValueOperator *>(block->body.at(nestedOperatorIdx));
    nestedOperator->reset();
}

#include <sstream>
#include "tr/opt/path/XPathLookup.h"

static
std::string schemaPath(schema_node_cptr snode) {
    std::stringstream path;
    std::stack<schema_node_cptr> path_sn;

    while (snode.found()) {
        path_sn.push(snode);
        snode = snode->parent;
    };

    while (!path_sn.empty()) {
        path << path_sn.top()->get_qname().getColonizedName().c_str() << "/";
        path_sn.pop();
    }

    return path.str();
};

XmlConstructor & SchemaScan::__toXML(XmlConstructor & producer) const
{
    producer.addElementValue(PHOPQNAME("path"), schemaPath(snode));
    return  producer;
};

XmlConstructor & SchemaValueScan::__toXML(XmlConstructor & producer) const
{
    producer.addElementValue(PHOPQNAME("path"), schemaPath(snode));

    for (MemoryTupleSequence::const_iterator it = sequence->begin(); it != sequence->end(); ++it) {
        producer.addElementValue(PHOPQNAME("value"), *it);
    };
    
    return producer;
};

XmlConstructor & BogusConstSequence::__toXML(XmlConstructor & producer) const
{
    for (MemoryTupleSequence::const_iterator it = sequence->begin(); it != sequence->end(); ++it) {
        producer.addElementValue(PHOPQNAME("value"), *it);
    };
    
    return  producer;
};

XmlConstructor & CachedNestedLoop::__toXML(XmlConstructor & producer) const
{
    return BinaryTupleOperator::__toXML(producer);
};


XmlConstructor & NestedEvaluation::__toXML(XmlConstructor & producer) const
{
    nestedOperator->toXML(producer);
    in.op->toXML(producer);
    return producer;
};


