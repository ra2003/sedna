/*
 * File:  ASTGrantPriv.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTGrantPriv.h"

ASTGrantPriv::~ASTGrantPriv()
{
    delete priv;
    delete obj;
    delete user;
}

void ASTGrantPriv::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

ASTNode *ASTGrantPriv::dup()
{
    return new ASTGrantPriv(loc, new std::string(*priv), (obj) ? new std::string(*obj) : NULL, new std::string(*user), mod);
}

ASTNode *ASTGrantPriv::createNode(scheme_list &sl)
{
    ASTLocation loc;
    std::string *user = NULL, *priv = NULL, *obj = NULL;
    ObjMod mod;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_NUMBER && sl[3].type == SCM_STRING && sl[4].type == SCM_STRING);

    loc = dsGetASTLocationFromSList(*sl[1].internal.list);
    mod = ObjMod(atol(sl[2].internal.num));

    user = new std::string(sl[3].internal.str);
    priv = new std::string(sl[4].internal.str);

    if (sl.size() > 5)
    {
        U_ASSERT(sl[5].type == SCM_STRING);
        obj = new std::string(sl[5].internal.str);
    }

    return new ASTGrantPriv(loc, priv, obj, user, mod);
}

void ASTGrantPriv::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
}
