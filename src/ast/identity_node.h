#ifndef __OG_AST_IDENTITY_NODE_H__
#define __OG_AST_IDENTITY_NODE_H__

#include <cdk/ast/unary_operation_node.h>

namespace og {

  /**
   * Class for describing the identity operator
   */
  class identity_node: public cdk::unary_operation_node {
  public:
    identity_node(int lineno, expression_node *arg) :
        cdk::unary_operation_node(lineno, arg) {
    }

    /**
     * @param av basic AST visitor
     * @param level syntactic tree level
     */
    void accept(basic_ast_visitor *av, int level) {
      av->do_identity_node(this, level);
    }

  };

} // og

#endif
