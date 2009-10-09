/*
 * File:  ASTLet.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTLet.h"

ASTLet::~ASTLet()
{
    delete tv;
    delete expr;
    delete fd;
}

void ASTLet::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

void ASTLet::setNextExpr(ASTNode *fd_)
{
    delete fd;

    fd = fd_;
}

ASTNodesVector *ASTLet::getVarList()
{
    ASTNodesVector *res;

    res = new ASTNodesVector();

    res->push_back(tv->dup());

    return res;
}

ASTNode *ASTLet::dup()
{
    ASTLet *res;

    res = new ASTLet(cd, tv->dup(), expr->dup(), (fd) ? fd->dup() : NULL);

    return res;
}

ASTNode *ASTLet::createNode(scheme_list &sl)
{
    ASTNodeCommonData cd;
    ASTNode *tv = NULL, *expr = NULL, *fd = NULL;
    ASTLet *res;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_LIST && sl[3].type == SCM_LIST && sl[4].type == SCM_LIST);

    cd = dsGetASTCommonFromSList(*sl[1].internal.list);
    tv = dsGetASTFromSchemeList(*sl[2].internal.list);
    expr = dsGetASTFromSchemeList(*sl[3].internal.list);
    fd = dsGetASTFromSchemeList(*sl[4].internal.list);

    res = new ASTLet(cd, tv, expr, fd);

    return res;
}

void ASTLet::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
    if (tv == oldc)
    {
        tv = newc;
        return;
    }
    if (expr == oldc)
    {
        expr = newc;
        return;
    }
    if (fd == oldc)
    {
        fd = newc;
        return;
    }
}
