# $Id: Makefile,v 1.1 2020/02/21 15:42:18 ist13500 Exp $
#---------------------------------------------------------------
#             CONFIGURE THESE VARIABLES IF NEEDED
#---------------------------------------------------------------

ROOT = ${HOME}/compiladores/root
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
OFILES  = $(SRC_CPP:%.cpp=%.o)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

all: ast/all.h ast/visitor_decls.h $(COMPILER)

%.tab.o:: %.tab.c
	$(CXX) $(CXXFLAGS) -DYYDEBUG -c $< -o $@

%.o:: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o:: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.tab.c:: %.y
	$(YACC) -dtv -b $* $<

%.tab.h:: %.y
	$(YACC) -dtv -b $* $<

$(L_NAME).cpp: $(L_NAME).l
	$(LEX) $(LFLAGS) $<

$(Y_NAME).tab.c: $(Y_NAME).y
$(Y_NAME).tab.h: $(Y_NAME).y

# this is needed to force byacc to run
$(L_NAME).o: $(L_NAME).cpp $(Y_NAME).tab.h

ast/all.h: 
	$(CDK_BIN_DIR)/mk-node-decls.pl $(LANGUAGE) > $@

ast/visitor_decls.h: 
	$(CDK_BIN_DIR)/mk-visitor-decls.pl $(LANGUAGE) > $@

$(COMPILER): $(L_NAME).o $(Y_NAME).tab.o $(OFILES)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) ast/all.h ast/visitor_decls.h *.tab.[ch] *.o $(OFILES) $(L_NAME).cpp $(Y_NAME).output $(COMPILER)

depend: ast/all.h
	$(CXX) $(CXXFLAGS) -MM $(SRC_CPP) > .makedeps

-include .makedeps

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
