#ifndef __OG_AST_TUPLE_NODE_H__
#define __OG_AST_TUPLE_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  /**
   * Class representing a tuple.
   */
  class tuple_node: public cdk::expression_node {
    cdk::sequence_node _elements;

  public:
    tuple_node(int lineno) :
      cdk::expression_node(lineno), _elements(lineno) {}

    tuple_node(int lineno, cdk::expression_node* expr) :
      cdk::expression_node(lineno), _elements(lineno, expr) {}

    tuple_node(int lineno, cdk::expression_node* expr, tuple_node* prev) :
      cdk::expression_node(lineno), _elements(lineno, expr, &prev->elements()) {}

    cdk::sequence_node& elements() {
      return _elements;
    }

    /**
     * @param av basic AST visitor
     * @param level syntactic tree level
     */
    void accept(basic_ast_visitor *av, int level) {
      av->do_tuple_node(this, level);
    }

  };

} // og

#endif
