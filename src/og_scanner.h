#ifndef __OGSCANNER_H__
#define __OGSCANNER_H__

#undef yyFlexLexer
#define yyFlexLexer og_scanner_FlexLexer
#include <FlexLexer.h>

typedef og_scanner_FlexLexer og_scanner;

#endif
