#include "DataGraphs.h"

#include "tr/opt/algebra/IndependentPlan.h"
#include "tr/models/XmlConstructor.h"

#include <algorithm>

using namespace opt;

DataGraph::DataGraph(VariableUsageGraph* _owner) : owner(_owner)
{
    U_ASSERT(_owner != NULL);
    memset(predicates, 0, sizeof(predicates[0]) * MAX_GRAPH_SIZE);
    memset(dataNodes, 0, sizeof(dataNodes[0]) * MAX_GRAPH_SIZE);
}

void DataGraphIndex::update()
{
    nodes.clear();
    out.clear();
    in.clear();
    predicates.clear();

    predicateMask = 0;

    memset(nodeIndex, 0, sizeof(nodeIndex[0]) * MAX_GRAPH_SIZE);
    memset(predicateIndex, 0, sizeof(predicateIndex[0]) * MAX_GRAPH_SIZE);

    unsigned aI = 0;

    FOR_ALL_GRAPH_ELEMENTS(dg->dataNodes, i) {
        DataNode * n = dg->dataNodes[i];
        DataNodeIndex * nidx = nodeIndex + i;

        nidx->output = (n->indexBit & dg->outputNodes) > 0;
        nidx->input = (n->indexBit & dg->inputNodes) > 0;

        n->absoluteIndex = aI++;
        nidx->p = n;
        nidx->predicates = 0;

        nodes.push_back(n);

        if ((n->indexBit & dg->outputNodes) > 0) {
            out.push_back(dg->dataNodes[i]);
        };

        if ((n->indexBit & dg->inputNodes) > 0) {
            in.push_back(n);
            U_ASSERT(n->varTupleId != invalidTupleId);
        };
    };

    FOR_ALL_GRAPH_ELEMENTS(dg->predicates, i) {
        Predicate * p = dg->predicates[i];
        PredicateIndex * pidx = predicateIndex + i;

        pidx->p = p;
        pidx->dataNodeMask = 0;

        for (DataNodeList::const_iterator it = p->dataNodeList.begin(); it != p->dataNodeList.end(); ++it) {
            pidx->dataNodeMask |= (*it)->indexBit;
            nodeIndex[(*it)->index].predicates |= p->indexBit;
        }

        pidx->evaluateAfter = 0;
        for (PredicateList::const_iterator it = p->evaluateAfter.begin(); it != p->evaluateAfter.end(); ++it) {
            pidx->evaluateAfter |= (*it)->indexBit;
        }

        pidx->implies = 0;
        for (PredicateList::const_iterator it = p->implies.begin(); it != p->implies.end(); ++it) {
            pidx->implies |= (*it)->indexBit;
        }
        
        pidx->neighbours = 0;

        predicateMask |= p->indexBit;
        predicates.push_back(p);
    };

    FOR_ALL_GRAPH_ELEMENTS(dg->predicates, i) {
        Predicate * p = dg->predicates[i];
        PredicateIndex * pidx = predicateIndex + i;

        for (DataNodeList::const_iterator jt = p->dataNodeList.begin(); jt != p->dataNodeList.end(); ++jt) {
            pidx->neighbours |= nodeIndex[(*jt)->index].predicates;
        }

        pidx->neighbours &= ~p->indexBit;
    };
    
    std::sort(nodes.begin(), nodes.end());
    std::sort(out.begin(), out.end());
    std::sort(in.begin(), in.end());
    std::sort(predicates.begin(), predicates.end());
}

void DataGraphIndex::tuplesInOut(TupleScheme* in, TupleScheme* out)
{
    FOR_ALL_GRAPH_ELEMENTS(dg->dataNodes, i) {
        DataNode * n = dg->dataNodes[i];

        if (n->varTupleId != invalidTupleId) {
            if (out != NULL &&
              (n->indexBit & dg->outputNodes) > 0) {
                out->insert(n->varTupleId);
            }
            
            if (in != NULL &&
              (n->indexBit & dg->inputNodes) > 0) {
                in->insert(n->varTupleId);
            }
        };
    };
    
}

DataGraphIndex::DataGraphIndex(DataGraph* _dg)
    : dg(_dg), predicateMask(0)
{
    nodes.reserve(MAX_GRAPH_SIZE / 2);
    in.reserve(MAX_GRAPH_SIZE / 2);
    out.reserve(MAX_GRAPH_SIZE / 2);
    predicates.reserve(MAX_GRAPH_SIZE / 2);

    update();
}

void DataGraphIndex::rebuild()
{
    make(dg->owner, dg);
    update();
}

DataGraph* DataGraphIndex::make(VariableUsageGraph* master, DataGraph* graph)
{
    PlanDesc lastIndex = 0;

    U_ASSERT(nodes.size() + predicates.size() <= MAX_GRAPH_SIZE);

    memset(graph->dataNodes, 0, sizeof(graph->dataNodes[0]) * MAX_GRAPH_SIZE);
    memset(graph->predicates, 0, sizeof(graph->predicates[0]) * MAX_GRAPH_SIZE);

    graph->inputNodes = 0;
    for (DataNodeList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        DataNode * n = (*it);
        n->setIndex(lastIndex);
        n->parent = graph;
        graph->dataNodes[lastIndex] = n;

        if (n->type == opt::DataNode::dnExternal) {
            graph->inputNodes |= n->indexBit;
        }
/*
        if (n->varTupleId != invalidTupleId) {
            master->addVariable(n);
        }
*/
        lastIndex++;
    };

    graph->outputNodes = 0;
    for (DataNodeList::const_iterator it = out.begin(); it != out.end(); ++it)
    {
        DataNode * n = (*it);
        U_ASSERT(n->varTupleId != invalidTupleId);
        graph->outputNodes |= n->indexBit;
    };

    for (PredicateList::const_iterator it = predicates.begin(); it != predicates.end(); ++it) {
        (*it)->setIndex(lastIndex);
        graph->predicates[lastIndex] = (*it);
        lastIndex++;
    };

    return graph;
}

XmlConstructor & DataGraph::toXML(XmlConstructor& constructor) const
{
    constructor.openElement(SE_EL_NAME("datagraph"));

    FOR_ALL_GRAPH_ELEMENTS(dataNodes, i)
    {
        dataNodes[i]->toXML(constructor);
    };

    FOR_ALL_GRAPH_ELEMENTS(predicates, i)
    {
        predicates[i]->toXML(constructor);
    };

    constructor.closeElement();

    return constructor;
}

XmlConstructor & DataNode::toXML(XmlConstructor& element) const
{
    const char * nodetype = NULL;

    switch(type) {
        case dnDatabase : nodetype = "root"; break;
        case dnFreeNode : nodetype = "free"; break;
        case dnConst : nodetype = "const"; break;
        case dnExternal : nodetype = "ext"; break;
        case dnAlias : nodetype = "alias"; break;
        case dnReplaced : nodetype = "REMOVED"; break;
    };

    element.openElement(SE_EL_NAME("node"));

    element.addAttributeValue(SE_EL_NAME("type"), nodetype);
    element.addAttributeValue(SE_EL_NAME("index"), tuple_cell::atomic_int(index));

    if (parent == NULL) {
        element.addAttributeValue(SE_EL_NAME("detached"), tuple_cell::atomic(true));
        element.closeElement();
        return element;
    }

    if (varTupleId != opt::invalidTupleId) {
        element.addAttributeValue(SE_EL_NAME("varId"), tuple_cell::atomic_int(varTupleId));

        if (parent->owner->variableMap.find(varTupleId) == parent->owner->variableMap.end()) {
            element.addAttributeValue(SE_EL_NAME("bad"), tuple_cell::atomic(true));
        } else {
            TupleInfo & vinfo = parent->owner->getVariable(varTupleId);

            if (!vinfo.name.empty()) {
                element.addAttributeValue(SE_EL_NAME("varName"), vinfo.name);
            };
        }
    };

    if (parent != NULL && parent->operation != NULL) {
        element.addAttributeValue(SE_EL_NAME("op-id"), tuple_cell::atomic_int(parent->operation->oid()));
    };

    if ((parent->outputNodes & this->indexBit) > 0) {
        element.addAttributeValue(SE_EL_NAME("output"), tuple_cell::atomic(true));
    };

    switch(type) {
        case dnDatabase :
            element.addElementValue(SE_EL_NAME("root"), root.toLRString());
            element.addElementValue(SE_EL_NAME("path"), path.toXPathString());
            break;
        case dnAlias :
            element.addElementValue(SE_EL_NAME("source"), tuple_cell::atomic_int(aliasFor->index));
            break;
        case dnReplaced :
            if (NULL != replacedWith) {
                element.addElementValue(SE_EL_NAME("with"), tuple_cell::atomic_int(replacedWith->index));
            }
            break;
        case dnConst :
            U_ASSERT(!constValue.isnull());
            for (MemoryTupleSequence::const_iterator it = constValue->begin(); it != constValue->end(); ++it) {
                element.addElementValue(SE_EL_NAME("value"), *it);
            };
            break;
        case dnFreeNode :
            break;
        case dnExternal :
            break;
    };

    element.closeElement();

    return element;
}


