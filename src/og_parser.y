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
%token <d> tREAL
%token <s> tIDENTIFIER tSTRING
%token tFOR tDO tTHEN tWRITE tWRITELN tPUBLIC tREQUIRE
%token tAUTO tINTD tREALD tSTRINGD tPTR tNULLPTR
%token tPROCEDURE tBREAK tCONTINUE tRETURN
%token tINPUT tSIZEOF

%nonassoc tIFX
%nonassoc tIF
%nonassoc tELIF
%nonassoc tELSE

%right '='
%left tAND tOR
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <node> instr
%type <sequence> instrs exprs
%type <expression> expr
%type <lvalue> lval
%type <block> block

%%

file : decls        { $$ = decl.value}
     ;

decls :       decl
      | decls decl


decl : var ';'
     | function
     | procedure
     ;

var :          tTYPE tIDENTIFIER
    |          tTYPE tIDENTIFIER '=' expr
    | tPUBLIC  tTYPE tIDENTIFIER
    | tPUBLIC  tTYPE tIDENTIFIER '=' expr
    | tREQUIRE tTYPE tIDENTIFIER
    | tREQUIRE tTYPE tIDENTIFIER '=' expr
    |          tAUTO identifiers '=' exprs
    | tPUBLIC  tAUTO identifiers '=' exprs
    ;

fun :          tTYPE tIDENTIFIER '('      ')'
    |          tTYPE tIDENTIFIER '('      ')' blk
    |          tTYPE tIDENTIFIER '(' vars ')'
    |          tTYPE tIDENTIFIER '(' vars ')' blk
    |          tAUTO tIDENTIFIER '('      ')'
    |          tAUTO tIDENTIFIER '('      ')' blk
    |          tAUTO tIDENTIFIER '(' vars ')'
    |          tAUTO tIDENTIFIER '(' vars ')' blk
    | tPUBLIC  tTYPE tIDENTIFIER '('      ')'
    | tPUBLIC  tTYPE tIDENTIFIER '('      ')' blk
    | tPUBLIC  tTYPE tIDENTIFIER '(' vars ')'
    | tPUBLIC  tTYPE tIDENTIFIER '(' vars ')' blk
    | tPUBLIC  tAUTO tIDENTIFIER '('      ')'
    | tPUBLIC  tAUTO tIDENTIFIER '('      ')' blk
    | tPUBLIC  tAUTO tIDENTIFIER '(' vars ')'
    | tPUBLIC  tAUTO tIDENTIFIER '(' vars ')' blk
    | tREQUIRE tTYPE tIDENTIFIER '('      ')'
    | tREQUIRE tTYPE tIDENTIFIER '('      ')' blk
    | tREQUIRE tTYPE tIDENTIFIER '(' vars ')'
    | tREQUIRE tTYPE tIDENTIFIER '(' vars ')' blk
    | tREQUIRE tAUTO tIDENTIFIER '('      ')'
    | tREQUIRE tAUTO tIDENTIFIER '('      ')' blk
    | tREQUIRE tAUTO tIDENTIFIER '(' vars ')'
    | tREQUIRE tAUTO tIDENTIFIER '(' vars ')' blk
    ;


proc :          tPROCEDURE tIDENTIFIER '('      ')'
     |          tPROCEDURE tIDENTIFIER '('      ')' blk
     |          tPROCEDURE tIDENTIFIER '(' vars ')'
     |          tPROCEDURE tIDENTIFIER '(' vars ')' blk
     | tPUBLIC  tPROCEDURE tIDENTIFIER '('      ')'
     | tPUBLIC  tPROCEDURE tIDENTIFIER '('      ')' blk
     | tPUBLIC  tPROCEDURE tIDENTIFIER '(' vars ')'
     | tPUBLIC  tPROCEDURE tIDENTIFIER '(' vars ')' blk
     | tREQUIRE tPROCEDURE tIDENTIFIER '('      ')'
     | tREQUIRE tPROCEDURE tIDENTIFIER '('      ')' blk
     | tREQUIRE tPROCEDURE tIDENTIFIER '(' vars ')'
     | tREQUIRE tPROCEDURE tIDENTIFIER '(' vars ')' blk
     ;

identifiers : tIDENTIFIER
            | identifiers ',' tIDENTIFIER
            ;


vars : var
     | vars ',' var
     ;

type : tINTD
     | tREALD
     | tSTRINGD
     | tPTR    '<' type  '>'
     | tPTR    '<' tAUTO '>'
     ;

block : '{' decls instrs '}'
      | '{'       instrs '}'
      | '{' decls        '}'
      | '{'              '}'
      ;

instrs : instr
       | instrs instr
       ;

instr : expr ';'                   { $$ = new og::evaluation_node(LINE, $1); }
      | tWRITE   exprs ';'         { $$ = new og::write_node(LINE, $2); }
      | tWRITELN exprs ';'         { $$ = new og::write_node(LINE, $2, true); }
      | tBREAK                     { $$ = new og::break_node(LINE); }
      | tCONTINUE                  { $$ = new og::continue_node(LINE);}
      | tRETURN                    { $$ = new og::return_node(LINE); }
      | tRETURN  exprs ';'         { $$ = new og::return_node(LINE, $2); }
      | cond_instr                 { $$ = $1; }
      | iter_instr                 { $$ = $1; }
      | block                      { $$ = $1; }
      ;

cond_instr : tIF expr THEN instr
           | tIF expr THEN instr tELSE instr
           | tIF expr THEN elif_instrs
           | tIF expr THEN elif_instrs tELSE instr
           ;

elif_instr : tELIF expr THEN instr
           ;

elif_instrs : elif_instr
            | elif_instrs elif_instr
            ;

iter_instr : tFOR vars ';' exprs ';' exprs tDO instr
           | tFOR vars ';' exprs ';'       tDO instr
           | tFOR vars ';'       ';' exprs tDO instr
           | tFOR vars ';'       ';'       tDO instr
           | tFOR      ';' exprs ';' exprs tDO instr
           | tFOR      ';' exprs ';'       tDO instr
           | tFOR      ';'       ';' exprs tDO instr
           | tFOR      ';'       ';'       tDO instr
           ;

exprs : expr         { $$ = new cdk::sequence_node(LINE, $1); }
      | exprs ',' expr { $$ = new cdk::sequence_node(LINE, $3, $1); }
      ;

expr : integer                 { $$ = new cdk::integer_node(LINE, $1); }
     | real                    { $$ = new cdk::double_node(LINE, $1);  }
     | string                  { $$ = new cdk::string_node(LINE, $1);  }
     | tNULLPTR                { $$ = new cdk::nullptr_node(LINE);     }
     | '+' expr %prec tUNARY   { $$ = new og::identity_node(LINE, $2); }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2);     }
     | expr '+' expr           { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr           { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr           { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr           { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr           { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr           { $$ = new cdk::lt_node(LINE, $1, $3);  }
     | expr '>' expr           { $$ = new cdk::gt_node(LINE, $1, $3);  }
     | expr tGE expr           { $$ = new cdk::ge_node(LINE, $1, $3);  }
     | expr tLE expr           { $$ = new cdk::le_node(LINE, $1, $3);  }
     | expr tNE expr           { $$ = new cdk::ne_node(LINE, $1, $3);  }
     | expr tEQ expr           { $$ = new cdk::eq_node(LINE, $1, $3);  }
     | expr tOR expr           { $$ = new cdk::or_node(LINE, $1, $3);  }
     | expr tAND expr          { $$ = new cdk::and_node(LINE, $1, $3); }
     | '(' expr ')'            { $$ = $2; }
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | tINPUT  '(' lval  ')'   { $$ = new og::input_node(LINE); }
     | tSIZEOF '(' exprs ')'   { $$ = new og::sizeof_node(LINE, $3); }
     ;

lval :      tIDENTIFIER        { $$ = new cdk::variable_node(LINE, $1); }
     | type tIDENTIFIER        { $$ = new cdk::variable_node(LINE, $1); }
     ;

real    : tREAL                { $$ = new cdk::double_node(LINE, $1);  } ;
integer : tINTEGER             { $$ = new cdk::integer_node(LINE, $1); } ;
string  : tSTRING              { $$ = $1} /* string node? */
        | string tSTRING       { $$ = new std::string(*$1 + *$2) delete $1; $delete $2; }
        ;


// list : stmt         { $$ = new cdk::sequence_node(LINE, $1); }
//        | list stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
//        ;


// stmt : expr ';'                         { $$ = new og::evaluation_node(LINE, $1); }
//         | tWRITE expr ';'                  { $$ = new og::write_node(LINE, $2); }
//         | tWRITELN expr ';'                  { $$ = new og::write_node(LINE, $2, true); }
//      | tINPUT ';'                   { $$ = new og::input_node(LINE); }
//      | tFOR exprs ';' exprs ';' exprs tDO stmt         { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
//      | tIF '(' expr ')' stmt %prec tIFX { $$ = new og::if_node(LINE, $3, $5); }
//      | tIF '(' expr ')' stmt tELSE stmt { $$ = new og::if_else_node(LINE, $3, $5, $7); }
//      | '{' list '}'                     { $$ = $2; } <------------------- block
//      ;

%%
