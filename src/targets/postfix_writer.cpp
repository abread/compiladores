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
  _pf.INT(3); // TODO: depends on size of the type
  _pf.SHTL();
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
  if (node->base()) {
    node->base()->accept(this, lvl);
  } else {
    if (_function) { //TODO what is this??
      _pf.LOCV(-_function->type()->size());
    } else {
      std::cerr << "FATAL: " << node->lineno() << ": trying to use return value outside function" << std::endl;
    }
  }
  node->index()->accept(this, lvl);
  _pf.INT(3); //TODO: depends on size of the referenced type
  _pf.SHTL();
  _pf.ADD(); // add pointer and index
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; //TODO: tuples
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    _pf.LDINT();
  }
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

  // if (_inFunctionBody || _inFunctionArgs) { TODO: don't think this is even needed
  //   error(node->lineno(), "cannot declare function in body or in args");
  //   return;
  // }
  //DAVID: FIXME: should be at the beginning
  _functions_to_declare.insert(name); //TODO: this
}


void og::postfix_writer::do_function_definition_node(og::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto sym = _symtab.find(node->identifier());
  _function = sym;
  _symtab.push();

  auto name = fix_function_name(node->identifier());
  _functions_to_declare.erase(name); //TODO: this

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

  frame_size_calculator lsc(_compiler, _function, _symtab);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize());

  _inFunctionBody = true;

  // _offset = -_function->type()->size(); TODO: maybe?
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


  // these are just a few library function imports
  if (name == "_main")  {
    _pf.EXTERN("readi");
    _pf.EXTERN("readd");
    _pf.EXTERN("printi");
    _pf.EXTERN("printd");
    _pf.EXTERN("prints");
    _pf.EXTERN("println");
    _pf.EXTERN("argc");
    _pf.EXTERN("argv");
    _pf.EXTERN("envp");
  }

    // TODO: use extern on undefined functions
    // for (std::string& s : _functions_to_declare)
    //   _pf.EXTERN(s);
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto name = fix_function_name(node->identifier());
  std::shared_ptr<og::symbol> symbol = _symtab.find(node->identifier()); //TODO: new_symbol()?

  size_t argsSize = 0;
  auto argTypes = symbol->argsType()->components();
  if (node->arguments()) {
    for (int ax = node->arguments()->size(); ax > 0; ax--) {
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->element(ax - 1));
      // convert ints to doubles in arguments
      if (arg->is_typed(cdk::TYPE_INT) && argTypes[ax - 1]->name() == cdk::TYPE_DOUBLE) { //TODO: this could probably be cleaned up
        cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(arg);
        cdk::double_node ddi(dclini->lineno(), dclini->value());
        ddi.accept(this, lvl);
        argsSize += ddi.type()->size();
      } else {
        arg->accept(this, lvl + 2);
        argsSize += arg->type()->size();
      }
    }
  }
  _pf.CALL(name);
  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }


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
  // TODO: tuples
  if (!_function->is_typed(cdk::TYPE_VOID)) {
    node->retval()->accept(this, lvl + 2);

    if (_function->is_typed(cdk::TYPE_INT) || _function->is_typed(cdk::TYPE_STRING)
        || _function->is_typed(cdk::TYPE_POINTER)) {
      _pf.STFVAL32();
    } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      if (node->retval()->is_typed(cdk::TYPE_INT)) {
        _pf.I2D();
      }
      _pf.STFVAL64();
    } else {
      // cannot happen!
      std::cerr << "ICE: unknown return type\n";
      exit(1);
    }
  }
  _pf.LEAVE();
  _pf.RET();
}

void og::postfix_writer::do_write_node(og::write_node * const node, int lvl) {
  // TODO: handle newline flag and print EVERYTHING
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value to print
  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    _pf.CALL("printi");
    _pf.TRASH(4); // delete the printed value
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.CALL("printd");
    _pf.TRASH(8); // delete the printed value
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
    throw std::string("continue outside 'for' loop");
  }
}

void og::postfix_writer::do_break_node(og::break_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forEnd.top())); // jump to for end
  } else {
    throw std::string("break outside 'for' loop");
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
      auto tuple_initializer = dynamic_cast<og::tuple_node *>(node->initializer());
      for (size_t ix = 0; ix < ids.size(); ix++) {
        std::string& id = ids[ix];
        cdk::expression_node * init = tuple_initializer->element(ix);
        symbol = _symtab.find(id);
        _offset -= init->type()->size();
        if (symbol) {
          symbol->set_offset(_offset);
        } else {
          throw "SHOULD NOT HAPPEN";
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
    throw "SHOULD NOT HAPPEN";
  }
}

void og::postfix_writer::define_variable(std::string& id, cdk::expression_node * init, int qualifier, int lvl) {
  std::shared_ptr<symbol> symbol = _symtab.find(id);
    if (_inFunctionBody) {
      init->accept(this, lvl);
      int symbolType = symbol->type()->name();
      if (symbolType == cdk::TYPE_INT || symbolType == cdk::TYPE_STRING || symbolType == cdk::TYPE_POINTER || symbolType == cdk::TYPE_STRUCT) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (symbolType == cdk::TYPE_DOUBLE) {
        if (init->is_typed(cdk::TYPE_INT)) {
          _pf.I2D();
        }
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        throw std::string("cannot initialize");
      }
  } else {
    if (qualifier == tREQUIRE) {
      _pf.EXTERN(id);
      return;
    }

    _pf.DATA();
    _pf.ALIGN();
    if (qualifier == tPUBLIC) {
      _pf.GLOBAL(id, _pf.OBJ());
    }
    _pf.LABEL(id);
    if (init) {
      if (symbol->type()->name() == cdk::TYPE_DOUBLE) {
        if (init->is_typed(cdk::TYPE_DOUBLE)) {
          init->accept(this, lvl);
        } else if (init->is_typed(cdk::TYPE_INT)) {
          // allocate a double if the type is double
          cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(init);
          cdk::double_node ddi(dclini->lineno(), dclini->value());
          ddi.accept(this, lvl);
        } else {
          throw std::string("bad initializer for real value");
        }
      } else {
        init->accept(this, lvl);
      }
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

  if (!node->initializer()) {
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
  } else {
    if (ids.size() == 1) {
      // we have 1 id to a tuple (e.g.: auto a = 1, 2, 3)
      define_variable(ids[0], node->initializer(), node->qualifier(), lvl);
    } else {
      auto tuple_initializer = dynamic_cast<og::tuple_node *>(node->initializer()); //TODO: this cast isn't possible for functions
      for (size_t ix = 0; ix < ids.size(); ix++) {
        auto id = ids[ix];
        cdk::expression_node * init = tuple_initializer->element(ix);
        define_variable(id, init, node->qualifier(), lvl);
      }
    }
  }
}

void og::postfix_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->arguments()->type()->size());
}

void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
}
