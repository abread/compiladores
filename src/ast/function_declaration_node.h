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
    std::string _identifier;
    cdk::sequence_node *_arguments;

  public:
    function_declaration_node(int lineno, int qualifier, const std::string &identifier, cdk::sequence_node *arguments) :
        function_declaration_node(lineno, qualifier, std::make_shared<cdk::primitive_type>(0, cdk::typename_type::TYPE_VOID), identifier, arguments) {}
    function_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &identifier, cdk::sequence_node *arguments) :
        cdk::typed_node(lineno),
        _qualifier(qualifier),
        _identifier(identifier),
        _arguments(arguments) {
          this->type(type);
        }

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

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_declaration_node(this, level);
    }

  };

} // og

#endif
