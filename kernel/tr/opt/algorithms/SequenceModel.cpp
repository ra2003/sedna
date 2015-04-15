#include "SequenceModel.h"

#include "tr/opt/graphs/DataGraphs.h"
#include "tr/models/XmlConstructor.h"

using namespace phop;

std::stack<GraphExecutionBlock * > GraphExecutionBlock::blockBuildingStack;

RTTI_DEF_BASE(IOperator)
RTTI_DEF(ITupleOperator)
RTTI_DEF(BinaryTupleOperator)
RTTI_DEF(UnaryTupleOperator)
RTTI_DEF(ItemOperator)

void GraphExecutionBlock::prepare(const opt::DataGraphIndex* dgi)
{
    U_ASSERT(sourceStack.empty());
    operatorMap.clear();

    if (dgi != NULL) {
        TupleIdMap realMap;

        for (opt::DataNodeList::const_iterator it = dgi->out.begin(); it != dgi->out.end(); ++it)
        {
            opt::DataNode * node = *it;
            if (node->varTupleId != opt::invalidTupleId)
            {
                realMap[node->varTupleId] = resultMap[node->absoluteIndex];
            };
        };

        resultMap = realMap;
    };

    previous = new tuple(top()->_tsize());
}

void BinaryTupleOperator::reset()
{
    ITupleOperator::reset();

    left.op = dynamic_cast<ITupleOperator *>(block->body[leftIdx]);
    right.op = dynamic_cast<ITupleOperator *>(block->body[rightIdx]);

    left.op->reset();
    right.op->reset();
}

void UnaryTupleOperator::reset()
{
    phop::ITupleOperator::reset();
    in.op = dynamic_cast<ITupleOperator *>(block->body[inIdx]);
    in.op->reset();
}

void ItemOperator::reset()
{
    phop::ITupleOperator::reset();
}


IOperator::IOperator(clsinfo_t _opinfo) : ObjectBase(_opinfo), block(NULL)
{
    block = GraphExecutionBlock::current();
    block->body.push_back(this);
    block->operatorMap.insert(OperatorMap::value_type(this, block->body.size()-1));
}

XmlConstructor & IOperator::toXML(XmlConstructor & element) const
{
    element.openElement(SE_EL_NAME(info()->name));
    __toXML(element);
    element.closeElement();
    return element;
}

BinaryTupleOperator::BinaryTupleOperator(clsinfo_t _opinfo, unsigned _size, const MappedTupleIn & _left, const MappedTupleIn & _right)
    : ITupleOperator(_opinfo, _size), left(_left), right(_right)
{
    leftIdx = block->operatorMap.at(_left.op);
    rightIdx = block->operatorMap.at(_right.op);
};

UnaryTupleOperator::UnaryTupleOperator(clsinfo_t _opinfo, unsigned int _size, const phop::MappedTupleIn& _in)
    : ITupleOperator(_opinfo, _size), in(_in)
{
    inIdx = block->operatorMap.at(_in.op);
}

XmlConstructor & BinaryTupleOperator::__toXML(XmlConstructor & element ) const
{
    left.op->toXML(element);
    right.op->toXML(element);
    return element;
}

XmlConstructor & UnaryTupleOperator::__toXML(XmlConstructor & element ) const
{
    in.op->toXML(element);
    return element;
}

XmlConstructor & ItemOperator::__toXML(XmlConstructor & element ) const
{
    in->toXML(element);
    return element;
}