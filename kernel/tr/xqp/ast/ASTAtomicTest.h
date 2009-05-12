/*
 * File:  ASTAtomicTest.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_ATOMIC_TEST_H_
#define _AST_ATOMIC_TEST_H_

#include "ASTNode.h"
#include "AST.h"

#include <string>

class ASTAtomicTest : public ASTNode
{
public:
    std::string *name;

public:
    ASTAtomicTest(ASTLocation &loc, std::string *atomic) : ASTNode(loc), name(atomic) {}

    ~ASTAtomicTest();

    void accept(ASTVisitor &v);
    ASTNode *dup();
};

#endif
