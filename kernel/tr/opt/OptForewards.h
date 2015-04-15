#ifndef _OPT_FOREWARDS_H_
#define _OPT_FOREWARDS_H_

/* Forward declarations for many important types */

#define MAX_GRAPH_SIZE 63
#define MEMORY_BLOCK_SIZE 0x0100000UL

class DataRoot;
class XmlConstructor;

struct StaticContext;

namespace rqp {
    class RPBase;
    class MapGraph;
    class PlanContext;
    class FunCallParams;
    class VarIn;

    struct RewritingContext;
}

namespace pe {
    class Step;
    class Path;
};

namespace phop {
/* Physical operations for cost based execution */
    struct FunctionInfo;

    class ITupleOperator;
    class IOperator;
    class IFunction;

    class GraphExecutionBlock;
}

namespace executor
{
    class IExecuteProc;

    struct DynamicContext;

    struct ResultSequence;
    struct VirtualSequence;
    struct UpdateSequence;

    struct VariableProducer;
    struct VarCacheInfo;
    struct VariableModel;
};

namespace opt
{
    class GraphCompiler;

/* Data graph */
    struct VariableUsageGraph;

    struct DataNode;
    struct Variable;
    struct DataGraph;
    struct DataGraphIndex;

    struct Predicate;
    struct ValuePredicate;
    struct StructuralPredicate;
    
/* Plan cost-based optimization */    
    class PhysicalModel;
    class PlanInfo;
    class TupleRef;

/* Cost model */
    class CostModel;
    struct OperationCost;
    struct Statistics;
    struct TupleStatistics;
    
/* Operation prototypes */
    class POProt;
    struct ComparisonPrototype;
};


#endif /* _OPT_FOREWARDS_H_ */