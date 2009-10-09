/*
 * File:  ASTLoadModule.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTLoadModule.h"

ASTLoadModule::~ASTLoadModule()
{
    destroyASTStringVector(modules);
}

void ASTLoadModule::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

ASTNode *ASTLoadModule::dup()
{
    return new ASTLoadModule(cd, duplicateASTStringVector(modules), mod);
}

ASTNode *ASTLoadModule::createNode(scheme_list &sl)
{
    ASTNodeCommonData cd;
    ASTStringVector *modules = NULL;
    LoadMod type;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_LIST && sl[3].type == SCM_NUMBER);

    cd = dsGetASTCommonFromSList(*sl[1].internal.list);
    modules = dsGetASTStringsFromSList(*sl[2].internal.list);
    type = LoadMod(atol(sl[3].internal.num));

    return new ASTLoadModule(cd, modules, type);
}

void ASTLoadModule::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
}
