# $Id: Makefile,v 1.5 2020/03/22 22:53:04 ist189409 Exp $
#---------------------------------------------------------------
#             CONFIGURE THESE VARIABLES IF NEEDED
#---------------------------------------------------------------

ROOT = ../build
#ROOT = ${HOME}/compiladores/root
CDK_INC_DIR = $(ROOT)/usr/include
CDK_LIB_DIR = $(ROOT)/usr/lib
CDK_BIN_DIR = $(ROOT)/usr/bin

LANGUAGE=og

#---------------------------------------------------------------
# PROBABLY, THERE'S NO NEED TO CHANGE ANYTHING BEYOND THIS POINT
#---------------------------------------------------------------

L_NAME=$(LANGUAGE)_scanner
Y_NAME=$(LANGUAGE)_parser

LFLAGS   =
YFLAGS   = -dtvP
CXXFLAGS = -std=c++17 -DYYDEBUG=1 -pedantic -Wall -Wextra -ggdb -I. -I$(CDK_INC_DIR) -Wno-unused-parameter
LDFLAGS  = -L$(CDK_LIB_DIR) -lcdk #-lLLVM
COMPILER = $(LANGUAGE)

LEX  = flex
YACC = byacc

SRC_CPP = $(shell find ast -name \*.cpp) $(wildcard targets/*.cpp) $(wildcard ./*.cpp)
AST_HEADERS = $(shell find ast -name \*.h -not -name all.h -not -name visitor_decls.h)
OFILES  = $(SRC_CPP:%.cpp=%.o)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

all: $(COMPILER)

%.tab.o: %.tab.c
	$(CXX) $(CXXFLAGS) -DYYDEBUG -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.tab.h %.tab.c: %.y
	$(YACC) -dtv -b $* $<

$(L_NAME).cpp: $(L_NAME).l
	$(LEX) $(LFLAGS) $<

# this is needed to force byacc to run
$(L_NAME).o: $(L_NAME).cpp $(Y_NAME).tab.h

ast/all.h:
	$(CDK_BIN_DIR)/mk-node-decls.pl $(LANGUAGE) > $@

ast/visitor_decls.h:
	$(CDK_BIN_DIR)/mk-visitor-decls.pl $(LANGUAGE) > $@

# force ast/{all,visitor_decls}.h to be generated before the parser&lexer are built
$(Y_NAME).tab.o: ast/all.h ast/visitor_decls.h
$(L_NAME).o: ast/all.h ast/visitor_decls.h

# for ast/{all,visitor_decls.h} to be regenerated when the ast is modified
ast/all.h: $(AST_HEADERS)
ast/visitor_decls.h: $(AST_HEADERS)

$(COMPILER): $(L_NAME).o $(Y_NAME).tab.o $(OFILES)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) ast/all.h ast/visitor_decls.h *.tab.[ch] *.o $(OFILES) $(L_NAME).cpp $(Y_NAME).output $(COMPILER)

# make depend doesn't figure this one out
depend: $(Y_NAME).tab.h

depend: ast/all.h ast/visitor_decls.h
	$(CXX) $(CXXFLAGS) -MM $(SRC_CPP) > .makedeps

-include .makedeps

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
