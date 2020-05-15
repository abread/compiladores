#ifndef __OG_AST_FUNCTION_DECLARATION_H__
#define __OG_AST_FUNCTION_DECLARATION_H__

#include <string>
#include <memory>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  /**
   * Class for describing function declarations.
   */
  class function_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::shared_ptr<cdk::basic_type> _typeHint;
    std::string _identifier;
    cdk::sequence_node *_arguments;

  public:
    function_declaration_node(int lineno, int qualifier, cdk::basic_type *typeHint, const std::string &identifier) :
        function_declaration_node(lineno, qualifier, typeHint, identifier, nullptr) {}
    function_declaration_node(int lineno, int qualifier, cdk::basic_type *typeHint, const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::typed_node(lineno),
        _qualifier(qualifier),
        _typeHint(std::shared_ptr<cdk::basic_type>(typeHint)),
        _identifier(identifier),
        _arguments(arguments) {}

  public:
    int qualifier() {
      return _qualifier;
    }
    const std::string &identifier() const {
      return _identifier;
    }
    cdk::sequence_node *arguments() {
      return _arguments;
    }
    std::shared_ptr<cdk::basic_type> typeHint() {
      return _typeHint;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_declaration_node(this, level);
    }

  };

} // og

#endif
