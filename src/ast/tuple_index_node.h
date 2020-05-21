#ifndef __OG_AST_TUPLE_INDEX_H_
#define __OG_AST_TUPLE_INDEX_H_

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>

namespace og {

  class tuple_index_node: public cdk::lvalue_node {
    cdk::expression_node *_base;
    size_t _index;

  public:
    tuple_index_node(int lineno, cdk::expression_node *base, size_t index) :
        cdk::lvalue_node(lineno), _base(base), _index(index) {
    }

  public:
    cdk::expression_node *base() {
      return _base;
    }
    size_t index() {
      return _index;
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tuple_index_node(this, level);
    }

  };

} // og

#endif
