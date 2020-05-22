#ifndef __OG_TARGET_FRAME_SIZE_CALCULATOR_H__
#define __OG_TARGET_FRAME_SIZE_CALCULATOR_H__

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

    // just so we can call ASSERT_SAFE_EXPRESSIONS
    std::shared_ptr<og::symbol> _function;

    size_t _localsize = 0;
    size_t _calltempsize = 0; // storage for tuple-returning functions (only one is used at a time, so a single space is sufficient)
    size_t _returntempsize = 0; // storage for return with tuple (only one return will run in each function so a single space is sufficient)

    std::map<cdk::basic_node const*, int> _unsharedTempSizeTab; // storage required for tuple_nodes temporary variables and for_node temporary variables

    bool _needTupleAddr = true; // true = needs tuple to be stored somewhere and evaluate to its base address
    bool _evaledTupleAddr = true; // true = tuple was evaluated to base address

    void load_value(cdk::typed_node *lval_or_expr, int lvl, cdk::basic_node const * caller);

  public:
    frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, std::shared_ptr<og::symbol> function, cdk::symbol_table<og::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab), _function(function) {
    }

  public:
    ~frame_size_calculator();

  public:
    size_t localsize() const {
      return _localsize;
    }

    const std::map<cdk::basic_node const*, int> unsharedTempSizeTab() const {
      return _unsharedTempSizeTab;
    }

    size_t calltempsize() const {
      return _calltempsize;
    }

    size_t returntempsize() const {
      return _returntempsize;
    }

    size_t tempsize() const {
      size_t totalsz = _calltempsize + _returntempsize;
      for (const auto&  tempsz : _unsharedTempSizeTab) {
        totalsz += tempsz.second;
      }

      return totalsz;
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
