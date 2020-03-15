#ifndef __OG_AST_NULLPTR_H__
#define __OG_AST_NULLPTR_H__

#include <cdk/ast/expression_node.h>

namespace og {

  class nullptr_node: public cdk::expression_node {
  public:
    nullptr_node(int lineno) :
        cdk::expression_node(lineno) {
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_nullptr_node(this, level);
    }

  };

} // og

#endif
