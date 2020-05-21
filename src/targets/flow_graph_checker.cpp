#include "targets/flow_graph_checker.h"
#include "ast/all.h" // automatically generated

// throw without loosing lineno information
#define THROW_ERROR(MSG) throw std::make_tuple<const cdk::basic_node*, std::string>(node, std::string(MSG))

void og::flow_graph_checker::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  _returning = false;
  node->block()->accept(this, lvl + 2);

  if (!_returning && !node->is_typed(cdk::TYPE_VOID)) {
    THROW_ERROR("function does not always return");
  }
}
void og::flow_graph_checker::do_return_node(og::return_node * const node, int lvl) {
  _returning = true;
  _jumping_in_cycle = true;
}

void og::flow_graph_checker::do_for_node(og::for_node * const node, int lvl) {
  _cycle_depth++;
  if (node->block()) {
    _returning = false;
    _jumping_in_cycle = false;
    node->block()->accept(this, lvl);
  }
  _cycle_depth--;
}
void og::flow_graph_checker::do_continue_node(og::continue_node * const node, int lvl) {
  if (_cycle_depth == 0) {
    THROW_ERROR("continue outside loop");
  }
  _jumping_in_cycle = true;
}
void og::flow_graph_checker::do_break_node(og::break_node * const node, int lvl) {
  if (_cycle_depth == 0) {
    THROW_ERROR("break outside loop");
  }
  _jumping_in_cycle = true;
}

void og::flow_graph_checker::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  _returning = false;
  _jumping_in_cycle = false;
  for (auto child : node->nodes()) {
    if (_returning) {
      THROW_ERROR("return must be the last instruction in a block");
    } else if (_jumping_in_cycle) {
      THROW_ERROR("break/continue must be the last instruction in a block");
    }

    child->accept(this, lvl + 2);
  }
}

void og::flow_graph_checker::do_block_node(og::block_node * const node, int lvl) {
  _returning = false;
  _jumping_in_cycle = false;
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
}

void og::flow_graph_checker::do_if_node(og::if_node * const node, int lvl) {
  node->block()->accept(this, lvl + 2);
  _returning = false; // may return early or not, doesn't count
  _jumping_in_cycle = false; // may jump early or not, doesn't count
}

void og::flow_graph_checker::do_if_else_node(og::if_else_node * const node, int lvl) {
  _returning = false;
  _jumping_in_cycle = false;
  node->thenblock()->accept(this, lvl + 2);
  bool then_was_returning = _returning;
  bool then_was_jumping_in_cycle = _jumping_in_cycle;

  if (node->elseblock()) {
    _returning = false; // check if else is returning
    _jumping_in_cycle = false; // check if else is jumping in cycle
    node->elseblock()->accept(this, lvl + 2);
  }

  // if is returning if both then and else are returning, same for jumping in cycle
  _returning = _returning && then_was_returning;
  _jumping_in_cycle = _jumping_in_cycle && then_was_jumping_in_cycle;
}

void og::flow_graph_checker::do_variable_declaration_node(og::variable_declaration_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_identity_node(og::identity_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_sizeof_node(og::sizeof_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_tuple_node(og::tuple_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_add_node(cdk::add_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_and_node(cdk::and_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_div_node(cdk::div_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_double_node(cdk::double_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_eq_node(cdk::eq_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_ge_node(cdk::ge_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_gt_node(cdk::gt_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_variable_node(cdk::variable_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_integer_node(cdk::integer_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_le_node(cdk::le_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_lt_node(cdk::lt_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_mod_node(cdk::mod_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_mul_node(cdk::mul_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_ne_node(cdk::ne_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_neg_node(cdk::neg_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_not_node(cdk::not_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_or_node(cdk::or_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_string_node(cdk::string_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_sub_node(cdk::sub_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_write_node(og::write_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_input_node(og::input_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_address_of_node(og::address_of_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_function_call_node(og::function_call_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_function_declaration_node(og::function_declaration_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_tuple_index_node(og::tuple_index_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  // EMPTY
}
void og::flow_graph_checker::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  // EMPTY
}
