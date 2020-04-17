#ifndef __OG_AST_FOR_NODE_H__
#define __OG_AST_FOR_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node: public cdk::basic_node {
    cdk::sequence_node *_initializers;
    cdk::sequence_node *_conditions;
    cdk::sequence_node *_increments;
    cdk::basic_node *_block;

  public:
    inline for_node(int lineno, cdk::sequence_node *initializers, cdk::sequence_node *conditions, cdk::sequence_node *increments, cdk::basic_node *block) :
        basic_node(lineno), _initializers(initializers), _conditions(conditions), _increments(increments), _block(block) {
    }

  public:
    inline cdk::sequence_node *initializers() {
      return _initializers;
    }
    inline cdk::sequence_node *conditions() {
      return _conditions;
    }
    inline cdk::sequence_node *increments() {
      return _increments;
    }
    inline cdk::basic_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_for_node(this, level);
    }

  };

} // og

#endif
