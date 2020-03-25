#ifndef __OG_AST_TUPLE_INDEX_H_
#define __OG_AST_TUPLE_INDEX_H_

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>
#include "targets/basic_ast_visitor.h"

namespace og {

  class tuple_index_node: public cdk::lvalue_node {
    cdk::expression_node *_base;
    cdk::integer_node *_index;

  public:
    tuple_index_node(int lineno, cdk::expression_node *base, cdk::integer_node *index) :
        cdk::lvalue_node(lineno), _base(base), _index(index) {
    }

  public:
    cdk::expression_node *base() {
      return _base;
    }
    cdk::expression_node *index() {
      return _index;
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tuple_index_node(this, level);
    }

  };

} // og

#endif
