%option c++ prefix="og_scanner_" outfile="og_scanner.cpp"
%option stack noyywrap yylineno 8bit

%{
// make relevant includes before including the parser's tab file
#include <string>
#include <iostream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "og_parser.tab.h"

// don't change this
#define yyerror LexerError

int xstoi(const std::string &s, int base = 10) {
  try {
    return std::stoi(s, NULL, base);
  } catch (const std::invalid_argument &e) {
    std::cerr << "invalid integer: " << s << std::endl;
    exit(1);
  } catch (const std::out_of_range &e) {
    std::cerr << "integer out of range: " << s << std::endl;
    exit(1);
  }
}
double xstod(const std::string &s) {
  try {
    return std::stod(s, NULL);
  } catch (const std::invalid_argument &e) {
    std::cerr << "invalid double: " << s << std::endl;
    exit(1);
  } catch (const std::out_of_range &e) {
    std::cerr << "double out of range: " << s << std::endl;
    exit(1);
  }
}
%}

%x X_STRING X_COMMENT
%%

"//".*                 ; /* ignore comments */

"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"*/"        yy_pop_state();
<X_COMMENT>"/*"        yy_push_state(X_COMMENT);
<X_COMMENT>.|\n         ; /* ignore comments */

">="                   return tGE;
"<="                   return tLE;
"=="                   return tEQ;
"!="                   return tNE;

"&&"                   return tAND;
"||"                   return tOR;

"auto"                 return tAUTO;
"int"                  return tINTD;
"real"                 return tREALD;
"string"               return tSTRINGD;
"ptr"                  return tPTR;
"public"               return tPUBLIC;
"require"              return tREQUIRE;
"sizeof"               return tSIZEOF;
"input"                return tINPUT;
"nullptr"              return tNULLPTR;
"procedure"            return tPROCEDURE;
"break"                return tBREAK;
"continue"             return tCONTINUE;
"return"               return tRETURN;
"if"                   return tIF;
"then"                 return tTHEN;
"elif"                 return tELIF;
"else"                 return tELSE;
"for"                  return tFOR;
"do"                   return tDO;
"write"                return tWRITE;
"writeln"              return tWRITELN;

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

\"                     yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"           yy_pop_state(); return tSTRING;
<X_STRING>\\\"         *yylval.s += yytext + 1;
<X_STRING>\\\\         *yylval.s += yytext + 1;
<X_STRING>.            *yylval.s += yytext;
<X_STRING>\n           yyerror("newline in string");


[0-9]+                               yylval.i = xstoi(yytext); return tINT;
0x[0-9a-fA-F]+                       yylval.i = xstoi(yytext, 16); return tINT;
[0-9]*\.[0-9]+([Ee][-+]?[0-9]+)?     yylval.d = xstod(yytext); return tREAL;
[0-9]+\.[0-9]*([Ee][-+]?[0-9]+)?     yylval.d = xstod(yytext); return tREAL;
[0-9]+([Ee][-+]?[0-9]+)              yylval.d = xstod(yytext); return tREAL;

[-()<>=+*/%;{}.~?\[\],@] return *yytext;

[ \t\n]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
