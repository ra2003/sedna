/*
 * File:  ASTUpdReplace.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTUpdReplace.h"

ASTUpdReplace::~ASTUpdReplace()
{
    delete what;
    delete new_expr;
}

void ASTUpdReplace::accept(ASTVisitor &v)
{
    v.visit(*this);
}

ASTNode *ASTUpdReplace::dup()
{
    return new ASTUpdReplace(loc, what->dup(), static_cast<ASTFunDef *>(new_expr->dup()));
}
