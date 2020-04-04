%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <cdk/compiler.h>
#include "ast/all.h"
#define LINE               compiler->scanner()->lineno()
#define yylex()            compiler->scanner()->scan()
#define yyerror(s)         compiler->scanner()->error(s)
#define YYPARSE_PARAM_TYPE std::shared_ptr<cdk::compiler>
#define YYPARSE_PARAM      compiler
//-- don't change *any* of these --- END!
%}

%union {
  int                  i; /* integer value */
  double               d; /* double value */
  std::string          *s; /* symbol name or string literal */
  cdk::basic_node      *node; /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;

  og::block_node       *blk;
};

%token <i> tINT
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tFOR tDO tIF tTHEN tELIF tELSE tWRITE tWRITELN tPUBLIC tREQUIRE
%token tAUTO tINTD tREALD tSTRINGD tPTR tNULLPTR
%token tPROCEDURE tBREAK tCONTINUE tRETURN
%token tINPUT tSIZEOF

%nonassoc tIF
%nonassoc tELIF
%nonassoc tELSE

%right '='
%left tAND tOR
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <node> stmt
%type <sequence> list exprs
%type <expression> expr
%type <lvalue> lval
%type <block> block

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

list : stmt	     { $$ = new cdk::sequence_node(LINE, $1); }
	   | list stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   ;

exprs : expr	     { $$ = new cdk::sequence_node(LINE, $1); }
	    | exprs ',' expr { $$ = new cdk::sequence_node(LINE, $3, $1); }
	    ;

stmt : expr ';'                         { $$ = new og::evaluation_node(LINE, $1); }
 	   | tWRITE expr ';'                  { $$ = new og::write_node(LINE, $2); }
 	   | tWRITELN expr ';'                  { $$ = new og::write_node(LINE, $2, true); }
     | tFOR exprs ';' exprs ';' exprs tDO stmt         { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
     | tIF '(' expr ')' stmt %prec tIFX { $$ = new og::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSE stmt { $$ = new og::if_else_node(LINE, $3, $5, $7); }
     | '{' list '}'                     { $$ = $2; }
     ;

expr : tINT                      { $$ = new cdk::integer_node(LINE, $1); }
	   | tSTRING                 { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2); }
     | tINPUT                    { $$ = new og::input_node(LINE); }
     | expr '+' expr	         { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr	         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr           { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr	         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr	         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | '(' expr ')'            { $$ = $2; }
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); }
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     ;

%%
