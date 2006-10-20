/**/

class XQueryParser

{

queryProlog!:
	<<ASTBase *prolog=NULL;>>
	( ( fd:functionDefn
	    <<if(prolog == NULL) prolog=#fd; else prolog->append(#fd);>>

	   | nd:namespaceDecl
	     <<if(prolog == NULL) prolog=#nd; else prolog->append(#nd);>>

	   | dds:defaultDecls
	     <<if(prolog == NULL) prolog=#dds; else prolog->append(#dds);>>

	   | bsd:boundarySpaceDecl
	     <<if(prolog == NULL) prolog=#bsd; else prolog->append(#bsd);>>



	   | dopt:declareOption
	     <<if(prolog == NULL) prolog=#dopt; else prolog->append(#dopt);>>

	   | imp:importModule <<if(prolog == NULL) prolog=#imp; else prolog->append(#imp);>>

	  ) SEMICOLON
	)*

	<<#0=prolog;>>

;


functionDefn!: 
	<<ASTBase* ret_type = NULL;
	  bool body = true;
	>>
	DECLARE FUNCTION qn:qname LPAR {pl:paramList} RPAR {AS st:sequenceType}
    <<if (#st == NULL) ret_type = #(#[AST_TYPE], #[AST_ITEM_TEST], #["zero-or-more", AST_MULTIPLICITY]);
      else ret_type = #st;
	>>
	( e:enclosedExpr | EXTERNAL <<body = false;>> )
	  <<
	    if (body)
	       #0=#(#[AST_FUNCTION], 
	            #qn,
	            #(#[AST_FPARAMS], #pl),
	            #(#[AST_RETURNED_TYPE], ret_type),
	            #(#[AST_BODY_FUNC], #e));
	    else
	       #0=#(#[AST_FUNCTION], 
	            #qn,
	            #(#[AST_FPARAMS], #pl),
	            #(#[AST_RETURNED_TYPE], ret_type),
	            #(#[AST_BODY_FUNC]));

	  >>
;

paramList!:
	p1:param <<#0=#p1;>>
	(COMMA p2:param <<#0->append(#p2);>> )*
;

param!:
	vr:varRef {td:typeDeclaration}

	<<if (#td == NULL) #0=#(#[AST_PARAM], #vr, #(#[AST_TYPE], #[AST_ITEM_TEST], #["zero-or-more", AST_MULTIPLICITY]));
      else #0=#(#[AST_PARAM], #vr, #td);
	>>
;


namespaceDecl!:
	DECLARE NAMESPACE nc:ncname EQUAL s:STRINGLITERAL
	<<#0=#(#[AST_DECL_NSP], 
	     #[#nc->getText(), AST_PREFIX],
	     #[$s->getText(), AST_STRING_CONST]);
	>>
;


defaultDecls!:
	DECLARE DEFAULT 
	 (ELEMENT NAMESPACE s:STRINGLITERAL
	  <<#0=#(#[AST_DECL_DEF_ELEM_NSP], 
	         #[$s->getText(), AST_STRING_CONST]);
	  >>
	 |
	  FUNCTION NAMESPACE s2:STRINGLITERAL
	  <<#0=#(#[AST_DECL_DEF_FUNC_NSP], 
	         #[$s2->getText(), AST_STRING_CONST]);
	  >>
	 | ORDER EMPTY 
	     (   GREATEST  <<#0=#[AST_DEF_ORDER_EG];>>

	      | LEAST <<#0=#[AST_DEF_ORDER_EL];>>
	     )


	 )

;

boundarySpaceDecl!:
	DECLARE BOUNDARYSPACE (PRESERVE <<#0=#[AST_BSPACE_P]; is_preserve_boundary_space = true;>>| STRIP<<#0=#[AST_BSPACE_S]; is_preserve_boundary_space = false;>>)

;

declareOption!:
	DECLARE OPTION qn:qname s:STRINGLITERAL
	<<#0=#(#[AST_DECLARE_OPT], #qn, #[$s->getText(), AST_STRING_CONST]);>>
;
/*
import!:
	importModule
;
*/
importModule!:
	IMPORT {MODULE NAMESPACE ncname EQUAL}   STRINGLITERAL {AT_ STRINGLITERAL  (COMMA  STRINGLITERAL)*}
	<<throw USER_EXCEPTION(XQST0016 );>>
;

}