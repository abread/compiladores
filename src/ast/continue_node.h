#ifndef __OG_AST_CONTINUE_H__
#define __OG_AST_CONTINUE_H__

#include <cdk/ast/basic_node.h>

namespace og {

  class continue_node: public cdk::basic_node {

  public:
    continue_node(int lineno) :
        cdk::basic_node(lineno) {
    }

  public:

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_continue_node(this, level);
    }

  };

} // og

#endif
