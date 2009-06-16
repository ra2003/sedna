/*
 * File:  ASTEmptyTest.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_EMPTY_TEST_H_
#define _AST_EMPTY_TEST_H_

#include "ASTNode.h"
#include "AST.h"

class ASTEmptyTest : public ASTNode
{
public:
    ASTEmptyTest(ASTLocation &loc) : ASTNode(loc) {}

    ~ASTEmptyTest() {}

    void accept(ASTVisitor &v);
    ASTNode *dup();
};

#endif