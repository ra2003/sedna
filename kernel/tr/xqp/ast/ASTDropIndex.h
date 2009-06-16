/*
 * File:  ASTDropIndex.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_DROP_INDEX_H_
#define _AST_DROP_INDEX_H_

#include "ASTNode.h"
#include "AST.h"

#include <string>

class ASTDropIndex : public ASTNode
{
public:
    ASTNode *index;

public:
    ASTDropIndex(ASTLocation &loc, ASTNode *index_) : ASTNode(loc), index(index_) {}

    ~ASTDropIndex();

    void accept(ASTVisitor &v);
    ASTNode *dup();
};

#endif