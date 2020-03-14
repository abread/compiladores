#ifndef __OG_AST_CONTINUE_H__
#define __OG_AST_CONTINUE_H__

#include <cdk/ast/basic_node.h>

namespace og {

  class continue_node: public cdk::basic_node {
    int _level;

  public:
    continue_node(int lineno, int level = 1) :
        cdk::basic_node(lineno), _level(level) {
    }

  public:
    int level() const {
      return _level;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_continue_node(this, level);
    }

  };

} // og

#endif