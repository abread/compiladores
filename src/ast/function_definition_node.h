#ifndef __OG_AST_FUNCTION_DEFINITION_H__
#define __OG_AST_FUNCTION_DEFINITION_H__

#include <string>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/primitive_type.h>
#include "ast/block_node.h"

namespace og {

  /**
   * Class for describing function definitions.
   */
  class function_definition_node: public cdk::basic_node {
    int _qualifier;
    cdk::basic_type *_type;
    std::string _identifier;
    cdk::sequence_node *_arguments;
    og::block_node *_block;

  public:
    function_definition_node(int lineno, int qualifier, const std::string &identifier, cdk::sequence_node *arguments, og::block_node *block) :
        cdk::basic_node(lineno),
        _qualifier(qualifier),
        _type(new cdk::primitive_type(0, cdk::typename_type::TYPE_VOID)),
        _identifier(identifier),
        _arguments(arguments),
        _block(block) {}

    function_definition_node(int lineno, int qualifier, cdk::basic_type *type, const std::string &identifier, cdk::sequence_node *arguments, og::block_node *block) :
        cdk::basic_node(lineno),
        _qualifier(qualifier),
        _type(type),
        _identifier(identifier),
        _arguments(arguments),
        _block(block) {}

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
    og::block_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // og

#endif
