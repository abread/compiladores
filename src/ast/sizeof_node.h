#ifndef __OG_AST_SIZEOF_H__
#define __OG_AST_SIZEOF_H__

#include <string>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace og {

  class sizeof_node: public cdk::expression_node {
    cdk::expression_node *_arguments;

  public:
    sizeof_node(int lineno, cdk::expression_node *arguments) :
        cdk::expression_node(lineno),
        _arguments(arguments) {}

  public:
    cdk::expression_node *arguments() {
      return _arguments;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sizeof_node(this, level);
    }

  };

} // og

#endif
