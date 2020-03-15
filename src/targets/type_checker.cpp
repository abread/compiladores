#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void og::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void og::type_checker::do_return_node(og::return_node *const node, int lvl) {
  // TODO
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
  // EMPTY
}
void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  // EMPTY
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
  // TODO
#if 0
  node->type(new basic_type(4, basic_type::TYPE_POINTER));
#endif
}

//---------------------------------------------------------------------------

void og::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in Simple, expressions are always int
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void og::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

  // in Simple, expressions are always int
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
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

//---------------------------------------------------------------------------

void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void og::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void og::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  try {
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
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void og::type_checker::do_program_node(og::program_node *const node, int lvl) {
  // EMPTY
}

void og::type_checker::do_evaluation_node(og::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void og::type_checker::do_block_node(og::block_node * const node, int lvl) {
  // TODO
  // EMPTY (?)
}

void og::type_checker::do_write_node(og::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void og::type_checker::do_input_node(og::input_node *const node, int lvl) {
  // TODO: refactor into expression
}

//---------------------------------------------------------------------------


void og::type_checker::do_address_of_node(og::address_of_node * const node, int lvl) {
  // TODO
#if 0
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  if (node->lvalue()->type()->name() == basic_type::TYPE_DOUBLE) {
    node->type(new basic_type(4, basic_type::TYPE_POINTER));
  } else {
    throw std::string("wrong type in unary logical expression");
  }
#endif
}

//---------------------------------------------------------------------------

void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  // TODO: while -> for
#if 0
  node->condition()->accept(this, lvl + 4);
#endif
}

void og::type_checker::do_continue_node(og::continue_node * const node, int lvl) {
  // EMPTY
}

void og::type_checker::do_break_node(og::break_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void og::type_checker::do_if_node(og::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void og::type_checker::do_tuple_node(og::tuple_node *const node, int lvl) {
  // TODO
}
