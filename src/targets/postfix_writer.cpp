#include <string>
#include <sstream>
#include <functional>
#include "targets/type_checker.h"
#include "targets/frame_size_calculator.h"
#include "targets/postfix_writer.h"
#include "ast/all.h"  // all.h is automatically generated

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

#define ERROR(MSG) { std::cerr << (MSG) << std::endl; exit(1); }

static std::string fix_function_name(std::string name) {
  if (name == "og") {
    return "_main"; // entry point
  } else if (name == "_main") {
    return "._main"; // avoid naming conflict
  } else {
    return name;
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void og::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.NOT();
}
void og::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.LABEL(mklbl(lbl));
}
void og::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(node->value());
  } else {
    _pf.SINT(node->value());
  }
}

void og::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value());
  } else {
    _pf.SDOUBLE(node->value());
  }
}

void og::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  /* leave the address on the stack */
  if (_inFunctionBody) {
    _pf.TEXT(); // return to the TEXT segment
    _pf.ADDR(mklbl(lbl1)); // the string to be printed
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DNEG();
  } else {
    _pf.NEG();
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::processIDBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
}

void og::postfix_writer::processIDComparison(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  bool has_doubles = (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE));

  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if (node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (has_doubles) {
    _pf.DCMP();
    _pf.INT(0);
  }
}


void og::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3); //TODO: depends on sizeof referenced type
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3); //TODO: depends on sizeof referenced type
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  } else {
    _pf.ADD();
  }
}
void og::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO: are pointers just ints? maybe uints?

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3); //TODO: depends on sizeof referenced type
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DSUB();
  } else {
    _pf.SUB();
  }
}
void og::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.MUL();
  } else {
    _pf.DMUL();
  }
}
void og::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.DIV();
  } else {
    _pf.DDIV();
  }
}
void og::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void og::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.LT();
}
void og::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.LE();
}
void og::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.GE();
}
void og::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.GT();
}
void og::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.NE();
}
void og::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  processIDComparison(node, lvl);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_address_of_node(og::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO: HMMMMMMMMMMMMMM
  // since the argument is an lvalue, it is already an address
  node->lvalue()->accept(this, lvl + 2);
}

void og::postfix_writer::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);

  _pf.INT(cdk::reference_type_cast(node->type())->referenced()->size());
  _pf.MUL();
  _pf.ALLOC(); // allocate
  _pf.SP();// put base pointer in stack
}

void og::postfix_writer::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // a pointer is a 32-bit integer
  // TODO
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id); //TODO: new_symbol?

  if (symbol->global()) {
    _pf.ADDR(node->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void og::postfix_writer::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  auto reftype = cdk::reference_type_cast(node->base()->type());
  _pf.INT(reftype->referenced()->size());
  _pf.MUL();
  _pf.ADD(); // add pointer and index
}

void og::postfix_writer::load(std::shared_ptr<cdk::basic_type> type, std::function<void()> baseProducer, int offset) {
  if (type->name() == cdk::TYPE_INT || type->name() == cdk::TYPE_STRING || type->name() == cdk::TYPE_POINTER) {
    baseProducer();
    _pf.INT(offset);
    _pf.ADD();
    _pf.LDINT();
  } else if (type->name() == cdk::TYPE_DOUBLE) {
    baseProducer();
    _pf.INT(offset);
    _pf.ADD();
    _pf.LDDOUBLE();
  } else if (type->name() == cdk::TYPE_STRUCT) {
    auto structType = cdk::structured_type_cast(type);

    // we push the last element of a tuple first
    offset += structType->size();
    for (ssize_t i = structType->length() - 1; i >= 0; i--) {
      offset -= structType->component(i)->size();
      load(structType->component(i), baseProducer, offset);
    }
  } else {
    ERROR("ICE: Invalid type for rvalue node");
  }
}

void og::postfix_writer::load(cdk::expression_node *const node, int lvl, int tempOffset) {
    node->accept(this, lvl);

    if (node->is_typed(cdk::TYPE_STRUCT)) {
      int tuple_base_addr_location = tempOffset;
      if (tuple_base_addr_location == 0) {
        std::cerr << "ICE(postfix_writer): Node was not assigned exclusive temporary storage for load\n";
        exit(1);
      }

      _pf.LOCAL(tuple_base_addr_location);
      _pf.STINT();

      load(node->type(), [this, tuple_base_addr_location]() { _pf.LOCV(tuple_base_addr_location); });
    }
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << ";; rvalue_node start\n";
  if (node->lvalue()->is_typed(cdk::TYPE_STRUCT)) {
    // delay getting value
    node->lvalue()->accept(this, lvl);
  } else {
    load(node->type(), [node, this, lvl]() { node->lvalue()->accept(this, lvl); });
  }
  os() << ";; rvalue_node end\n";
}

void og::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); // determine the new value
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      _pf.I2D();
    }
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // typechecker takes care of declaring functions
  auto name = fix_function_name(node->identifier());

  _functions_to_declare.insert(name);
}

std::map<const cdk::basic_node*, int> calculate_unshared_temp_offsets(const og::frame_size_calculator &fsc) {
  std::map<const cdk::basic_node*, int> offsetTab;

  int offset = - fsc.localsize() - fsc.calltempsize() - fsc.returntempsize();
  for (auto& [node, tempsz] : fsc.unsharedTempSizeTab()) {
    offset -= tempsz;
    offsetTab[node] = offset;
  }

  return offsetTab;
}

void og::postfix_writer::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto sym = _symtab.find(node->identifier());
  _function = sym;
  _symtab.push();

  auto name = fix_function_name(node->identifier());
  _functions_to_declare.erase(name);

  _offset = 8;
  if (node->is_typed(cdk::TYPE_STRUCT)) {
    // account for hidden first argument (return write pointer)
    _offset += 4;
  }

  // declare args, and their respective scope
  _symtab.push();
  if (node->arguments()) {
    _inFunctionArgs = true;
    for (auto arg : node->arguments()->nodes()) {
      arg->accept(this, lvl);
    }
    _inFunctionArgs = false;
  }

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC) {
    _pf.GLOBAL(name, _pf.FUNC());
  }
  _pf.LABEL(name);

  frame_size_calculator fsc(_compiler, _function, _symtab);
  node->accept(&fsc, lvl);
  _callTempOffset = - fsc.localsize() - fsc.calltempsize();
  if (fsc.returntempsize())
    _returnTempOffset = _callTempOffset - fsc.returntempsize();
  _unsharedTempOffsetTab = calculate_unshared_temp_offsets(fsc);

  _pf.ENTER(fsc.localsize() + fsc.tempsize());

  _inFunctionBody = true;

  _offset = 0;
  os() << "        ;; before body " << std::endl;
  node->block()->accept(this, lvl);
  os() << "        ;; after body " << std::endl;
  _inFunctionBody = false;
  _symtab.pop(); //arguments
  // make sure that voids are returned from
  if (_function->is_typed(cdk::TYPE_VOID)) {
    _pf.LEAVE();
    _pf.RET();
  }
  _function = nullptr;
  _callTempOffset = 0;
  _returnTempOffset = 0;
  _unsharedTempOffsetTab.clear();


  // these are just a few library function imports
  if (name == "_main")  {
    for (std::string ext : _functions_to_declare)
      _pf.EXTERN(ext);
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto name = fix_function_name(node->identifier());
  std::shared_ptr<og::symbol> symbol = _symtab.find(node->identifier()); //TODO: new_symbol()?

  size_t argsSize = 0;
  auto argTypes = symbol->argsType()->components();

  bool return_struct = node->is_typed(cdk::TYPE_STRUCT);

  if (node->arguments()) {
    for (int ax = node->arguments()->size(); ax > 0; ax--) {

      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->element(ax - 1));

      if (return_struct && argTypes[ax - 1]->name() == cdk::TYPE_DOUBLE) {
        _pf.I2D(); // convert address to a double
      }
      // convert ints to doubles in arguments
      arg->accept(this, lvl + 2);
      if (arg->is_typed(cdk::TYPE_INT) && argTypes[ax - 1]->name() == cdk::TYPE_DOUBLE) {
        _pf.I2D();
        argsSize += argTypes[ax - 1]->size();
      } else {
        argsSize += arg->type()->size();
      }

    }
  }

  if (return_struct) {
    argsSize += 4;
    _pf.LOCAL(_callTempOffset);
  }
  _pf.CALL(name);
  if (argsSize != 0) {
    _pf.TRASH(argsSize); // leaves the SP if the return is a struct
  }

  if (return_struct) {
    _pf.LOCAL(_callTempOffset);
  }

  if (symbol->is_typed(cdk::TYPE_INT) || symbol->is_typed(cdk::TYPE_POINTER) || symbol->is_typed(cdk::TYPE_STRING)) {
    _pf.LDFVAL32();
  } else if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else if (symbol->is_typed(cdk::TYPE_VOID) || symbol->is_typed(cdk::TYPE_STRUCT)) {
    // EMPTY
  } else {
    // cannot happen!
    ERROR("ICE(postfix_writer): unsupported function return type");
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO: tuples
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->is_typed(cdk::TYPE_INT) || node->argument()->is_typed(cdk::TYPE_POINTER)) {
    _pf.TRASH(4); // delete the evaluated value
  } else if (node->argument()->is_typed(cdk::TYPE_STRING)) {
    _pf.TRASH(4); // delete the evaluated value's address
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8);
  } else if (node->argument()->is_typed(cdk::TYPE_VOID)) {
    // EMPTY
  } else {
    ERROR("ICE(postfix_writer/evaluation_node): unknown type for expression");
  }
}

void og::postfix_writer::do_block_node(og::block_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _symtab.push();
  if (node->declarations()) {
    node->declarations()->accept(this, lvl + 2);
  }
  if (node->instructions()) {
    node->instructions()->accept(this, lvl + 2);
  }
  _symtab.pop();
}

void og::postfix_writer::do_return_node(og::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (!_function->is_typed(cdk::TYPE_VOID)) {
    load(node->retval(), lvl, _returnTempOffset);
    if (_function->is_typed(cdk::TYPE_INT) || _function->is_typed(cdk::TYPE_STRING)
        || _function->is_typed(cdk::TYPE_POINTER)) {
      _pf.STFVAL32();
    } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      if (node->retval()->is_typed(cdk::TYPE_INT)) {
        _pf.I2D();
      }
      _pf.STFVAL64();
    } else if (_function->is_typed(cdk::TYPE_STRUCT)) {
      store(_function->type(), node->retval()->type(), [this]() { _pf.LOCV(8); });
    } else {
      // cannot happen!
      ERROR("ICE: unknown return type");
    }
  }
  _pf.LEAVE();
  _pf.RET();
}

void og::postfix_writer::do_write_node(og::write_node * const node, int lvl) {
  // TODO: handle newline flag and print EVERYTHING
  ASSERT_SAFE_EXPRESSIONS; //TODO: print structs
  load(node->argument(), lvl, tempOffsetForNode(node)); // determine the value to print

  for (auto node : node->argument()->elements()) {
    auto expr = static_cast<cdk::expression_node*>(node);

    if (expr->is_typed(cdk::TYPE_INT)) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (expr->is_typed(cdk::TYPE_DOUBLE)) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // delete the printed value
    } else if (expr->is_typed(cdk::TYPE_STRING)) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else {
      ERROR("ICE(postfix_writer/write_node): unkown type for write node");
    }
  }
  if (node->newline()) {
    _functions_to_declare.insert("println");
    _pf.CALL("println"); // print a newline
  }
}


//---------------------------------------------------------------------------

void og::postfix_writer::do_input_node(og::input_node * const node, int lvl) {
  // TODO: refactor to be an expression
  ASSERT_SAFE_EXPRESSIONS;
  _functions_to_declare.insert("readi"); //TODO: this
  _pf.CALL("readi");
  _pf.LDFVAL32();
  _pf.STINT();
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_for_node(og::for_node * const node, int lvl) {
  // HACK: typechecker declares initializer variables prematurely
  _symtab.push();
  ASSERT_SAFE_EXPRESSIONS;
  _symtab.pop();

  int lblini, lblincr, lblend;
  _forIni.push(lblini = ++_lbl);
  _forIncr.push(lblincr = ++_lbl);
  _forEnd.push(lblend = ++_lbl);

  if (node->initializers()) {
    node->initializers()->accept(this, lvl);
  }

  _pf.LABEL(mklbl(lblini));

  os() << "        ;; FOR conditions" << std::endl;
  if (node->conditions()) {
    load(node->conditions(), lvl, tempOffsetForNode(node));

    // multiple conditions
    size_t n_conditions = 1;

    if (node->conditions()->is_typed(cdk::TYPE_STRUCT)) {
      n_conditions = cdk::structured_type_cast(node->conditions()->type())->length();
    }

    for (size_t i = 0; i < n_conditions; i++) {
      _pf.JZ(mklbl(lblend));
    }
  }

  os() << "        ;; FOR block" << std::endl;
  if (node->block()) {
    node->block()->accept(this, lvl + 2);
  }

  os() << "        ;; FOR increments" << std::endl;
  _pf.LABEL(mklbl(lblincr));
  if (node->increments()) {
    node->increments()->accept(this, lvl);
  }

  _pf.JMP(mklbl(lblini));
  _pf.LABEL(mklbl(lblend));

  _forIni.pop();
  _forIncr.pop();
  _forEnd.pop();
}

void og::postfix_writer::do_continue_node(og::continue_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forIncr.top())); // jump to next cycle
  } else {
    ERROR("continue outside 'for' loop");
  }
}

void og::postfix_writer::do_break_node(og::break_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forEnd.top())); // jump to for end
  } else {
    ERROR("break outside 'for' loop");
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_if_node(og::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_if_else_node(og::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

void og::postfix_writer::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto elements = node->elements();

  os() << ";; tuple_node start\n";
  if (_inFunctionBody) {
    if (node->is_typed(cdk::TYPE_STRUCT)) {
      // alocate space in BSS for the whole tuple
      int tuple_base_addr = tempOffsetForNode(node);
      if (tuple_base_addr == 0) {
        std::cerr << "ICE(postfix_writer): Node was not assigned exclusive temporary storage for tuple_node\n";
        exit(1);
      }

      os() << ";; tuple_node load start\n";
      for (auto it = elements.rbegin(); it != elements.rend(); it++) {
        (*it)->accept(this, lvl);
      }
      os() << ";; tuple_node load end; store start\n";
      store(node->type(), node->type(), [this, tuple_base_addr]() { _pf.LOCAL(tuple_base_addr); });

      // leave base address in stack
      _pf.LOCAL(tuple_base_addr);
    } else {
      // only 1 element
      node->element(0)->accept(this, lvl);
    }
  } else {
    for (auto it = elements.begin(); it != elements.end(); it++) {
      (*it)->accept(this, lvl);
    }
  }
  os() << ";; tuple_node end\n";
}

void og::postfix_writer::set_declaration_offsets(og::variable_declaration_node * const node) {
  std::shared_ptr<og::symbol> symbol = new_symbol();
  int offset, typesize = node->type()->size();

  if (_inFunctionArgs) {
    offset = _offset;
    _offset += typesize;
  } else if (_inFunctionBody) {
    auto ids = node->identifiers();

    if (!node->initializer() || ids.size() == 1) {
      _offset -= typesize;
      offset = _offset;
    } else {
      // cases like auto a, b = 1, 2.3
      auto rvalueType = cdk::structured_type_cast(node->initializer()->type());

      for (size_t ix = 0; ix < ids.size(); ix++) {
        std::string& id = ids[ix];
        auto compType = rvalueType->component(ix);
        symbol = _symtab.find(id);
        _offset -= compType->size();
        if (symbol) {
          symbol->set_offset(_offset);
        } else {
          ERROR("SHOULD NOT HAPPEN");
        }
      }
      reset_new_symbol();
      return;
    }
  } else { // global
    offset = 0;
  }

  if (symbol) {
    symbol->set_offset(offset);
    reset_new_symbol();
  } else {
    ERROR("SHOULD NOT HAPPEN");
  }
}

void og::postfix_writer::store(std::shared_ptr<cdk::basic_type> lvalType, std::shared_ptr<cdk::basic_type> rvalType, std::function<void()> baseSupplier, int offset) {
  if (lvalType->name() == cdk::TYPE_INT || lvalType->name() == cdk::TYPE_STRING || lvalType->name() == cdk::TYPE_POINTER) {
    baseSupplier();
    _pf.INT(offset);
    _pf.ADD();
    _pf.STINT();
  } else if (lvalType->name() == cdk::TYPE_DOUBLE) {
    if (rvalType->name() == cdk::TYPE_INT) {
      _pf.I2D();
    }

    baseSupplier();
    _pf.INT(offset);
    _pf.ADD();
    _pf.STDOUBLE();
  } else if (lvalType->name() == cdk::TYPE_STRUCT) {
    if (rvalType->name() != cdk::TYPE_STRUCT) ERROR("typechecker is dumb");

    auto lvalStructType = cdk::structured_type_cast(lvalType);
    auto rvalStructType = cdk::structured_type_cast(rvalType);

    for (size_t i = 0; i < lvalStructType->length(); i++) {
       store(lvalStructType->component(i), rvalStructType->component(i), baseSupplier, offset);
       offset += lvalStructType->component(i)->size();
    }
  } else {
    ERROR("cannot initialize");
  }
}

void og::postfix_writer::define_global_variable(const std::string& id, cdk::expression_node * init, int qualifier, int lvl) {
  if (qualifier == tREQUIRE) {
    _pf.EXTERN(id);
    return;
  }

  std::shared_ptr<symbol> symbol = _symtab.find(id);

  _pf.DATA();
  _pf.ALIGN();
  if (qualifier == tPUBLIC) {
    _pf.GLOBAL(id, _pf.OBJ());
  }
  _pf.LABEL(id);
  if (init) {
    if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
      if (init->is_typed(cdk::TYPE_DOUBLE)) {
        init->accept(this, lvl);
      } else if (init->is_typed(cdk::TYPE_INT)) {
        // allocate a double if the type is double
        cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(init);
        if (dclini == nullptr) ERROR("only literals are allowed in global variable initializers");

        cdk::double_node ddi(dclini->lineno(), dclini->value());
        ddi.accept(this, lvl);
      } else {
        ERROR("bad initializer for real value");
      }
    } else {
      init->accept(this, lvl);
    }
  }
}

void og::postfix_writer::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // declare variable with typechecker

  auto ids = node->identifiers();

  set_declaration_offsets(node);
  // do nothing with function arguments
  if (_inFunctionArgs) {
    return;
  }

  if (node->initializer()) {
    if (_inFunctionBody) {
      load(node->initializer(), lvl, tempOffsetForNode(node));

      if (ids.size() == 1) {
        // we have 1 id to an expression (e.g.: auto a = 1, 2, 3)
        auto id = ids[0];
        auto type = node->initializer()->type();
        std::shared_ptr<symbol> symbol = _symtab.find(id);

        store(symbol->type(), type, [this, symbol](){ _pf.LOCAL(symbol->offset()); });
      } else {
        // we have n-ids to a n-tuple expression (e.g.: auto a, b, c = 1, 2, 3)

        auto rvalType = cdk::structured_type_cast(node->initializer()->type());

        for (size_t ix = 0; ix < ids.size(); ix++) {
          auto type = rvalType->component(ix);
          auto id = ids[ix];
          std::shared_ptr<symbol> symbol = _symtab.find(id);

          store(symbol->type(), type, [this, symbol](){ _pf.LOCAL(symbol->offset()); });
        }
      }
    } else {
      if (ids.size() == 1) {
        // we have 1 id to a tuple (e.g.: auto a = 1, 2, 3)
        define_global_variable(ids[0], node->initializer(), node->qualifier(), lvl);
      } else {
        // we have n-ids to a n-tuple expression (e.g.: auto a, b, c = 1, 2, 3)
        auto tuple_initializer = dynamic_cast<og::tuple_node *>(node->initializer());

        for (size_t ix = 0; ix < ids.size(); ix++) {
          auto id = node->identifiers()[ix];
          auto init = tuple_initializer->element(ix);
          define_global_variable(id, init, node->qualifier(), lvl);
        }
      }
    }
  } else {
    // tuples have to be initialized, so we can pick the first id
    auto id = ids[0];
    if (!_inFunctionBody) {
      if (node->qualifier() == tREQUIRE) {
        _pf.EXTERN(id);
      } else {
        _pf.BSS();
        _pf.ALIGN();
        _pf.SALLOC(node->type()->size());
        if (node->qualifier() == tPUBLIC) {
          _pf.GLOBAL(id, _pf.OBJ());
        }
        _pf.LABEL(id);
      }
    }
  }
}

void og::postfix_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  auto components = (cdk::structured_type_cast(node->base()->type()))->components();

  int offset = 0;
  for (size_t ix = 0; ix < node->index()-1; ix++) {
    offset += components[ix]->size();
  }
  _pf.INT(offset);
  _pf.ADD();
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->arguments()->type()->size());
}

void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}
