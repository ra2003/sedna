/*
 * File:  ASTMetaSchemaCol.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTMetaSchemaCol.h"

ASTMetaSchemaCol::~ASTMetaSchemaCol()
{
    delete coll;
}

void ASTMetaSchemaCol::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

ASTNode *ASTMetaSchemaCol::dup()
{
    return new ASTMetaSchemaCol(loc, coll->dup());
}

ASTNode *ASTMetaSchemaCol::createNode(scheme_list &sl)
{
    ASTLocation loc;
    ASTNode *col;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_LIST);

    loc = dsGetASTLocationFromSList(*sl[1].internal.list);
    col = dsGetASTFromSchemeList(*sl[2].internal.list);

    return new ASTMetaSchemaCol(loc, col);
}

void ASTMetaSchemaCol::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
    if (coll == oldc)
    {
        coll = newc;
        return;
    }
}
