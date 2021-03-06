#include <string>
#include "targets/frame_size_calculator.h"
#include "targets/type_checker.h"
#include "targets/symbol.h"
#include "ast/all.h"

og::frame_size_calculator::~frame_size_calculator() {
  os().flush();
}

void og::frame_size_calculator::load_value(cdk::typed_node * const lval_or_expr, int lvl, cdk::basic_node const * const caller) {
  bool old = _needTupleAddr;
  _needTupleAddr = false;
  _evaledTupleAddr = true; // assume the worst
  lval_or_expr->accept(this, lvl);
  _needTupleAddr = old;

  if (lval_or_expr->is_typed(cdk::TYPE_STRUCT) && _evaledTupleAddr) {
    _unsharedTempSizeTab[caller] = 4; // will need to store a pointer to the top of the tuple
  }
}

void og::frame_size_calculator::do_add_node(cdk::add_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_and_node(cdk::and_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl);
}
void og::frame_size_calculator::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_div_node(cdk::div_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_double_node(cdk::double_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_eq_node(cdk::eq_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_ge_node(cdk::ge_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_gt_node(cdk::gt_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_variable_node(cdk::variable_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_integer_node(cdk::integer_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_le_node(cdk::le_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_lt_node(cdk::lt_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_mod_node(cdk::mod_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_mul_node(cdk::mul_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_ne_node(cdk::ne_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_neg_node(cdk::neg_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::frame_size_calculator::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_not_node(cdk::not_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::frame_size_calculator::do_or_node(cdk::or_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_needTupleAddr) {
    node->lvalue()->accept(this, lvl);
  } else {
    load_value(node->lvalue(), lvl, node);
  }

  if (node->lvalue()->is_typed(cdk::TYPE_STRUCT)) {
    _evaledTupleAddr = _needTupleAddr;
  }

}
void og::frame_size_calculator::do_string_node(cdk::string_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_sub_node(cdk::sub_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::frame_size_calculator::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  bool old = _needTupleAddr;
  _needTupleAddr = false;
  node->argument()->accept(this, lvl);
  _needTupleAddr = old;
}
void og::frame_size_calculator::do_write_node(og::write_node * const node, int lvl) {
  // we don't need space for the whole tuple, just for each argument (possibly)
  for (auto el : node->argument()->elements()) {
    el->accept(this, lvl);
  }
}
void og::frame_size_calculator::do_input_node(og::input_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_address_of_node(og::address_of_node * const node, int lvl) {
  node->lvalue()->accept(this, lvl);
}
void og::frame_size_calculator::do_function_call_node(og::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->arguments()) {
    // we don't need space for the whole tuple, just for each argument (possibly)
    for (auto el : node->arguments()->elements()) {
      el->accept(this, lvl);
    }
  }

  if (node->is_typed(cdk::TYPE_STRUCT)) {
    _calltempsize = std::max(_calltempsize, node->type()->size());
  }
}
void og::frame_size_calculator::do_function_declaration_node(og::function_declaration_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_tuple_index_node(og::tuple_index_node * const node, int lvl) {
  bool old = _needTupleAddr;
  _needTupleAddr = true;
  node->base()->accept(this, lvl);
  _needTupleAddr = old;

  if (node->is_typed(cdk::TYPE_STRUCT)) {
    _evaledTupleAddr = true;
  }
}
void og::frame_size_calculator::do_continue_node(og::continue_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_return_node(og::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->retval()) {
    load_value(node->retval(), lvl, node);
  }
}
void og::frame_size_calculator::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::frame_size_calculator::do_break_node(og::break_node * const node, int lvl) {
  // EMPTY
}
void og::frame_size_calculator::do_identity_node(og::identity_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::frame_size_calculator::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
}
void og::frame_size_calculator::do_sizeof_node(og::sizeof_node * const node, int lvl) {
  // EMPTY
  // expressions in sizeof are not evaluated
}
void og::frame_size_calculator::do_tuple_node(og::tuple_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->size() > 1) { // implies TYPE_STRUCT, when size == 1 we can just pass whatever's inside as is
    bool old = _needTupleAddr;
    _needTupleAddr = false;

    // if tuple contains tuples, it will need space to store a pointer to the start of the inner tuple
    bool needsBaseAddrExtra = false;
    for (auto el : node->elements()) {
      auto expr = static_cast<cdk::expression_node*>(el);

      _evaledTupleAddr = true; // assume the worst
      expr->accept(this, lvl);

      if (expr->is_typed(cdk::TYPE_STRUCT) && _evaledTupleAddr) {
        needsBaseAddrExtra = true;
      }
    }

    _needTupleAddr = old;

    size_t sz = 0;
    if (_needTupleAddr)
      sz += node->type()->size();
    if (needsBaseAddrExtra)
      sz += 4;

    if (sz)
      _unsharedTempSizeTab[node] = sz;

    _evaledTupleAddr = _needTupleAddr;
  } else {
    node->element(0)->accept(this, lvl);
  }
}

void og::frame_size_calculator::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (auto child : node->nodes()) {
    child->accept(this, lvl + 2);
  }
}

void og::frame_size_calculator::do_block_node(og::block_node * const node, int lvl) {
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
}

void og::frame_size_calculator::do_for_node(og::for_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->initializers()) {
    node->initializers()->accept(this, lvl);
  }

  if (node->condition()->is_typed(cdk::TYPE_STRUCT)) {
    load_value(node->condition(), lvl, node);
  }

  if (node->block()) {
    node->block()->accept(this, lvl + 2);
  }

  if (node->increments()) {
    node->increments()->accept(this, lvl + 2);
  }
}

void og::frame_size_calculator::do_if_node(og::if_node * const node, int lvl) {
  node->condition()->accept(this, lvl);
  node->block()->accept(this, lvl + 2);
}

void og::frame_size_calculator::do_if_else_node(og::if_else_node * const node, int lvl) {
  node->condition()->accept(this, lvl);
  node->thenblock()->accept(this, lvl + 2);
  if (node->elseblock()) node->elseblock()->accept(this, lvl + 2);
}

void og::frame_size_calculator::do_variable_declaration_node(og::variable_declaration_node * const node, int lvl) {
  _symtab.push(); // isolate any declarations from the outside world
  ASSERT_SAFE_EXPRESSIONS;
  _symtab.pop();

  _localsize += node->type()->size();

  if (node->initializer()) {
    load_value(node->initializer(), lvl, node);
  }
}

void og::frame_size_calculator::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  //ASSERT_SAFE_EXPRESSIONS; // writer already called it

  node->block()->accept(this, lvl + 2);

  if (node->is_typed(cdk::TYPE_STRUCT)) {
    _returntempsize = 4; // will need to store a pointer to the top of the tuple
  }
}
