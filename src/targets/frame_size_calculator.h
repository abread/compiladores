#ifndef __GR8_TARGET_FRAME_SIZE_CALCULATOR_H__
#define __GR8_TARGET_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stack>
#include <cdk/symbol_table.h>
#include "targets/symbol.h"

namespace og {

  class frame_size_calculator: public basic_ast_visitor {
    cdk::symbol_table<og::symbol> &_symtab;

    size_t _localsize;

  public:
    frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab), _localsize(0) {
    }

  public:
    ~frame_size_calculator();

  public:
    size_t localsize() const {
      return _localsize;
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // og

#endif
