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
    std::shared_ptr<cdk::basic_type> _typeHint;
    std::string _identifier;
    cdk::sequence_node *_arguments;
    block_node *_block;

  public:
    function_definition_node(int lineno, int qualifier, cdk::basic_type *typeHint, const std::string &identifier, block_node *block) :
        function_definition_node(lineno, qualifier, typeHint, identifier, nullptr, block) {}
    function_definition_node(int lineno, int qualifier, cdk::basic_type *typeHint, const std::string &identifier, cdk::sequence_node *arguments, block_node *block) :
        cdk::typed_node(lineno),
        _qualifier(qualifier),
        _typeHint(std::shared_ptr<cdk::basic_type>(typeHint)),
        _identifier(identifier),
        _arguments(arguments),
        _block(block) {}

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
    std::shared_ptr<cdk::basic_type> typeHint() {
      return _typeHint;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // og

#endif
