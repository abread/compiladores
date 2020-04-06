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
    variable_declaration_node(int lineno, int qual, cdk::basic_type *type, std::string &id) :
      variable_declaration_node(lineno, qual, type, id, nullptr) {}

    variable_declaration_node(int lineno, int qual, cdk::basic_type *type, std::string &id, cdk::expression_node *init) :
      cdk::typed_node(lineno),
      _qualifier(qual),
      _identifiers(std::vector<std::string>(1, id)),
      _initializer(init) {
        this->type(std::shared_ptr<cdk::basic_type>(type));
      }

    variable_declaration_node(int lineno, int qual, std::vector<std::string> &ids) :
      variable_declaration_node(lineno, qual, new cdk::primitive_type(), ids, nullptr) {}

    variable_declaration_node(int lineno, int qual, std::vector<std::string> &ids, cdk::expression_node *init) :
      variable_declaration_node(lineno, qual, new cdk::primitive_type(), ids, init) {}

    variable_declaration_node(int lineno, int qual, cdk::basic_type *type, std::vector<std::string> &ids, cdk::expression_node *init) :
      cdk::typed_node(lineno),
      _qualifier(qual),
      _identifiers(ids),
      _initializer(init) {
        this->type(std::shared_ptr<cdk::basic_type>(type));
      }

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
