#ifndef __OG_AST_VARIABLE_DECLARATION_H__
#define __OG_AST_VARIABLE_DECLARATION_H__

#include <vector>
#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>

namespace og {

  class variable_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::shared_ptr<cdk::basic_type> _typeHint;
    std::vector<std::string> _identifiers;
    cdk::expression_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qual, cdk::basic_type *typeHint, std::string &id, cdk::expression_node *init = nullptr) :
      cdk::typed_node(lineno),
      _qualifier(qual),
      _typeHint(std::shared_ptr<cdk::basic_type>(typeHint)),
      _identifiers(std::vector<std::string>{id}),
      _initializer(init) {}

    variable_declaration_node(int lineno, int qual, cdk::basic_type *typeHint, std::vector<std::string> &ids, cdk::expression_node *init = nullptr) :
      cdk::typed_node(lineno),
      _qualifier(qual),
      _typeHint(std::shared_ptr<cdk::basic_type>(typeHint)),
      _identifiers(ids),
      _initializer(init) {}

  public:
    int qualifier() {
      return _qualifier;
    }
    std::shared_ptr<cdk::basic_type> typeHint() {
      return _typeHint;
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
