#include <string>
#include "targets/xml_writer.h"
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated

//---------------------------------------------------------------------------

void og::xml_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::xml_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void og::xml_writer::do_not_node(cdk::not_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void og::xml_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  os() << std::string(lvl, ' ') << "<sequence_node size='" << node->size() << "'>" << std::endl;
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  process_literal(node, lvl);
}

void og::xml_writer::do_string_node(cdk::string_node * const node, int lvl) {
  process_literal(node, lvl);
}
void og::xml_writer::do_double_node(cdk::double_node * const node, int lvl) {
  process_literal(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_unary_operation(cdk::unary_operation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  do_unary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_binary_operation(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->left()->accept(this, lvl + LVL_INCR);
  node->right()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_add_node(cdk::add_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_div_node(cdk::div_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_le_node(cdk::le_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_and_node(cdk::and_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}
void og::xml_writer::do_or_node(cdk::or_node * const node, int lvl) {
  do_binary_operation(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  //TODO: I guess this is fine but TODO anyway
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->name() << "</" << node->label() << ">" << std::endl;
}

void og::xml_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->lvalue()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  //TODO: multiple assignment?
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);

  node->lvalue()->accept(this, lvl + LVL_INCR);
  reset_new_symbol();

  node->rvalue()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  // TODO: HMMMMM why was this here? Only visit the block? Maybe a definition isn't also a declaration afterall?
  openTag(node, lvl);
  node->block()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<function_call_node identifier='" << node->identifier() << "'>" << std::endl;

  if (node->arguments()) {
    openTag("arguments", lvl + LVL_INCR);
    node->arguments()->accept(this, lvl + 2*LVL_INCR);
    closeTag("arguments", lvl + LVL_INCR);
  }

  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<function_declaration_node qualifier='" << node->qualifier() << "' identifier='" << node->identifier() << "'>" << std::endl;
  if (node->arguments()) { //TODO: empty sequence vs nullptr again
    openTag("arguments", lvl + LVL_INCR);
    node->arguments()->accept(this, lvl + 2*LVL_INCR);
    closeTag("arguments", lvl + LVL_INCR);
  }
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_block_node(og::block_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; //TODO: empty sequences instead of nullptr?
  openTag(node, lvl);
  if (node->declarations()) {
    openTag("declarations", lvl + LVL_INCR);
    node->declarations()->accept(this, lvl + 2*LVL_INCR);
    closeTag("declarations", lvl + LVL_INCR);
  }
  if (node->instructions()) {
    openTag("instructions", lvl + LVL_INCR);
    node->instructions()->accept(this, lvl + 2*LVL_INCR);
    closeTag("instructions", lvl + LVL_INCR);
  }
  closeTag(node, lvl);
}


void og::xml_writer::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("base", lvl + LVL_INCR);
  node->base()->accept(this, lvl + 2*LVL_INCR);
  closeTag("base", lvl);
  openTag("index", lvl + LVL_INCR);
  node->index()->accept(this, lvl + 2*LVL_INCR);
  closeTag("index", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_address_of_node(og::address_of_node * const node, int lvl) {
  //TODO: unary operation?
  //do_unary_operation(node, lvl);
}

void og::xml_writer::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  node->argument()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; //TODO: Not a literal?
  openTag(node, lvl);
  closeTag(node, lvl);
}

void og::xml_writer::do_write_node(og::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<write_node newline='" << ((node->newline()) ? "true" : "false") << "'>" << std::endl;
  node->argument()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_input_node(og::input_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_for_node(og::for_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);

  if (node->initializers()) {
    openTag("initializers", lvl + LVL_INCR);
    node->initializers()->accept(this, lvl + 2*LVL_INCR);
    closeTag("initializers", lvl + LVL_INCR);
  }

  if (node->conditions()) {
    openTag("conditions", lvl + LVL_INCR);
    node->conditions()->accept(this, lvl + 2*LVL_INCR);
    closeTag("conditions", lvl + LVL_INCR);
  }

  if (node->increments()) {
    openTag("increments", lvl + LVL_INCR);
    node->increments()->accept(this, lvl + 2*LVL_INCR);
    closeTag("increments", lvl + LVL_INCR);
  }

  openTag("block", lvl + LVL_INCR);
  node->block()->accept(this, lvl + 2*LVL_INCR);
  closeTag("block", lvl + LVL_INCR);
  closeTag(node, lvl);
}


void og::xml_writer::do_break_node(og::break_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  closeTag(node, lvl);
}

void og::xml_writer::do_continue_node(og::continue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  closeTag(node, lvl);
}

void og::xml_writer::do_return_node(og::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  if (node->retval())
    node->retval()->accept(this, lvl + LVL_INCR);
  closeTag(node, lvl);
}

//---------------------------------------------------------------------------

void og::xml_writer::do_if_node(og::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + LVL_INCR);
  node->condition()->accept(this, lvl + 2*LVL_INCR);
  closeTag("condition", lvl + LVL_INCR);
  openTag("then", lvl + LVL_INCR);
  node->block()->accept(this, lvl + 2*LVL_INCR);
  closeTag("then", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_if_else_node(og::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("condition", lvl + LVL_INCR);
  node->condition()->accept(this, lvl + 2*LVL_INCR);
  closeTag("condition", lvl + LVL_INCR);
  openTag("then", lvl + LVL_INCR);
  node->thenblock()->accept(this, lvl + 2*LVL_INCR);
  closeTag("then", lvl + LVL_INCR);
  openTag("else", lvl + LVL_INCR);
  node->elseblock()->accept(this, lvl + 2*LVL_INCR);
  closeTag("else", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  openTag(node, lvl);
  openTag("elements", lvl + LVL_INCR);
  node->elements()->accept(this, lvl + 2*LVL_INCR);
  closeTag("elements", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; //TODO: a bit dirty no? And maybe get qualifier name somehow?
  os() << std::string(lvl, ' ') << "<variable_declaration_node qualifier='" << node->qualifier() << "'>" << std::endl;
  openTag("identifiers", lvl + LVL_INCR);
  for (const std::string& identifier : node->identifiers()) {
    os() << std::string(lvl + 2*LVL_INCR, ' ') << "<identifier>" << identifier << "</identifier>" << std::endl;
  }
  closeTag("identifiers", lvl + LVL_INCR);
  if (node->initializer()) {
    openTag("initializer", lvl + LVL_INCR);
    node->initializer()->accept(this, lvl + 2*LVL_INCR);
    closeTag("initializer", lvl + LVL_INCR);
  }
  closeTag(node, lvl);
}

void og::xml_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << std::string(lvl, ' ') << "<tuple_index_node index='" << node->index() << "'>" << std::endl;
  openTag("base", lvl + LVL_INCR);
  node->base()->accept(this, lvl + 2*LVL_INCR);
  closeTag("base", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  openTag(node, lvl);
  openTag("arguments", lvl + LVL_INCR);
  node->arguments()->accept(this, lvl + 2*LVL_INCR);
  closeTag("arguments", lvl + LVL_INCR);
  closeTag(node, lvl);
}

void og::xml_writer::do_identity_node(og::identity_node *const node, int lvl) {
  do_unary_operation(node, lvl);
}
