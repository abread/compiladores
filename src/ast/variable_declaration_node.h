#ifndef __OG_AST_VARIABLE_DECLARATION_H__
#define __OG_AST_VARIABLE_DECLARATION_H__

#include <vector>
#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>

namespace og {

  class variable_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::vector<std::string> _identifiers;
    cdk::expression_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qualifier, std::string id) :
      cdk::typed_node(lineno),
      _qualifier(qualifier),
      _identifiers(),
      _initializer(nullptr) {
        _identifiers.push_back(id);
      }
    variable_declaration_node(variable_declaration_node *old, std::string id) :
      cdk::typed_node(old->lineno()),
      _qualifier(old->qualifier()),
      _identifiers(old->identifiers()),
      _initializer(old->initializer()) {
        _identifiers.push_back(id);
      }
    variable_declaration_node(variable_declaration_node *old, cdk::expression_node* initializer) :
      cdk::typed_node(old->lineno()),
      _qualifier(old->qualifier()),
      _identifiers(old->identifiers()),
      _initializer(initializer) {}

  public:
    int qualifier() {
      return _qualifier;
    }
    std::vector<std::string> &identifiers() {
      return _identifiers;
    }
    cdk::expression_node *initializer() {
      return _initializer;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_variable_declaration_node(this, level);
    }

  };

} // og

#endif
