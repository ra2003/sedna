/*
 * File:  dm_accessors.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */


#include "dm_accessors.h"
#include "casting_operations.h"
#include "e_string.h"
#include "PPUtils.h"
#include "pstr.h"
#include "d_printf.h"

tuple_cell dm_base_uri(xptr node)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!! this implementation is naive...
    return tuple_cell::eos();
}

tuple_cell dm_node_name(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: return tuple_cell::eos();
		case element		: {
								  const char* p = GETSCHEMENODE(XADDR(node))->xmlns ? 
									              GETSCHEMENODE(XADDR(node))->xmlns->prefix :
								                  NULL;
								  return tuple_cell::atomic_xs_QName_deep(p, GETSCHEMENODE(XADDR(node))->name);
							  }
        case attribute		: {
								  const char* p = GETSCHEMENODE(XADDR(node))->xmlns ? 
									              GETSCHEMENODE(XADDR(node))->xmlns->prefix :
								                  NULL;
								  return tuple_cell::atomic_xs_QName_deep(p, GETSCHEMENODE(XADDR(node))->name);
							  }
        case xml_namespace	: {
                                  ns_dsc *ns = NS_DSC(node);
                                  if (ns->ns->prefix) 
                                      return tuple_cell::atomic_xs_QName_deep(NULL, ns->ns->prefix);
                                  else 
                                      return tuple_cell::eos();
                              }
        case pr_ins			: {
                                  pi_dsc *pi = PI_DSC(node);
                                  shft target = pi->target;
                                  xptr data = pi->data;
							      CHECKP(data);
							      data = PSTRDEREF(data);
                                  char *t = new char[target + 1];
							      t[target] = '\0';
                                  e_str_copy_to_buffer(t, data, target);
                                  return tuple_cell::atomic(xs_QName, t);
                              }
        case comment		: return tuple_cell::eos();
        case text			: return tuple_cell::eos();
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:node-name");
    }
}


xptr get_parent_node(xptr node)
{
    CHECKP(node);

    xptr p = GETPARENTPOINTER(node);

    if (p == NULL) return p;
    else
    {
        CHECKP(p);
        return *(xptr*)XADDR(p);
    }
}

tuple_cell dm_parent(xptr node)
{
    xptr p = get_parent_node(node);
    return (p == NULL) ? tuple_cell::eos() : tuple_cell::node(p);
}

/*******************************************************************************
 * dm:string-value works as follows. The result is a atomic value, which type
 * is xs:string. The value is constructed from content of the node.
 * if node is a text node or a attribute node, then value is the value of a 
 * node. The resulting tuple represents descriptor of the resulting atomic 
 * value. Data remains the same.
 *
 * In the case of element the result value is the concatenation of all
 * descendant _text_ nodes. New data constructed only if concatenation is 
 * needed (when element has several descendant text nodes). 
 ******************************************************************************/

enum dm_string_value_result_type {dsvrt_empty, dsvrt_pstr_both, dsvrt_e_str};

struct dm_string_value_result
{
    dm_string_value_result_type type;
    e_str_buf buf;
    xptr p;
    int size;

    void init()
    {
        type = dsvrt_empty;
        buf.reinit();
        p = XNULL;
        size = 0;
    }
};

static dm_string_value_result dsvr;

void dm_string_value_traverse(xptr node)
{
    CHECKP(node);
    xmlscm_type node_type = GETSCHEMENODE(XADDR(node))->type;

    switch (node_type)
    {
        case element:   {
                            xptr p = first_child(node);

                            while (p != NULL)
                            {
                                dm_string_value_traverse(p);

                                CHECKP(p);
                                p = GETRIGHTPOINTER(p);
                            }

                            return;
                        }
        case text:      {
                            int size = T_DSC(node)->size;
                            xptr data = T_DSC(node)->data;
                            CHECKP(data);
                            data = PSTRDEREF(data);

                            switch (dsvr.type)
                            {
                                case dsvrt_empty    : dsvr.type = dsvrt_pstr_both;
                                                      dsvr.p = data;
                                                      dsvr.size = size;
                                                      break;
                                case dsvrt_pstr_both: dsvr.type = dsvrt_e_str;
                                                      dsvr.buf.append_pstr(dsvr.p, dsvr.size);
                                                      dsvr.buf.append_pstr(data, size);
                                                      break;
                                case dsvrt_e_str    : dsvr.buf.append_pstr(data, size);
                                                      break;
                                default             : throw USER_EXCEPTION2(SE1003, "Unexpected type of dsvr passed to dm_string_value_traverse");
                            }

                         }
    }
}

inline tuple_cell dm_string_value_call_traverse(xptr node)
{
    // it is assumed that CHECKP is already called on node
    dsvr.init();
    dm_string_value_traverse(node);
    switch (dsvr.type)
    {
        case dsvrt_empty    : return EMPTY_STRING_TC;
        case dsvrt_pstr_both: return tuple_cell::atomic_pstr(xs_string, dsvr.size, dsvr.p);
        case dsvrt_e_str    : return dsvr.buf.content();
        default             : throw USER_EXCEPTION2(SE1003, "Unexpected type of dsvr passed to dm:string-value");
    }
}

tuple_cell dm_string_value(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: {
                                  return dm_string_value_call_traverse(node);
                              }
        case element		: {
                                  if (E_DSC(node)->type == xdt_untyped)
                                  {
                                      return dm_string_value_call_traverse(node);
                                  }
                                  else 
                                  {
                                      xptr p = first_child(node);
                                      if (p == NULL) return EMPTY_STRING_TC;
                                      else return dm_string_value(p);
                                  }
                              }
        case attribute		: {
                                  int size = A_DSC(node)->size;
                                  xptr data = A_DSC(node)->data;

                                  if (size == 0) return EMPTY_STRING_TC;

                                  CHECKP(data);
                                  return tuple_cell::atomic_pstr(xs_string, 
                                                                 size, 
                                                                 PSTRDEREF(data));
                              }
        case xml_namespace	: {
                                  ns_dsc *ns = NS_DSC(node);
                                  return tuple_cell::atomic_deep(xs_string, ns->ns->uri);
                              }
        case pr_ins			: {
                                  int size = PI_DSC(node)->size;
                                  xptr data = PI_DSC(node)->data;

                                  if (size == 0) return EMPTY_STRING_TC;

							      CHECKP(data);
                                  return tuple_cell::atomic_pstr(xs_string, 
                                                                 size, 
                                                                 PSTRDEREF(data));
                              }
        case comment		: {
                                  int size = T_DSC(node)->size;
                                  xptr data = T_DSC(node)->data;

                                  if (size == 0) return EMPTY_STRING_TC;

                                  CHECKP(data);
                                  return tuple_cell::atomic_pstr(xs_string, 
                                                                 size, 
                                                                 PSTRDEREF(data));
                              }
        case text			: {
                                  int size = T_DSC(node)->size;
                                  xptr data = T_DSC(node)->data;
                                  CHECKP(data);
                                  return tuple_cell::atomic_pstr(xs_string, 
                                                                 size, 
                                                                 PSTRDEREF(data));
                              }
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:string-value");
    }
}

/*******************************************************************************
 * dm:typed-value works as follows. The result is a atomic value or nothing(?).
 * If the result is value, then s_dsc is constructed. 
 * If kind of a node is element then its type is considered. If element has
 * SimpleType, then the resulting atomic value has the same type, else type
 * is untypedAtomic. Typed value of a attribute node is the same
 * as node has.
 ******************************************************************************/
tuple_cell dm_typed_value(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: return cast(dm_string_value(node), xdt_untypedAtomic);
        case element		: {
                                  xmlscm_type type = E_DSC(node)->type;
                                  if (type == xdt_untyped)
                                  {
                                      tuple_cell res = dm_string_value(node);
                                      res.set_xtype(xdt_untypedAtomic);
                                      return res;
                                  }
                                  else 
                                      return cast(dm_string_value(node), type);
                              }
        case attribute		: {
                                  xmlscm_type type = A_DSC(node)->type;
                                  return cast(dm_string_value(node), type);
                              }
        case xml_namespace	: return dm_string_value(node);
        case pr_ins			: return dm_string_value(node);
        case comment		: return dm_string_value(node);
        case text			: {
                                  int size = T_DSC(node)->size;
                                  xptr data = T_DSC(node)->data;
                                  CHECKP(data);
                                  return tuple_cell::atomic_pstr(xdt_untypedAtomic, 
                                                                 size, 
                                                                 PSTRDEREF(data));
                              }
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:typed-value");
    }
}

char* type2string(xmlscm_type type)
{
    switch (type)
    {
        case xdt_untyped		: return "xdt:untyped";
        case xdt_untypedAtomic	: return "xdt:untypedAtomic";
        case xs_gYearMonth		: return "xs:gYearMonth";
        case xs_gYear			: return "xs:gYear";
        case xs_gMonthDay		: return "xs:gMonthDay";
        case xs_gDay			: return "xs:gDay";
        case xs_gMonth			: return "xs:gMonth";
        case xs_dateTime		: return "xs:dateTime";
        case xs_time			: return "xs:time";
        case xs_date			: return "xs:date";
        case xs_duration		: return "xs:duration";
        case xs_boolean			: return "xs:boolean";
        case xs_base64Binary	: return "xs:base64Binary";
        case xs_hexBinary		: return "xs:hexBinary";
        case xs_float			: return "xs:float";
        case xs_double			: return "xs:double";
        case xs_anyURI			: return "xs:anyURI";
        case xs_QName			: return "xs:QName";
        case xs_NOTATION		: return "xs:NOTATION";
        case xs_string			: return "xs:string";
        case xs_decimal			: return "xs:decimal";
        case xs_integer			: return "xs:integer";
        case xs_anyType			: return "xs:anyType";
        default					: throw USER_EXCEPTION2(SE1003, "Unexpected XML Schema type passed to dm:type"); 
    }
}

tuple_cell dm_type_name(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: return tuple_cell::eos();
        case element		: return tuple_cell::atomic_deep(xs_QName, type2string(E_DSC(node)->type));
        case attribute		: return tuple_cell::atomic_deep(xs_QName, type2string(A_DSC(node)->type));
        case xml_namespace	: return tuple_cell::eos();
        case pr_ins			: return tuple_cell::eos();
        case comment		: return tuple_cell::eos();
        case text			: return tuple_cell::atomic_xs_QName_deep("xdt", "untypedAtomic");
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:type-name");
    }
}

tuple_cell dm_nilled(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: return tuple_cell::eos();
        case element		: return tuple_cell::atomic(false);
        case attribute		: return tuple_cell::eos();
        case xml_namespace	: return tuple_cell::eos();
        case pr_ins			: return tuple_cell::eos();
        case comment		: return tuple_cell::eos();
        case text			: return tuple_cell::eos();
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:nilled");
    }
}

tuple_cell dm_document_uri(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case document		: {
                                  d_dsc *d = D_DSC(node);
							      int size = d->size;
							      xptr data = d->data;
							      CHECKP(data);
							      data = PSTRDEREF(data);
                                  char *t = new char[size + 1];
							      t[size] = '\0';
                                  e_str_copy_to_buffer(t, data, size);
                                  return tuple_cell::atomic(xs_anyURI, t);
                              }
        case element		: return tuple_cell::eos();
        case attribute		: return tuple_cell::eos();
        case xml_namespace	: return tuple_cell::eos();
        case pr_ins			: return tuple_cell::eos();
        case comment		: return tuple_cell::eos();
        case text			: return tuple_cell::eos();
        default				: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:nilled");
    }
}

dm_node_kind_type dm_node_kind(xptr node)
{
    CHECKP(node);

    switch (GETSCHEMENODE(XADDR(node))->type)
    {
        case element	: return nk_element;
        case text		: return nk_text;
        case attribute	: return nk_attribute;
        case document	: return nk_document;
        default			: throw USER_EXCEPTION2(SE1003, "Unexpected type of node passed to dm:node-kind");
    }
}

