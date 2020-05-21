#ifndef __OG_AST_TUPLE_NODE_H__
#define __OG_AST_TUPLE_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  /**
   * Class representing a tuple.
   */
  class tuple_node: public cdk::expression_node {
    cdk::sequence_node *_elements;

  public:
    tuple_node(int lineno, cdk::sequence_node* seq) :
      cdk::expression_node(lineno), _elements(seq) {}
    tuple_node(cdk::sequence_node* seq) :
      tuple_node(seq->lineno(), seq) {}

    cdk::sequence_node* seq() {
      return _elements;
    }

    std::vector<cdk::basic_node*> &elements() {
      return _elements->nodes();
    }

    cdk::expression_node* element(size_t i) {
      return static_cast<cdk::expression_node*>(_elements->node(i));
    }

    size_t size() {
      return _elements->size();
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
