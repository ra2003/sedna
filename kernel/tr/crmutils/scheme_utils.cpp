#include "scheme_utils.h"
#include "vmm.h"
#include "node_utils.h"

xptr sch_right_sibling(xptr& node)
{
	CHECKP(node);
	return ((n_dsc*)XADDR(node))->rdsc;
 
}
xptr sch_left_sibling(xptr& node)
{
	CHECKP(node);
	return ((n_dsc*)XADDR(node))->ldsc;
}
xptr sch_child_dm(xptr& node)
{
	return getFirstByOrderChildNode(node);
}
t_item sch_kind(xptr& node)
{
	CHECKP(node);
	return (GETBLOCKBYNODE(node))->snode->type;
}
char* sch_name(xptr& node)
{
	CHECKP(node);
	return (GETBLOCKBYNODE(node))->snode->name;
}

