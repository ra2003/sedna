/*
 * File:  ASTPi.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_PI_H_
#define _AST_PI_H_

#include "ASTNode.h"
class ASTVisitor;

#include <string>

class ASTPi : public ASTNode
{
public:
    std::string *name; // pi name
    std::string *cont; // pi content

    bool deep_copy; // pi will be attached to virtual_root and copied on demand

public:
    ASTPi(const ASTNodeCommonData &loc, std::string *name_, std::string *cont_) : ASTNode(loc), name(name_), cont(cont_) {}

    ~ASTPi();

    void accept(ASTVisitor &v);
    ASTNode *dup();
    void modifyChild(const ASTNode *oldc, ASTNode *newc);

    static ASTNode *createNode(scheme_list &sl);
};

#endif
