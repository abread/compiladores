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
%token tPRIVATE tPUBLIC tREQUIRE
%token tFOR tDO tTHEN tWRITE tWRITELN
%token tAUTO tINTD tREALD tSTRINGD tPTR tNULLPTR
%token tPROCEDURE tBREAK tCONTINUE tRETURN
%token tINPUT tSIZEOF

%nonassoc tIFX
%nonassoc tIF
%nonassoc tELIF
%nonassoc tELSE

%nonassoc tNOBLOCK
%nonassoc '{'
%nonassoc ','

%right '='
%left tAND tOR
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <s> string
%type <node> instr decl cond_instr elif iter_instr var func proc
%type <sequence> instrs file decls vars exprs
%type <expression> expr
%type <lvalue> lval
%type <blk> block
%type <t> type auto_t void_t

%%

file : decls        { $$ = $1; }
     ;

decls :       decl  { $$ = new cdk::sequence_node(LINE, $1); }
      | decls decl  { $$ = new cdk::sequence_node(LINE, $2, $1); }


decl : var ';' { $$ = $1; }
     | func    { $$ = $1; }
     | proc    { $$ = $1; }
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

/* just shorthands for them to be used as any other type */
void_t : tPROCEDURE { $$ = new cdk::primitive_type(0, cdk::typename_type::TYPE_VOID); }
auto_t : tAUTO      { $$ = new cdk::primitive_type(); }



func : tPUBLIC  type   tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3); delete $3; }
     | tPUBLIC  type   tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3, $5); delete $3; }
     | tPUBLIC  type   tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $6); delete $3; }
     | tPUBLIC  type   tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $5, $7); delete $3; }
     | tREQUIRE type   tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3); delete $3; }
     | tREQUIRE type   tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3, $5); delete $3; }
     | tREQUIRE type   tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $6); delete $3; }
     | tREQUIRE type   tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $5, $7); delete $3; }
     |          type   tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2); delete $2; }
     |          type   tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2, $4); delete $2; }
     |          type   tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $5); delete $2; }
     |          type   tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $4, $6); delete $2; }
     | tPUBLIC  auto_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3); delete $3; }
     | tPUBLIC  auto_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3, $5); delete $3; }
     | tPUBLIC  auto_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $6); delete $3; }
     | tPUBLIC  auto_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $5, $7); delete $3; }
     | tREQUIRE auto_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3); delete $3; }
     | tREQUIRE auto_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3, $5); delete $3; }
     | tREQUIRE auto_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $6); delete $3; }
     | tREQUIRE auto_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $5, $7); delete $3; }
     |          auto_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2); delete $2; }
     |          auto_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2, $4); delete $2; }
     |          auto_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $5); delete $2; }
     |          auto_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $4, $6); delete $2; }
     ;

identifiers : tIDENTIFIER
            | identifiers ',' tIDENTIFIER
            ;

proc : tPUBLIC  void_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3); delete $3; }
     | tPUBLIC  void_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPUBLIC, $2, *$3, $5); delete $3; }
     | tPUBLIC  void_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $6); delete $3; }
     | tPUBLIC  void_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPUBLIC, $2, *$3, $5, $7); delete $3; }
     | tREQUIRE void_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3); delete $3; }
     | tREQUIRE void_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tREQUIRE, $2, *$3, $5); delete $3; }
     | tREQUIRE void_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $6); delete $3; }
     | tREQUIRE void_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tREQUIRE, $2, *$3, $5, $7); delete $3; }
     |          void_t tIDENTIFIER '('      ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2); delete $2; }
     |          void_t tIDENTIFIER '(' vars ')' %prec tNOBLOCK { $$ = new og::function_declaration_node(LINE, tPRIVATE, $1, *$2, $4); delete $2; }
     |          void_t tIDENTIFIER '('      ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $5); delete $2; }
     |          void_t tIDENTIFIER '(' vars ')' block          { $$ = new og::function_definition_node(LINE, tPRIVATE, $1, *$2, $4, $6); delete $2; }
     ;

vars : var                 { $$ = new cdk::sequence_node(LINE, $1); }
     | vars ',' var        { $$ = new cdk::sequence_node(LINE, $3, $1); }
     ;

type : tINTD                    { $$ = new cdk::primitive_type(4, cdk::typename_type::TYPE_INT); }
     | tREALD                   { $$ = new cdk::primitive_type(8, cdk::typename_type::TYPE_DOUBLE); }
     | tSTRINGD                 { $$ = new cdk::primitive_type(8, cdk::typename_type::TYPE_STRING); }
     | tPTR    '<' type  '>'    { $$ = new cdk::reference_type(8, std::shared_ptr<cdk::basic_type>($3)); }
     | tPTR    '<' tAUTO '>'    { $$ = new cdk::primitive_type(8, cdk::typename_type::TYPE_POINTER); }
     ;

block : '{' decls instrs '}'    { $$ = new og::block_node(LINE, $2, $3); }
      | '{'       instrs '}'    { $$ = new og::block_node(LINE, nullptr, $2); }
      | '{' decls        '}'    { $$ = new og::block_node(LINE, $2, nullptr); }
      | '{'              '}'    { $$ = new og::block_node(LINE, nullptr, nullptr); }
      ;

instrs : instr                  { $$ = new cdk::sequence_node(LINE, $1); }
       | instrs instr           { $$ = new cdk::sequence_node(LINE, $2, $1); }
       ;

instr : expr ';'                   { $$ = new og::evaluation_node(LINE, $1); }
      | tWRITE   exprs ';'         { $$ = new og::write_node(LINE, new og::tuple_node(LINE, $2)); }
      | tWRITELN exprs ';'         { $$ = new og::write_node(LINE, new og::tuple_node(LINE, $2), true); }
      | tBREAK                     { $$ = new og::break_node(LINE); }
      | tCONTINUE                  { $$ = new og::continue_node(LINE);}
      | tRETURN ';'                { $$ = new og::return_node(LINE); }
      | tRETURN exprs ';'          { $$ = new og::return_node(LINE, new og::tuple_node(LINE, $2)); }
      | cond_instr                 { $$ = $1; }
      | iter_instr                 { $$ = $1; }
      | block                      { $$ = $1; }
      ;

cond_instr : tIF expr tTHEN instr %prec tIFX       { $$ = new og::if_node(LINE, $2, $4); }
           | tIF expr tTHEN instr elif             { $$ = new og::if_else_node(LINE, $2, $4, $5); }
           ;

elif : tELSE instr                       { $$ = $2; }
     | tELIF expr tTHEN instr %prec tIFX { $$ = new og::if_node(LINE, $2, $4); }
     | tELIF expr tTHEN instr elif       { $$ = new og::if_else_node(LINE, $2, $4, $5); }
     ;

iter_instr : tFOR vars  ';' exprs ';' exprs tDO instr    { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
           | tFOR vars  ';' exprs ';'       tDO instr    { $$ = new og::for_node(LINE, $2, $4, nullptr, $7); }
           | tFOR vars  ';'       ';' exprs tDO instr    { $$ = new og::for_node(LINE, $2, nullptr, $5, $7); }
           | tFOR vars  ';'       ';'       tDO instr    { $$ = new og::for_node(LINE, $2, nullptr, nullptr, $6); }
           | tFOR exprs ';' exprs ';' exprs tDO instr    { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
           | tFOR exprs ';' exprs ';'       tDO instr    { $$ = new og::for_node(LINE, $2, $4, nullptr, $7); }
           | tFOR exprs ';'       ';' exprs tDO instr    { $$ = new og::for_node(LINE, $2, nullptr, $5, $7); }
           | tFOR exprs ';'       ';'       tDO instr    { $$ = new og::for_node(LINE, $2, nullptr, nullptr, $6); }
           | tFOR       ';' exprs ';' exprs tDO instr    { $$ = new og::for_node(LINE, nullptr, $3, $5, $7); }
           | tFOR       ';' exprs ';'       tDO instr    { $$ = new og::for_node(LINE, nullptr, $3, nullptr, $6); }
           | tFOR       ';'       ';' exprs tDO instr    { $$ = new og::for_node(LINE, nullptr, nullptr, $4, $6); }
           | tFOR       ';'       ';'       tDO instr    { $$ = new og::for_node(LINE, nullptr, nullptr, nullptr, $5); }
           ;

exprs : expr                        { $$ = new cdk::sequence_node(LINE, $1); }
      | exprs ',' expr              { $$ = new cdk::sequence_node(LINE, $3, $1); }
      ;

expr : tINT                      { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                     { $$ = new cdk::double_node(LINE, $1);  }
     | string                    { $$ = new cdk::string_node(LINE, *$1); delete $1; }
     | tNULLPTR                  { $$ = new og::nullptr_node(LINE);      }
     | '+' expr %prec tUNARY     { $$ = new og::identity_node(LINE, $2); }
     | '-' expr %prec tUNARY     { $$ = new cdk::neg_node(LINE, $2);     }
     | expr '+' expr             { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr             { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr             { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr             { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr             { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr             { $$ = new cdk::lt_node(LINE, $1, $3);  }
     | expr '>' expr             { $$ = new cdk::gt_node(LINE, $1, $3);  }
     | expr tGE expr             { $$ = new cdk::ge_node(LINE, $1, $3);  }
     | expr tLE expr             { $$ = new cdk::le_node(LINE, $1, $3);  }
     | expr tNE expr             { $$ = new cdk::ne_node(LINE, $1, $3);  }
     | expr tEQ expr             { $$ = new cdk::eq_node(LINE, $1, $3);  }
     | expr tOR expr             { $$ = new cdk::or_node(LINE, $1, $3);  }
     | expr tAND expr            { $$ = new cdk::and_node(LINE, $1, $3); }
     | '(' expr ')'              { $$ = $2; }
     | lval                      { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr             { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | lval '?'                  { $$ = new og::address_of_node(LINE, $1); }
     | tINPUT  '(' lval  ')'     { $$ = new og::input_node(LINE); }
     | tSIZEOF '(' exprs ')'     { $$ = new og::sizeof_node(LINE, new og::tuple_node(LINE, $3)); }
     | '[' expr ']'              { $$ = new og::stack_alloc_node(LINE, $2); }
     | tIDENTIFIER '(' exprs ')' { $$ = new og::function_call_node(LINE, *$1, new og::tuple_node(LINE, $3)); delete $1; }
     | tIDENTIFIER '('       ')' { $$ = new og::function_call_node(LINE, *$1); delete $1; }
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, *$1); delete $1; }
     | lval '[' expr ']'       { $$ = new og::pointer_index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     | lval '@' tINT           { $$ = new og::tuple_index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     ;

string : tSTRING              { $$ = $1; }
       | string tSTRING       { $$ = new std::string(*$1 + *$2); delete $1; delete $2; }
       ;
%%
