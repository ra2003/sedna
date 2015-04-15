#include "FunctionOperations.h"

#include "tr/opt/functions/Functions.h"

using namespace rqp;
using namespace opt;

RTTI_DEF(Construct)
RTTI_DEF(FunCallParams)

void FunCallParams::evaluateTo(executor::DynamicContext* dynamicContext)
{
    function->execute(this, dynamicContext);
}

XmlConstructor& FunCallParams::__toXML(XmlConstructor& constructor) const
{
    constructor.addAttributeValue(SE_EL_NAME("name"), function->getQName().getColonizedName());

    int i = 0;
    for (ParamList::const_iterator it = paramList.begin(); it != paramList.end(); ++it)
    {
        constructor.openElement(SE_EL_NAME("param"));
        constructor.addAttributeValue(SE_EL_NAME("index"), tuple_cell::atomic_int(i++));
        constructor.addAttributeValue(SE_EL_NAME("varId"), tuple_cell::atomic_int(*it));
        constructor.closeElement();
    };

    if (function_data != NULL) {
        function_data->toXML(constructor);
    };

    return constructor;
}

XmlConstructor& Construct::__toXML(XmlConstructor& element) const
{
    element.addElementValue(SE_EL_NAME("type"), type2string(type));

    if (getName() != null_obj) {
        element.openElement(SE_EL_NAME("name"));
        getName()->toXML(element);
        element.closeElement();
    }

    return rqp::ListOperation::__toXML(element);
};
