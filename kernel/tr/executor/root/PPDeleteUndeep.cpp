/*
 * File:  PPDeleteUndeep.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "sedna.h"

#include "PPDeleteUndeep.h"
#include "updates.h"
#include "locks.h"

PPDeleteUndeep::PPDeleteUndeep(PPOpIn _child_, 
                               dynamic_context *_cxt_) : PPUpdate(),
                                                         child(_child_),
                                                         cxt(_cxt_)
{
}

PPDeleteUndeep::~PPDeleteUndeep()
{
    delete child.op;
    child.op = NULL;
    delete cxt;
    cxt = NULL;
}

void PPDeleteUndeep::open()
{
    local_lock_mrg->lock(lm_x);
    dynamic_context::global_variables_open();
    child.op->open();
}

void PPDeleteUndeep::close()
{
    child.op->close();
    dynamic_context::global_variables_close();
}

void PPDeleteUndeep::execute()
{
    delete_undeep(child);
}

