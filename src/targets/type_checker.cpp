#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void og::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (auto el : node->nodes()) {
    el->accept(this, lvl + 2);
  }
}

//---------------------------------------------------------------------------

void og::type_checker::do_return_node(og::return_node *const node, int lvl) {
  // TODO
  if (node->retval())
    node->retval()->accept(this, lvl + 2);
#if 0
  if (node->retval()) {
    if (_function->type()->name() == basic_type::TYPE_VOID) throw std::string("initializer specified for void function.");

    node->retval()->accept(this, lvl + 2);

    std::cout << "FUNCT TYPE" << _function->type()->name()  << std::endl;
    std::cout << "RETVAL TYPE" << node->retval()->type()->name() << std::endl;

    if (_function->type()->name() == basic_type::TYPE_INT) {
      if (node->retval()->type()->name() != basic_type::TYPE_INT) throw std::string(
          "wrong type for initializer (integer expected).");
    } else if (_function->type()->name() == basic_type::TYPE_DOUBLE) {
      if (node->retval()->type()->name() != basic_type::TYPE_INT
          && node->retval()->type()->name() != basic_type::TYPE_DOUBLE) throw std::string(
          "wrong type for initializer (integer or double expected).");
    } else if (_function->type()->name() == basic_type::TYPE_STRING) {
      if (node->retval()->type()->name() != basic_type::TYPE_STRING) throw std::string(
          "wrong type for initializer (string expected).");
    } else if (_function->type()->name() == basic_type::TYPE_POINTER) {
      //DAVID: FIXME: trouble!!!
      int ft = 0, rt = 0;
      basic_type *ftype = _function->type();
      for (; ftype->name() == basic_type::TYPE_POINTER; ft++, ftype = ftype->_subtype);
      basic_type *rtype = node->retval()->type();
      for (; rtype && rtype->name() == basic_type::TYPE_POINTER; rt++, rtype = rtype->_subtype);

    std::cout << "FUNCT TYPE"  << _function->type()->name()      << " --- " << ft << " -- " << ftype->name() << std::endl;
    std::cout << "RETVAL TYPE" << node->retval()->type()->name() << " --- " << rt << " -- " << rtype << std::endl;

      bool compatible = (ft == rt) && (rtype == 0 || (rtype != 0 && ftype->name() == rtype->name()));
      if (!compatible) throw std::string("wrong type for return expression (pointer expected).");

    } else {
      throw std::string("unknown type for initializer.");
    }
  }
#endif
}

//---------------------------------------------------------------------------

void og::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
}

//---------------------------------------------------------------------------

void og::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_STRING));
}

void og::type_checker::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_POINTER));
}

//---------------------------------------------------------------------------

void og::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(node->argument()->type());
}

void og::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void og::type_checker::do_identity_node(og::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void og::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  // TODO
}

void og::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol) {
    node->type(symbol->type());
  } else {
    // TODO: create variable symbols
    throw std::string("undefined variable: "+id);
  }
}

void og::type_checker::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);

  if (! node->base()->is_typed(cdk::TYPE_POINTER)) {
    throw std::string("pointer expression expected in pointer indexing");
  }

  if (! node->index()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expected in index");
  }

  auto ref = cdk::reference_type_cast(node->base()->type());
  node->type(ref->referenced());
  // TODO
#if 0
  if (node->base()) {
    node->base()->accept(this, lvl + 2);
    if (node->base()->type()->name() != basic_type::TYPE_POINTER) throw std::string(
        "pointer expression expected in index left-value");
  } else {
    if (_function->type()->name() != basic_type::TYPE_POINTER) throw std::string(
        "return pointer expression expected in index left-value");
  }
  node->index()->accept(this, lvl + 2);
  if (node->index()->type()->name() != basic_type::TYPE_INT) throw std::string("integer expression expected in left-value index");
  node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
#endif
}

void og::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  /*try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }*/
}

void og::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);
  // TODO

  /*try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = std::make_shared<og::symbol>(cdk::make_primitive_type(4, cdk::TYPE_INT), id, 0);
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);  //DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in Simple, expressions are always int
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));*/
}

//---------------------------------------------------------------------------

void og::type_checker::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  // TODO
  if (node->qualifier() == tREQUIRE)
    throw std::string("can't require a function definition");

  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 2);
  }

  node->block()->accept(this, lvl + 2);
}


void og::type_checker::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  // TODO
  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 2);
  }

  // temporary until we properly use the function symbol table
  node->type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
}

void og::type_checker::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  // TODO
  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 2);
  }
}

void og::type_checker::do_evaluation_node(og::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void og::type_checker::do_block_node(og::block_node * const node, int lvl) {
  if (node->declarations()) {
    node->declarations()->accept(this, lvl + 2);
  }

  if (node->instructions()) {
    node->instructions()->accept(this, lvl + 2);
  }
}

void og::type_checker::do_write_node(og::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void og::type_checker::do_input_node(og::input_node *const node, int lvl) {
  ASSERT_UNSPEC;
  // TODO
  node->type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------


void og::type_checker::do_address_of_node(og::address_of_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::make_reference_type(4, node->lvalue()->type()));
  // TODO: confirm
#if 0
  if (node->lvalue()->type()->name() == basic_type::TYPE_DOUBLE) {
    node->type(new basic_type(4, basic_type::TYPE_POINTER));
  } else {
    throw std::string("wrong type in unary logical expression");
  }
#endif
}

void og::type_checker::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(0, cdk::TYPE_UNSPEC)));
  // TODO
#if 0
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->type()->name() != basic_type::TYPE_INT) throw std::string(
      "integer expression expected in allocation expression");
//FIXME: check the following two lines
  auto mytype = new basic_type(4, basic_type::TYPE_POINTER);
  mytype->_subtype = new basic_type(8, basic_type::TYPE_DOUBLE);
  node->type(mytype);
#endif
}

//---------------------------------------------------------------------------

void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  if (node->initializers()) {
    node->initializers()->accept(this, lvl + 2);
  }

  if (node->conditions()) {
    node->conditions()->accept(this, lvl + 2);
  }

  if (node->increments()) {
    node->increments()->accept(this, lvl + 2);
  }

  node->block()->accept(this, lvl + 2);
}

void og::type_checker::do_continue_node(og::continue_node * const node, int lvl) {
  // EMPTY
}

void og::type_checker::do_break_node(og::break_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void og::type_checker::do_if_node(og::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);
}

void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);
}

void og::type_checker::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->seq()->accept(this, lvl + 2);

  std::vector<std::shared_ptr<cdk::basic_type>> el_types;
  for (size_t i = 0; i < node->elements().size(); i++) {
    cdk::expression_node* el = node->element(i);
    el_types.push_back(el->type());
  }

  node->type(cdk::make_structured_type(el_types));
}

void og::type_checker::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  ASSERT_UNSPEC;
  if (node->initializer() && node->qualifier() == tREQUIRE)
    throw std::string("external(required) variables cannot be initialized");

  // TODO
}

void og::type_checker::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);

  if (! node->base()->is_typed(cdk::TYPE_STRUCT))
    throw std::string("tuple index base is not a tuple");

  // TODO
  node->type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
}

void og::type_checker::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->arguments()->accept(this, lvl + 2);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
