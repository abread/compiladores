#ifndef __OG_AST_FUNCTION_DECLARATION_H__
#define __OG_AST_FUNCTION_DECLARATION_H__

#include <string>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/primitive_type.h>

namespace og {

  /**
   * Class for describing function declarations.
   */
  class function_declaration_node: public cdk::basic_node {
    int _qualifier;
    cdk::basic_type *_type;
    std::string _identifier;
    cdk::sequence_node *_arguments;

  public:
    function_declaration_node(int lineno, int qualifier, const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::basic_node(lineno),
        _qualifier(qualifier),
        _type(new cdk::primitive_type(0, cdk::typename_type::TYPE_VOID)),
        _identifier(identifier),
        _arguments(arguments) {}

    function_declaration_node(int lineno, int qualifier, cdk::basic_type *type, const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::basic_node(lineno),
        _qualifier(qualifier),
        _type(type),
        _identifier(identifier),
        _arguments(arguments) {}

  public:
    int qualifier() {
      return _qualifier;
    }
    cdk::basic_type *type() {
      return _type;
    }
    const std::string &identifier() const {
      return _identifier;
    }
    cdk::sequence_node *arguments() {
      return _arguments;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_declaration_node(this, level);
    }

  };

} // og

#endif
