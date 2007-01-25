/*
 * File:  PPQueryRoot.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */


#include "sedna.h"

#include "PPQueryRoot.h"
#include "crmutils.h"
#include "locks.h"


PPQueryRoot::PPQueryRoot(dynamic_context *_cxt_,
                         PPOpIn _child_,
                         se_ostream& _s_,
                         t_print _print_mode_) : PPQueryEssence(),
                                                 cxt(_cxt_),
                                                 child(_child_),
                                                 s(_s_),
                                                 print_mode(_print_mode_),
                                                 data(_child_.ts)
{
}

PPQueryRoot::~PPQueryRoot()
{
//    d_printf1("PPQueryRoot::~PPQueryRoot() begin\n");
    delete child.op;
    child.op = NULL;
    delete cxt;
    cxt = NULL;
//    d_printf1("PPQueryRoot::~PPQueryRoot() end\n");
}

void PPQueryRoot::open()
{
//    d_printf1("PPQueryRoot::open() begin\n");
    local_lock_mrg->lock(lm_s);
	first=true;
    dynamic_context::global_variables_open();
    child.op->open();
	if (print_mode==sxml)
	{
		dynamic_context::add_char_mapping("<","<",-1);
		dynamic_context::add_char_mapping(">",">",-1);
		dynamic_context::add_char_mapping("&","&",-1);
		dynamic_context::add_char_mapping("\"","\\\"",-1);		
		dynamic_context::add_char_mapping("\\","\\\\",-1);
	}
//    d_printf1("PPQueryRoot::open() end\n");
}

void PPQueryRoot::close()
{
    child.op->close();
    dynamic_context::global_variables_close();
}

bool PPQueryRoot::next()
{
    child.op->next(data);

    if (data.is_eos()) return false;

    switch (cxt->st_cxt->output_indent)
    {
        case se_output_indent_yes: print_tuple_indent(data, dynamic_context::ostr(), print_mode, first, cxt); break;
        case se_output_indent_no : print_tuple(data, dynamic_context::ostr(), print_mode, cxt); break;
        default                  : throw USER_EXCEPTION2(SE1003, "Unexpected se_output_indent");
    }
	
	if (first)
		first = false;

    return true;
}

void PPQueryRoot::execute()
{
    while (next());
}
