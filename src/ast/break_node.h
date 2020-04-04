#ifndef __OG_AST_BREAK_H__
#define __OG_AST_BREAK_H__

#include <cdk/ast/basic_node.h>

namespace og {

  class break_node: public cdk::basic_node {

  public:
    break_node(int lineno) :
        cdk::basic_node(lineno) {
    }

  public:
    void accept(basic_ast_visitor *sp) {
      sp->do_break_node(this);
    }

  };

} // og

#endif
