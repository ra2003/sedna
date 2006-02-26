/*
 * File:  context.h
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */


enum in_status {INSIDE_PREDICATE=0, OUTSIDE_PREDICATE};

enum pos_fns {NOT_EXIST_POS_BASED_FNS=0, EXIST_FN_POSITION, EXIST_FN_LAST};

/* in_context class is responsible for context, which is passed to the grammar
   rules. A in_context class consists of the status field, which indicates
   either recognizing expr located in the predicate or not.
*/
class in_context
{
public:
  enum in_status in_st;
  in_context();
  in_context(enum in_status);
};

/* out_context class is responsible for context, which is generated by the 
   grammar rule and is passed to the one that called this grammar rule. 
   A  out_context class consists of the p_fns field, which defines the existing
   of position based operations in the expression, recognized by the grammar
   rule.
*/
class out_context
{
public:
  enum pos_fns p_fns;
  out_context();
  out_context(enum pos_fns);	
  out_context& operator= (out_context );
  bool operator> (out_context &);
};


out_context &max_out_context(out_context&, out_context&);




