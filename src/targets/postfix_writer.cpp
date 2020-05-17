#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/frame_size_calculator.h"
#include "targets/postfix_writer.h"
#include "ast/all.h"  // all.h is automatically generated

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

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
  node->argument()->accept(this, lvl); // determine the value
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.NEG();
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DNEG();
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
  //TODO
#if 0
  node->argument()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ALLOC(); // allocate
  _pf.SP();// put base pointer in stack
#endif
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
  auto symbol = _symtab.find(id);

  if (symbol->global()) {
    _pf.ADDR(node->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void og::postfix_writer::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO
#if 0
  if (node->base()) {
    node->base()->accept(this, lvl);
  } else {
    if (_function) {
      _pf.LOCV(-_function->type()->size());
    } else {
      std::cerr << "FATAL: " << node->lineno() << ": trying to use return value outside function" << std::endl;
    }
  }
  node->index()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ADD(); // add pointer and index
#endif
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  _pf.LDINT(); // depends on type size
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

void og::postfix_writer::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto sym = _symtab.find(node->identifier());
  _function = sym;
  _symtab.push();

  auto name = fix_function_name(node->identifier());

  _offset = 8;
  // declare args, and their respective scope
  _symtab.push();
  if (node->arguments()) {
    _inFunctionArgs = true; //FIXME really needed?
    for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
        cdk::basic_node *arg = node->arguments()->node(ix);
        if (arg == nullptr) break; //TODO: needed? The grammar probably won't allow this case
        arg->accept(this, 0);
    }
    _inFunctionArgs = false; //FIXME really needed?
  }

  //TODO: adapt for all functions, watch out for qualifiers
  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC) {
    _pf.GLOBAL(name, _pf.FUNC());
  }
  _pf.LABEL(name);

  frame_size_calculator lsc(_compiler, _symtab);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  _inFunctionBody = true;

  _offset = -_function->type()->size();
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
  _function == nullptr;


  // these are just a few library function imports
  if (name == "_main")  {
    _pf.EXTERN("readi");
    _pf.EXTERN("printi");
    _pf.EXTERN("prints");
    _pf.EXTERN("println");
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto name = fix_function_name(node->identifier());

  size_t argsSize = 0;
  if (node->arguments()) {
    for (int ax = node->arguments()->size(); ax > 0; ax--) {
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->element(ax - 1));
      arg->accept(this, lvl + 2);
      argsSize += arg->type()->size();
    }
  }
  _pf.CALL(name);
  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }

  std::shared_ptr<og::symbol> symbol = _symtab.find(node->identifier());

  if (symbol->is_typed(cdk::TYPE_INT) || symbol->is_typed(cdk::TYPE_POINTER) || symbol->is_typed(cdk::TYPE_STRING)) {
    _pf.LDFVAL32();
  } else if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else if (symbol->is_typed(cdk::TYPE_STRUCT)) {
    // TODO
    throw std::string("functions that return tuples not yet supported");
  } else if (symbol->is_typed(cdk::TYPE_VOID)) {
    // EMPTY
  } else {
    // cannot happen!
    std::cerr << "ICE(postfix_writer): unsupported function return type\n";
    exit(1);
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // typechecker takes care of declaring functions
  auto name = fix_function_name(node->identifier());
  // TODO
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
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
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
  //TODO return node
#if 0
  // should not reach here without returning a value (if not void)
  if (_function->type()->name() != basic_type::TYPE_VOID) {
    node->retval()->accept(this, lvl + 2);

    if (_function->type()->name() == basic_type::TYPE_INT || _function->type()->name() == basic_type::TYPE_STRING
        || _function->type()->name() == basic_type::TYPE_POINTER) {
      _pf.STFVAL32();
    } else if (_function->type()->name() == basic_type::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == basic_type::TYPE_INT)
        _pf.I2D();
      _pf.STFVAL64();
    } else {
      std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;
    }
  }
  _pf.LEAVE();
  _pf.RET();
#endif
}

void og::postfix_writer::do_write_node(og::write_node * const node, int lvl) {
  // TODO: handle newline flag
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value to print
  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    _pf.CALL("printi");
    _pf.TRASH(4); // delete the printed value
  } else if (node->argument()->is_typed(cdk::TYPE_STRING)) {
    _pf.CALL("prints");
    _pf.TRASH(4); // delete the printed value's address
  } else {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
  if (node->newline()) {
    _pf.CALL("println"); // print a newline
  }
}


//---------------------------------------------------------------------------

void og::postfix_writer::do_input_node(og::input_node * const node, int lvl) {
  // TODO: refactor to be an expression
  ASSERT_SAFE_EXPRESSIONS;
  _pf.CALL("readi");
  _pf.LDFVAL32();
  //node->argument()->accept(this, lvl);
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

  os() << "        ;; FOR conditions" << std::endl; //TODO: there might be multiple conditions
  if (node->conditions()) {
    node->conditions()->accept(this, lvl);
  }
  _pf.JZ(mklbl(lblend));

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
    throw "continue outside 'for' loop";
  }
}

void og::postfix_writer::do_break_node(og::break_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forEnd.top())); // jump to for end
  } else {
    throw "break outside 'for' loop";
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
  for (size_t i = 0; i < elements.size(); i++) {
    elements[i]->accept(this, lvl);
  }
}

void og::postfix_writer::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // declare variable with typechecker

  auto id = node->identifiers()[0]; //TODO: tuples :(
  int offset = 0, typesize = node->type()->size();

  if (_inFunctionBody) {
    _offset -= typesize;
    offset = _offset;
  } else if (_inFunctionArgs) {
    offset = _offset;
    _offset += typesize;
  } else {
    offset = 0;
  }

  std::shared_ptr<og::symbol> symbol = new_symbol();
  if (symbol) {
    symbol->set_offset(offset);
    reset_new_symbol();
  }

  if (_inFunctionBody) {
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
    }
    if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING)
        || node->is_typed(cdk::TYPE_POINTER)) {
      _pf.LOCAL(symbol->offset());
      _pf.STINT();
    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.LOCAL(symbol->offset());
      _pf.STDOUBLE();
    } else {
      throw "cannot initialize";
    }
  } else {
    if (node->qualifier() == tREQUIRE) {
      _pf.EXTERN(id);
      return;
    }

    if (node->initializer()) {
      _pf.DATA();
    } else {
      _pf.BSS();
    }

    _pf.ALIGN();
    if (node->qualifier() == tPUBLIC) {
      _pf.GLOBAL(id, _pf.OBJ());
    }
    _pf.LABEL(id);
    if (node->initializer()) {
      if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
          node->initializer()->accept(this, lvl);
        } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
          // allocate a double if the type is double
          cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(node->initializer());
          cdk::double_node ddi(dclini->lineno(), dclini->value());
          ddi.accept(this, lvl);
        } else {
          throw "bad initializer for real value";
        }
      } else {
        node->initializer()->accept(this, lvl); //TODO: compare with gr8 because of string alloc
      }
    } else {
      _pf.SALLOC(node->type()->size());
    }
  }
}

void og::postfix_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO
}

void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
}
