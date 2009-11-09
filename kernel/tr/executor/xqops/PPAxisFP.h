/*
* File:  PPAxisFP.h
* Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
*/

#ifndef _PPAXISFP_H
#define _PPAXISFP_H

#include "common/sedna.h"

#include "tr/executor/base/PPBase.h"
#include "tr/executor/base/XPath.h"
#include "tr/crmutils/node_utils.h"
#include "tr/executor/base/xptrChanneledMerge.h"

class PPAxisFP : public PPIterator
{
protected:
    typedef void (PPAxisFP::*t_next_fun)(tuple &t);

    /* given parameters */
    PPOpIn child;
    NodeTestType nt_type;
    NodeTestData nt_data;

    /* obtained parameters and local data */
    xptr cur;
    xptr base;
    bool is_col;
    t_next_fun next_fun;   
    std::map<schema_node_xptr,std::vector<schema_node_xptr> > desc_sch;	
    xptrChanneledMerge* merge_tree;

    void next_qname_and_text(tuple &t,const char* uri,const char* name,t_item type,comp_schema cfun);

    void next_processing_instruction    (tuple &t);
    void next_comment                   (tuple &t);
    void next_text                      (tuple &t);
    void next_node                      (tuple &t);
    void next_qname                     (tuple &t);
    void next_attribute                 (tuple &t);
    void next_document                  (tuple &t);
    void next_wildcard_star             (tuple &t);
    void next_wildcard_ncname_star      (tuple &t);
    void next_wildcard_star_ncname      (tuple &t);

    bool following;

private:
    virtual void do_open   ();
    virtual void do_reopen ();
    virtual void do_close  ();
    virtual void do_next   (tuple &t) {
        (this->*next_fun)(t);
    }

    virtual PPIterator* do_copy(dynamic_context *_cxt_);

public:
    PPAxisFP(dynamic_context *_cxt_,
        operation_info _info_,
        PPOpIn _child_,
        NodeTestType _nt_type_,
        NodeTestData _nt_data_,
        bool _following_);
    virtual ~PPAxisFP();
};

#endif
