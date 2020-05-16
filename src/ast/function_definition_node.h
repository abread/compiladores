#ifndef __OG_AST_FUNCTION_DEFINITION_H__
#define __OG_AST_FUNCTION_DEFINITION_H__

#include <string>
#include <memory>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include "ast/block_node.h"

namespace og {

  /**
   * Class for describing function definitions.
   */
  class function_definition_node: public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::sequence_node *_arguments;
    block_node *_block;

  public:
    function_definition_node(int lineno, int qualifier, cdk::basic_type *type, const std::string &identifier, block_node *block) :
        function_definition_node(lineno, qualifier, type, identifier, nullptr, block) {}
    function_definition_node(int lineno, int qualifier, cdk::basic_type *type, const std::string &identifier, cdk::sequence_node *arguments, block_node *block) :
        cdk::typed_node(lineno),
        _qualifier(qualifier),
        _identifier(identifier),
        _arguments(arguments),
        _block(block) {
          this->type(std::shared_ptr<cdk::basic_type>(type));
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
    block_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // og

#endif
