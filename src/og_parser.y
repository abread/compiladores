%{
#include <memory>
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
  int                  i; /* tINT value */
  double               d; /* double value */
  std::string          *s; /* symbol name or string literal */
  cdk::basic_node      *node; /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  cdk::basic_type      *t;

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

%type <s> string
%type <node> instr decl cond_instr elif iter_instr
%type <sequence> instrs exprs file decls
%type <expression> expr
%type <lvalue> lval
%type <blk> block
%type <t> type

%%

file : decls        { $$ = $1; }
     ;

decls :       decl  { $$ = new cdk::sequence_node(LINE, $1); }
      | decls decl  { $$ = new cdk::sequence_node(LINE, $2, $1); }


decl : var ';'
     | function
     | procedure
     ;

var :           type tIDENTIFIER
    |           type tIDENTIFIER '=' expr
    | tPUBLIC   type tIDENTIFIER
    | tPUBLIC   type tIDENTIFIER '=' expr
    | tREQUIRE  type tIDENTIFIER
    | tREQUIRE  type tIDENTIFIER '=' expr
    |          tAUTO identifiers '=' exprs
    | tPUBLIC  tAUTO identifiers '=' exprs
    ;

fun :           type tIDENTIFIER '('      ')'
    |           type tIDENTIFIER '('      ')' block
    |           type tIDENTIFIER '(' vars ')'
    |           type tIDENTIFIER '(' vars ')' block
    |          tAUTO tIDENTIFIER '('      ')'
    |          tAUTO tIDENTIFIER '('      ')' block
    |          tAUTO tIDENTIFIER '(' vars ')'
    |          tAUTO tIDENTIFIER '(' vars ')' block
    | tPUBLIC   type tIDENTIFIER '('      ')'
    | tPUBLIC   type tIDENTIFIER '('      ')' block
    | tPUBLIC   type tIDENTIFIER '(' vars ')'
    | tPUBLIC   type tIDENTIFIER '(' vars ')' block
    | tPUBLIC  tAUTO tIDENTIFIER '('      ')'
    | tPUBLIC  tAUTO tIDENTIFIER '('      ')' block
    | tPUBLIC  tAUTO tIDENTIFIER '(' vars ')'
    | tPUBLIC  tAUTO tIDENTIFIER '(' vars ')' block
    | tREQUIRE  type tIDENTIFIER '('      ')'
    | tREQUIRE  type tIDENTIFIER '('      ')' block
    | tREQUIRE  type tIDENTIFIER '(' vars ')'
    | tREQUIRE  type tIDENTIFIER '(' vars ')' block
    | tREQUIRE tAUTO tIDENTIFIER '('      ')'
    | tREQUIRE tAUTO tIDENTIFIER '('      ')' block
    | tREQUIRE tAUTO tIDENTIFIER '(' vars ')'
    | tREQUIRE tAUTO tIDENTIFIER '(' vars ')' block
    ;


proc :          tPROCEDURE tIDENTIFIER '('      ')'
     |          tPROCEDURE tIDENTIFIER '('      ')' block
     |          tPROCEDURE tIDENTIFIER '(' vars ')'
     |          tPROCEDURE tIDENTIFIER '(' vars ')' block
     | tPUBLIC  tPROCEDURE tIDENTIFIER '('      ')'
     | tPUBLIC  tPROCEDURE tIDENTIFIER '('      ')' block
     | tPUBLIC  tPROCEDURE tIDENTIFIER '(' vars ')'
     | tPUBLIC  tPROCEDURE tIDENTIFIER '(' vars ')' block
     | tREQUIRE tPROCEDURE tIDENTIFIER '('      ')'
     | tREQUIRE tPROCEDURE tIDENTIFIER '('      ')' block
     | tREQUIRE tPROCEDURE tIDENTIFIER '(' vars ')'
     | tREQUIRE tPROCEDURE tIDENTIFIER '(' vars ')' block
     ;

identifiers : tIDENTIFIER
            | identifiers ',' tIDENTIFIER
            ;


vars : var
     | vars ',' var
     ;

type : tINTD                                     { $$ = new cdk::primitive_type(4, cdk::typename_type::TYPE_INT); }
     | tREALD                                    { $$ = new cdk::primitive_type(8, cdk::typename_type::TYPE_DOUBLE); }
     | tSTRINGD                                  { $$ = new cdk::primitive_type(8, cdk::typename_type::TYPE_STRING); }
     | tPTR    '<' type  '>'                     { $$ = new cdk::reference_type(8, std::shared_ptr<cdk::basic_type>($3)); }
     | tPTR    '<' tAUTO '>                      { $$ = new cdk::primitive_type(8, basic_type::TYPE_POINTER); }'
     ;

block : '{' decls instrs '}'    { $$ = new og::block_node(LINE, $2, $3); }
      | '{'       instrs '}'    { $$ = new og::block_node(LINE, NULL, $2); }
      | '{' decls        '}'    { $$ = new og::block_node(LINE, $2, NULL); }
      | '{'              '}'    { $$ = new og::block_node(LINE, NULL, NULL); }
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

cond_instr : tIF expr tTHEN instr                  { $$ = new og::if_node(LINE, $2, $4); }
           | tIF expr tTHEN instr elif             { $$ = new og::if_else_node(LINE, $2, $4, $5); }
           ;

elif : tELSE instr                  { $$ = $2; }
     | tELIF expr tTHEN instr       { $$ = new og::if_node(LINE, $2, $4); }
     | tELIF expr tTHEN instr elif  { $$ = new og::if_else_node(LINE, $2, $4, $5); }
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

expr : tINT                    { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                   { $$ = new cdk::double_node(LINE, $1);  }
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
     | type tIDENTIFIER        { $$ = new cdk::variable_node(LINE, $1, $2); }
     ;

string  : tSTRING              { $$ = $1; }
        | string tSTRING       { $$ = new std::string(*$1 + *$2) delete $1; $delete $2; }
        ;
%%
