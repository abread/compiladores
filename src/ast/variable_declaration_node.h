#ifndef __OG_AST_VARIABLE_DECLARATION_H__
#define __OG_AST_VARIABLE_DECLARATION_H__

#include <cdk/ast/basic_node.h>
#include <cdk/types/basic_type.h>
#include "ast/tuple_node.h"
#include "targets/basic_ast_visitor.h"

namespace og {

  // not really an expression, but is has type...
  class variable_declaration_node: public cdk::basic_node {
    int _qualifier;
    cdk::basic_type *_varType;
    cdk::sequence_node *_identifiers;
    tuple_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qualifier, cdk::basic_type *varType, cdk::sequence_node *identifiers, tuple_node *initializer) :
        cdk::basic_node(lineno),
        _qualifier(qualifier),
        _varType(varType),
        _identifiers(identifiers),
        _initializer(initializer) {}

  public:
    int qualifier() {
      return _qualifier;
    }
    cdk::basic_type *varType() {
      return _varType;
    }
    cdk::sequence_node *identifiers() {
      return _identifiers;
    }
    tuple_node *initializer() {
      return _initializer;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_variable_declaration_node(this, level);
    }

  };

} // og

#endif
