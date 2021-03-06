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
    auto reftype = cdk::reference_type_cast(node->type());
    _pf.INT(reftype->referenced()->size());
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto reftype = cdk::reference_type_cast(node->type());
    _pf.INT(reftype->referenced()->size());
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  } else {
    _pf.ADD();
  }
}
void og::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto reftype = cdk::reference_type_cast(node->type());
    _pf.INT(reftype->referenced()->size());
    _pf.MUL();
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
  node->lvalue()->accept(this, lvl + 2);
}

void og::postfix_writer::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);

  size_t elsize = cdk::reference_type_cast(node->type())->referenced()->size();
  if (elsize) {
    _pf.INT(elsize);
    _pf.MUL();
  }
  _pf.ALLOC(); // allocate
  _pf.SP();// put base pointer in stack
}

void og::postfix_writer::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
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
  auto symbol = _symtab.find(id);

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
  if (reftype->referenced()->size() != 1) {
    _pf.INT(reftype->referenced()->size());
    _pf.MUL();
  }
  _pf.ADD(); // add pointer and index
}

/*
loads a potentially complex (tuple) value to the top of the stack from an address
this base addressed is emitted by the baseProducer, which is called for each primitive type loaded
baseProducer must leave the stack unchanged apart from the pushed base address
*/
void og::postfix_writer::load_from_base(std::shared_ptr<cdk::basic_type> type, std::function<void()> baseProducer, int offset) {
  if (type->name() == cdk::TYPE_INT || type->name() == cdk::TYPE_STRING || type->name() == cdk::TYPE_POINTER) {
    baseProducer();
    if (offset) {
      _pf.INT(offset);
      _pf.ADD();
    }
    _pf.LDINT();
  } else if (type->name() == cdk::TYPE_DOUBLE) {
    baseProducer();
    if (offset) {
      _pf.INT(offset);
      _pf.ADD();
    }
    _pf.LDDOUBLE();
  } else if (type->name() == cdk::TYPE_STRUCT) {
    auto structType = cdk::structured_type_cast(type);

    offset += structType->size();
    for (size_t i = 0; i < structType->length(); i++) {
      offset -= structType->component(i)->size();
      load_from_base(structType->component(i), baseProducer, offset);
    }
  } else {
    ERROR("ICE: Invalid type for rvalue node");
  }
}

/*
also loads a potentially complex (tuple) value to the top of the stack from a lval or expression node
it is assumed the node, if is_typed(cdk::TYPE_STRUCT), follows _needTupleAddr/_evaledTupleAddr conventions.

the node will be visited, and, if a struct base address is pushed to the stack instead of its value:
- the base address will be stored in FP-tempOffset (because re-visiting the node may evaluate a tuple expression again), and removed from the stack
- the tuple will be pushed to the stack (using the stored base address)

FP-tempOffset should be unshared temporary storage (assigned by og::frame_size_calculator)
*/
void og::postfix_writer::load(cdk::typed_node *const lval_or_expr, int lvl, int tempOffset) {
  bool old = _needTupleAddr;
  _needTupleAddr = false;
  _evaledTupleAddr = true; // assume the worst
  lval_or_expr->accept(this, lvl);
  _needTupleAddr = old;

  if (lval_or_expr->is_typed(cdk::TYPE_STRUCT) && _evaledTupleAddr) {
    int tuple_base_addr_location = tempOffset;
    if (tuple_base_addr_location == 0) {
      std::cerr << "ICE(postfix_writer): Node was not assigned exclusive temporary storage for load\n";
      exit(1);
    }

    _pf.LOCAL(tuple_base_addr_location);
    _pf.STINT();

    load_from_base(lval_or_expr->type(), [this, tuple_base_addr_location]() { _pf.LOCV(tuple_base_addr_location); });
  }
  _evaledTupleAddr = false;
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  os() << ";; rvalue_node start\n";
  if (node->lvalue()->is_typed(cdk::TYPE_STRUCT)) {
    if (_needTupleAddr) {
      // delay getting value
      node->lvalue()->accept(this, lvl);
      _evaledTupleAddr = true;
    } else {
      load(node->lvalue(), lvl, tempOffsetForNode(node));
      _evaledTupleAddr = false;
    }
  } else {
    load_from_base(node->type(), [node, this, lvl]() { node->lvalue()->accept(this, lvl); });
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

  _extern_functions.insert(name);
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
  _extern_functions.erase(name);

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
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto name = fix_function_name(node->identifier());
  std::shared_ptr<og::symbol> symbol = _symtab.find(node->identifier());

  size_t argsSize = 0;
  auto argTypes = symbol->argsType()->components();

  bool return_struct = node->is_typed(cdk::TYPE_STRUCT);

  if (node->arguments()) {
    for (int ax = node->arguments()->size(); ax > 0; ax--) {
      cdk::expression_node * arg = node->arguments()->element(ax - 1);
      // convert ints to doubles in arguments
      arg->accept(this, lvl + 2);
      if (arg->is_typed(cdk::TYPE_INT) && argTypes[ax - 1]->name() == cdk::TYPE_DOUBLE) {
        _pf.I2D();
      }
    }
    argsSize += symbol->argsType()->size();
  }

  if (return_struct) {
    argsSize += 4;
    _pf.LOCAL(_callTempOffset);
  }
  _pf.CALL(name);
  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }

  if (symbol->is_typed(cdk::TYPE_INT) || symbol->is_typed(cdk::TYPE_POINTER) || symbol->is_typed(cdk::TYPE_STRING)) {
    _pf.LDFVAL32();
  } else if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else if (symbol->is_typed(cdk::TYPE_VOID)) {
    // EMPTY
  } else if (return_struct) {
    _pf.LOCAL(_callTempOffset);
    _evaledTupleAddr = true;
  } else {
    // cannot happen!
    ERROR("ICE(postfix_writer): unsupported function return type");
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  bool old = _needTupleAddr;
  _needTupleAddr = false; // we don't *need* an address
  node->argument()->accept(this, lvl); // determine the value
  _needTupleAddr = old;

  if (node->argument()->is_typed(cdk::TYPE_STRUCT) && _evaledTupleAddr) {
    // tuple evaluated to a base address
    // it *was* evaluated, just trash it
    _pf.TRASH(4);
  } else {
    _pf.TRASH(node->argument()->type()->size());
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
  ASSERT_SAFE_EXPRESSIONS;

  for (auto node : node->argument()->elements()) {
    auto expr = static_cast<cdk::expression_node*>(node);

    expr->accept(this, lvl);
    if (expr->is_typed(cdk::TYPE_INT) || expr->is_typed(cdk::TYPE_POINTER)) {
      _extern_functions.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (expr->is_typed(cdk::TYPE_DOUBLE)) {
      _extern_functions.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // delete the printed value
    } else if (expr->is_typed(cdk::TYPE_STRING)) {
      _extern_functions.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else {
      ERROR("ICE(postfix_writer/write_node): unkown type for write node");
    }
  }
  if (node->newline()) {
    _extern_functions.insert("println");
    _pf.CALL("println"); // print a newline
  }
}


//---------------------------------------------------------------------------

void og::postfix_writer::do_input_node(og::input_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->is_typed(cdk::TYPE_INT)) {
    _extern_functions.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _extern_functions.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    ERROR("ICE(postfix_writer): input_node does not support type")
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_for_node(og::for_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lblini, lblincr, lblend;
  _forIni.push(lblini = ++_lbl);
  _forIncr.push(lblincr = ++_lbl);
  _forEnd.push(lblend = ++_lbl);

  _symtab.push();

  if (node->initializers()) {
    node->initializers()->accept(this, lvl);
  }

  _pf.LABEL(mklbl(lblini));

  os() << "        ;; FOR condition" << std::endl;
  if (node->condition()) {
    load(node->condition(), lvl, tempOffsetForNode(node));

    if (node->condition()->is_typed(cdk::TYPE_STRUCT)) {
      // condition is at the top of the stack, move it to the start of the tuple (shortens tuple by 4 bytes)
      _pf.SP();
      _pf.INT(node->condition()->type()->size() - 4);
      _pf.ADD();
      _pf.STINT();

      // trash everything but the condition (which is now further down the stack)
      size_t to_trash = node->condition()->type()->size() - 4 - 4;
      if (to_trash > node->condition()->type()->size()) {
        std::cerr << "ICE(postfix_writer): for condition to_trash calculation underflow\n";
        exit(1);
      }

      if (to_trash) {
        _pf.TRASH(to_trash);
      }
    }

    _pf.JZ(mklbl(lblend));
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

  _symtab.pop();

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
    if (node->size() > 1) { // implies TYPE_STRUCT, when there is only one element with TYPE_STRUCT we can just pass its value up
      // store tuple in temp storage to allow indexing operations
      int tuple_base_addr = tempOffsetForNode(node);
      if (tuple_base_addr == 0 && _needTupleAddr) {
        std::cerr << "ICE(postfix_writer): tuple_node was not assigned exclusive temporary storage\n";
        exit(1);
      }

      os() << ";; tuple_node load start\n";
      int inner_tupple_base_addr_location = tuple_base_addr;
      if (_needTupleAddr) inner_tupple_base_addr_location += node->type()->size();

      for (auto el : elements) {
        auto expr = static_cast<cdk::expression_node*>(el);

        load(expr, lvl, inner_tupple_base_addr_location);
      }
      os() << ";; tuple_node load end; store start\n";

      if (_needTupleAddr) {
        store(node->type(), node->type(), [this, tuple_base_addr]() { _pf.LOCAL(tuple_base_addr); });

        // leave base address in stack
        _pf.LOCAL(tuple_base_addr);
        _evaledTupleAddr = true;
      } else {
        _evaledTupleAddr = false;
      }
    } else {
      // only 1 element
      node->element(0)->accept(this, lvl);
    }
  } else {
    for (auto it = elements.rbegin(); it != elements.rend(); it++) {
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
    if (offset) {
      _pf.INT(offset);
      _pf.ADD();
    }
    _pf.STINT();
  } else if (lvalType->name() == cdk::TYPE_DOUBLE) {
    if (rvalType->name() == cdk::TYPE_INT) {
      _pf.I2D();
    }

    baseSupplier();
    if (offset) {
      _pf.INT(offset);
      _pf.ADD();
    }
    _pf.STDOUBLE();
  } else if (lvalType->name() == cdk::TYPE_STRUCT) {
    if (rvalType->name() != cdk::TYPE_STRUCT) ERROR("typechecker is dumb");

    auto lvalStructType = cdk::structured_type_cast(lvalType);
    auto rvalStructType = cdk::structured_type_cast(rvalType);

    for (ssize_t i = lvalStructType->length() - 1; i >= 0; i--) {
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

        for (ssize_t ix = ids.size() - 1; ix >= 0; ix--) {
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

        // when parentheses are used aroud the n-tuple, it may be wrapped in multiple tuple_nodes
        while (tuple_initializer->size() != ids.size())
          tuple_initializer = dynamic_cast<og::tuple_node *>(tuple_initializer->element(0));

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

  bool old = _needTupleAddr;
  _needTupleAddr = true;
  node->base()->accept(this, lvl);
  _needTupleAddr = old;

  auto components = (cdk::structured_type_cast(node->base()->type()))->components();

  int offset = 0;
  for (ssize_t ix = components.size() - 1; ix > (ssize_t)node->index() - 1; ix--) {
    offset += components[ix]->size();
  }

  if (offset) {
    _pf.INT(offset);
    _pf.ADD();
  }

  _evaledTupleAddr = true; // may have evaled the address of a non-tuple but that's fine
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->arguments()->type()->size());
}

void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}
