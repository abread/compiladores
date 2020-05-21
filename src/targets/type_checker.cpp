#include <string>
#include <type_traits>
#include "targets/type_checker.h"
#include "targets/flow_graph_checker.h"
#include "ast/all.h" // automatically generated
#include <cdk/types/primitive_type.h>

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

// throw without loosing lineno information
#define THROW_ERROR(MSG) throw std::make_tuple<const cdk::basic_node*, std::string>(node, std::string(MSG))

static bool is_ID(cdk::typed_node *const node) {
  return node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE);
}

static bool is_PID(cdk::typed_node *const node) {
  return is_ID(node) || node->is_typed(cdk::TYPE_POINTER);
}

// Different operations have different notions of allowed types
class TypeCompatOptions {
public:
  TypeCompatOptions(bool id = true, bool di = true, bool unspec = false, bool ptrassign = true, bool generalizeptr = false)
    : acceptID(id), acceptDI(di), acceptUnspec(unspec), ptrAssignment(ptrassign) {}
  bool acceptID; // accept (TYPE_INT, TYPE_DOUBLE), return TYPE_DOUBLE
  bool acceptDI; // accept (TYPE_DOUBLE, TYPE_INT), return TYPE_DOUBLE
  bool acceptUnspec; // accept TYPE_UNSPEC in one or more arguments, return the one that isn't TYPE_UNSPEC if it exists
  bool ptrAssignment; // (ptr<auto>, ptr<X>) returns ptr<auto> and (ptr<X>, ptr<auto>) returns ptr<X> (allow conversion to/from void ptr)
  bool generalizePtr; // (ptr<X>, ptr<Y>) returns ptr<auto>
};

const TypeCompatOptions DEFAULT_TYPE_COMPAT = TypeCompatOptions();
const TypeCompatOptions GENERALIZE_TYPE_COMPAT = TypeCompatOptions(true, true, true, true, true);
const TypeCompatOptions ASSIGNMENT_TYPE_COMPAT = TypeCompatOptions(false, true, false, true, false);
const TypeCompatOptions INITIALIZER_TYPE_COMPAT = TypeCompatOptions(false, true, true, true, false);
const TypeCompatOptions DECL_TYPE_COMPAT = TypeCompatOptions(false, false, true, false, false);

static std::shared_ptr<cdk::basic_type> compatible_types(std::shared_ptr<cdk::basic_type> a, std::shared_ptr<cdk::basic_type> b, TypeCompatOptions opts);

static std::shared_ptr<cdk::basic_type> compatible_types_struct(std::shared_ptr<cdk::structured_type> a, std::shared_ptr<cdk::structured_type> b, TypeCompatOptions opts) {
  if (a->length() != b->length()) {
    return nullptr;
  }

  std::vector<std::shared_ptr<cdk::basic_type>> components(a->length());
  for (size_t i = 0; i < a->length(); i++) {
    auto comp = compatible_types(a->component(i), b->component(i), opts);

    if (comp == nullptr) {
      return nullptr;
    }

    components[i] = comp;
  }

  if (components.size() == 1) {
    // tuple with 1 element is the same as that element
    return components[0];
  }

  return cdk::make_structured_type(components);
}

static std::shared_ptr<cdk::basic_type> is_void_ptr(std::shared_ptr<cdk::reference_type> t) {
  if (t->referenced()->name() == cdk::TYPE_POINTER) {
    return is_void_ptr(cdk::reference_type_cast(t->referenced()));
  } else if (t->referenced()->name() == cdk::TYPE_VOID) {
    return t;
  } else {
    return nullptr;
  }
}

static std::shared_ptr<cdk::basic_type> compatible_types_ptr(std::shared_ptr<cdk::reference_type> a, std::shared_ptr<cdk::reference_type> b, TypeCompatOptions opts) {
  if (auto aa = is_void_ptr(a); aa && (opts.ptrAssignment || opts.generalizePtr)) {
    return aa;
  } else if (auto bb = is_void_ptr(b); bb && (opts.ptrAssignment || opts.generalizePtr)) {
    if (opts.ptrAssignment) {
      return a;
    } else if (opts.generalizePtr) {
      return bb;
    } else {
      std::cerr << "ICE(type_checker/compatible_types_ptr): my creator was careless\n";
      exit(1);
    }
  } else {
    // ptr<int> != ptr<double> always
    opts.acceptID = false;
    opts.acceptDI = false;

    auto referenced = compatible_types(a->referenced(), b->referenced(), opts);
    if (referenced == nullptr) {
      if (opts.generalizePtr) {
        referenced = cdk::make_primitive_type(1, cdk::TYPE_VOID);
      } else {
        return nullptr;
      }
    }

    return cdk::make_reference_type(4, referenced);
  }
}

static std::shared_ptr<cdk::basic_type> compatible_types(std::shared_ptr<cdk::basic_type> a, std::shared_ptr<cdk::basic_type> b, TypeCompatOptions opts = DEFAULT_TYPE_COMPAT) {
  if (opts.acceptUnspec && a->name() == cdk::TYPE_UNSPEC) {
    return b;
  } else if (opts.acceptUnspec && b->name() == cdk::TYPE_UNSPEC) {
    return a;
  } else if (opts.acceptDI && a->name() == cdk::TYPE_DOUBLE && b->name() == cdk::TYPE_INT) {
    return a;
  } else if (opts.acceptID && a->name() == cdk::TYPE_INT && b->name() == cdk::TYPE_DOUBLE) {
    return b;
  } else if (a->name() == cdk::TYPE_STRUCT && cdk::structured_type_cast(a)->length() == 1) {
    auto aa = cdk::structured_type_cast(a);
    return compatible_types(aa->component(0), b);
  } else if (b->name() == cdk::TYPE_STRUCT && cdk::structured_type_cast(b)->length() == 1) {
    auto bb = cdk::structured_type_cast(b);
    return compatible_types(a, bb->component(0));
  } else if (a->name() != b->name()) { // structs with different sizes may be alright, pointers are all 4 bytes
    return nullptr;
  }

  if (a->name() == cdk::TYPE_STRUCT) {
    return compatible_types_struct(cdk::structured_type_cast(a), cdk::structured_type_cast(b), opts);
  } else if (a->name() == cdk::TYPE_POINTER) {
    return compatible_types_ptr(cdk::reference_type_cast(a), cdk::reference_type_cast(b), opts);
  } else {
    return a;
  }
}

static std::shared_ptr<cdk::basic_type> compatible_types(cdk::typed_node *const aNode, cdk::typed_node *const bNode, TypeCompatOptions opts = DEFAULT_TYPE_COMPAT) {
  return compatible_types(aNode->type(), bNode->type(), opts);
}


//---------------------------------------------------------------------------

void og::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_continue_node(og::continue_node * const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_break_node(og::break_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void og::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (auto el : node->nodes()) {
    el->accept(this, lvl + 2);
  }
}

//---------------------------------------------------------------------------

void og::type_checker::do_return_node(og::return_node *const node, int lvl) {
  if (_function == nullptr) {
    THROW_ERROR("return outside function body");
  }

  if (node->retval()) {
    if (_function->is_typed(cdk::TYPE_VOID)) {
      THROW_ERROR("non-empty return in void function.");
    }

    node->retval()->accept(this, lvl + 2);

    TypeCompatOptions opts = ASSIGNMENT_TYPE_COMPAT;
    if (_function->autoType()) {
      // infering return type, relax rules (allow ints to become doubles in the ret type, generalize pointers, etc.)
      opts = GENERALIZE_TYPE_COMPAT;
    }

    auto type = compatible_types(_function->type(), node->retval()->type(), opts);

    if (type == nullptr) {
      THROW_ERROR("return expression incompatible with function return type");
    }

    if (_function->autoType()) {
      // we've inferred a better return type (or stayed the same)
      _function->type(type);
    }
  } else if (! _function->is_typed(cdk::TYPE_VOID)) {
    THROW_ERROR("empty return in non-void function");
  }
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
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(1, cdk::TYPE_VOID)));
}

//---------------------------------------------------------------------------

void og::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(node->argument()->type());
}

void og::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);

  if (!is_ID(node)) {
    THROW_ERROR("invalid type");
  }
}

void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);

  if (!node->is_typed(cdk::TYPE_INT)) {
    THROW_ERROR("invalid type");
  }
}

void og::type_checker::do_identity_node(og::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);

  if (!is_ID(node)) {
    THROW_ERROR("invalid type");
  }
}

void og::type_checker::processComparisonExpression(cdk::binary_operation_node *const node, int lvl, bool allowPointers) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (compatible_types(node->left(), node->right()) &&
      ( (allowPointers && is_PID(node->left())) || is_ID(node->left()) )) {
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  } else {
    THROW_ERROR("invalid types in comparison expr");
  }
}

void og::type_checker::processArithmeticExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (auto type = compatible_types(node->left(), node->right()); type != nullptr && is_ID(node->left()) && is_ID(node->right())) {
    node->type(type);
  } else {
    THROW_ERROR("invalid type for int/double binary expr");
  }
}

void og::type_checker::processLogicExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else {
    THROW_ERROR("invalid type for int binary expr");
  }
}

void og::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
    return;
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    node->type(node->right()->type());
    return;
  }

  auto type = compatible_types(node->left(), node->right());
  if (!type || !is_ID(node->left())) {
    THROW_ERROR("invalid types in binary expr");
  }

  node->type(type);
}
void og::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
    return;
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    node->type(node->right()->type());
    return;
  }

  auto type = compatible_types(node->left(), node->right());
  if (!type || !is_PID(node->left())) {
    THROW_ERROR("invalid types in binary expr");
  }

  if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    // pointer subtraction
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  } else {
    node->type(type);
  }
}
void og::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processArithmeticExpression(node, lvl);
}
void og::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processArithmeticExpression(node, lvl);
}
void og::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else {
    THROW_ERROR("invalid type for int binary expr");
  }
}
void og::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processComparisonExpression(node, lvl, false);
}
void og::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processComparisonExpression(node, lvl, false);
}
void og::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processComparisonExpression(node, lvl, false);
}
void og::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processComparisonExpression(node, lvl, false);
}
void og::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processComparisonExpression(node, lvl, true);
}
void og::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processComparisonExpression(node, lvl, true);
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processLogicExpression(node, lvl);
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processLogicExpression(node, lvl);
}

//---------------------------------------------------------------------------

void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol) {
    node->type(symbol->type());
  } else {
    THROW_ERROR("undefined variable: " + id);
  }
}

void og::type_checker::do_pointer_index_node(og::pointer_index_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);

  if (! node->base()->is_typed(cdk::TYPE_POINTER)) {
    THROW_ERROR("pointer expression expected in pointer indexing");
  }

  if (! node->index()->is_typed(cdk::TYPE_INT)) {
    THROW_ERROR("integer expected in index");
  }

  auto ref = cdk::reference_type_cast(node->base()->type());
  node->type(ref->referenced());
}

void og::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void og::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);

  if (node->lvalue()->is_typed(cdk::TYPE_STRUCT) || node->rvalue()->is_typed(cdk::TYPE_STRUCT)) {
    THROW_ERROR("tuple assignment not supported");
  }

  if (compatible_types(node->lvalue(), node->rvalue(), ASSIGNMENT_TYPE_COMPAT) == nullptr) {
    THROW_ERROR("incompatible types in assignment");
  }

  node->type(node->lvalue()->type());

  if (auto input = dynamic_cast<og::input_node*>(node->rvalue()); input && node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    // HACK: a = input must work with doubles
    // if this isn't here, input will return an int which will be converted to a double (when a is a double)
    input->type(node->type());
  } else if (auto alloc = dynamic_cast<og::stack_alloc_node *>(node->rvalue())) {
    // HACK: alloc gets its type from the left value
    alloc->type(node->lvalue()->type());
  }
}

//---------------------------------------------------------------------------

template <typename T>
std::shared_ptr<og::symbol> og::type_checker::declare_function(T *const node, int lvl) {
  static_assert(std::is_same<T, og::function_declaration_node>::value || std::is_same<T, og::function_definition_node>::value, "trying to declare functions from a weird kind of node");

  int qualifier = node->qualifier();
  auto ret_type = node->type();
  auto id = node->identifier();
  std::shared_ptr<cdk::structured_type> args_type;

  // compute arguments type
  std::vector<std::shared_ptr<cdk::basic_type>> each_arg_type;
  if (node->arguments()) {
    // arguments need no visit, they already have a type (auto args are forbidden)

    for (auto n : node->arguments()->nodes()) {
      auto decl = static_cast<og::variable_declaration_node*>(n);
      each_arg_type.push_back(decl->type());
    }
  }

  args_type = cdk::make_structured_type(each_arg_type);

  auto sym = std::make_shared<og::symbol>(qualifier, ret_type, id, args_type);

  auto old_sym = _symtab.find_local(id);
  if (old_sym) {
    if (!compatible_types(old_sym->type(), sym->type(), DECL_TYPE_COMPAT)
        || !compatible_types(old_sym->argsType(), sym->argsType(), DECL_TYPE_COMPAT)
        || old_sym->qualifier() != sym->qualifier()
        || old_sym->autoType() != sym->autoType()) {
      THROW_ERROR("conflicting declarations for " + id);
    }

    return old_sym;
  } else {
    _symtab.insert(id, sym);
    if (!_typecheckingFunction) _parent->set_new_symbol(sym);
    return sym;
  }
}

void og::type_checker::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  declare_function(node, lvl);
}

void og::type_checker::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  if (node->qualifier() == tREQUIRE)
    THROW_ERROR("can't require a function definition");

  auto sym = declare_function(node, lvl);

  // Mark function as defined (throw if redefinition)
  if (sym && sym->definedOrInitialized()) {
    THROW_ERROR("function redefinition: " + sym->name());
  }
  sym->definedOrInitialized() = true;

  // ensure return type is known
  _function = sym;
  _typecheckingFunction = true;
  _symtab.push();

  if (node->arguments()) {
    node->arguments()->accept(this, lvl);
  }

  node->block()->accept(this, lvl+2);

  _symtab.pop();
  _typecheckingFunction = false;
  _function = nullptr;

  if (sym->is_typed(cdk::TYPE_UNSPEC)) {
    THROW_ERROR("auto function didn't return anything");
  }

  // Check flow graph to ensure function returns correctly (also checks breaks/continues)
  flow_graph_checker fgc(_compiler);
  node->accept(&fgc, lvl); // will throw if it fails

  node->type(sym->type());
}

void og::type_checker::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;

  auto id = node->identifier();

  auto sym = _symtab.find(id);
  if (!sym) {
    THROW_ERROR("tried to use undeclared function: " + id);
  }

  std::shared_ptr<cdk::basic_type> argsType;
  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 2);
    argsType = node->arguments()->type();

    // make sure argsType is a structured type
    if (argsType->name() != cdk::TYPE_STRUCT) {
      argsType = cdk::make_structured_type(std::vector<std::shared_ptr<cdk::basic_type>>(1, argsType));
    }
  } else {
    std::vector<std::shared_ptr<cdk::basic_type>> empty;
    argsType = cdk::make_structured_type(empty);
  }

  if (compatible_types(sym->argsType(), argsType, ASSIGNMENT_TYPE_COMPAT) == nullptr) {
    THROW_ERROR("incorrect argument type in call to " + id);
  }

  if (sym->is_typed(cdk::TYPE_UNSPEC)) {
    THROW_ERROR("called function does not have a known return type: " + id);
  }

  node->type(sym->type());
}

void og::type_checker::do_evaluation_node(og::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void og::type_checker::do_block_node(og::block_node * const node, int lvl) {
  // HACK: run typechecker for function definition ahead of code generation
  if (_typecheckingFunction) {
    _symtab.push();

    if (node->declarations()) {
      node->declarations()->accept(this, lvl + 2);
    }

    if (node->instructions()) {
      node->instructions()->accept(this, lvl + 2);
    }

    _symtab.pop();
  }
}

void og::type_checker::do_write_node(og::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_STRUCT)) {
    auto type = cdk::structured_type_cast(node->argument()->type());

    for (auto comp : type->components()) {
      if (comp->name() != cdk::TYPE_INT && comp->name() != cdk::TYPE_DOUBLE && comp->name() != cdk::TYPE_STRING) {
        THROW_ERROR("invalid expression type in write statement");
      }
    }

    if (node->argument()->size() != type->length()) {
      THROW_ERROR("invalid expression type in write statement");
    }
  } else if (!node->argument()->is_typed(cdk::TYPE_INT) && !node->argument()->is_typed(cdk::TYPE_DOUBLE) && !node->argument()->is_typed(cdk::TYPE_STRING)) {
    THROW_ERROR("invalid expression type in write statement");
  }
}

//---------------------------------------------------------------------------

void og::type_checker::do_input_node(og::input_node *const node, int lvl) {
  ASSERT_UNSPEC;
  // HACK: assignment node may override type to be a double
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void og::type_checker::do_address_of_node(og::address_of_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::make_reference_type(4, node->lvalue()->type()));
}

void og::type_checker::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(1, cdk::TYPE_VOID)));
}

//---------------------------------------------------------------------------

void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  // HACK: typechecking condition requires defining symbols: don't leak them
  _symtab.push();

  if (node->initializers()) {
    node->initializers()->accept(this, lvl + 2);
  }

  if (node->condition()) {
    node->condition()->accept(this, lvl + 2);

    if (node->condition()->is_typed(cdk::TYPE_STRUCT)) {
      auto type = cdk::structured_type_cast(node->condition()->type());

      if (type->component(type->length() - 1)->name() != cdk::TYPE_INT) {
        THROW_ERROR("for condition must end with int/bool expression");
      }
    } else if (!node->condition()->is_typed(cdk::TYPE_INT)) {
      THROW_ERROR("for condition must end with int/bool expression");
    }

  }

  if (node->increments()) {
    node->increments()->accept(this, lvl + 2);
  }

  node->block()->accept(this, lvl + 2);

  // Check flow graph to ensure breaks and continues are placed correctly
  //flow_graph_checker fgc(_compiler);
  //node->accept(&fgc, lvl); // will throw if it fails
  // already checked in function_definition_node, and all loops must be inside a function definition

  _symtab.pop();
}


//---------------------------------------------------------------------------

void og::type_checker::do_if_node(og::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);

  if (! node->condition()->is_typed(cdk::TYPE_INT))
    THROW_ERROR("invalid type for condition" + cdk::to_string(node->condition()->type()));
}

void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);

  if (! node->condition()->is_typed(cdk::TYPE_INT))
    THROW_ERROR("invalid type for condition" + cdk::to_string(node->condition()->type()));
}

void og::type_checker::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->seq()->accept(this, lvl + 2);

  std::vector<std::shared_ptr<cdk::basic_type>> el_types;
  for (size_t i = 0; i < node->elements().size(); i++) {
    cdk::expression_node* el = node->element(i);
    el_types.push_back(el->type());
  }

  if (el_types.size() == 1) {
    // tuple with 1 element is the same as that element
    node->type(el_types[0]);
  } else {
    node->type(cdk::make_structured_type(el_types));
  }
}

std::shared_ptr<og::symbol> og::type_checker::declare_var(int qualifier, std::shared_ptr<cdk::basic_type> typeHint, const std::string &id, std::shared_ptr<cdk::basic_type> initializerType = nullptr) {
  auto type = typeHint;
  if (initializerType != nullptr) {
    type = compatible_types(typeHint, initializerType, INITIALIZER_TYPE_COMPAT);
    if (type == nullptr) {
      throw std::string("mismatch between declared type and initializer type in variable: " + id);
    }
  }

  auto sym = std::make_shared<og::symbol>(qualifier, type, id);
  if (initializerType != nullptr) {
    sym->definedOrInitialized() = true;
  }

  if (_symtab.insert(id, sym)) {
    if (!_typecheckingFunction) _parent->set_new_symbol(sym);
  } else {
    throw std::string("variable redeclaration: " + id);
  }

  return sym;
}

static bool is_literal(cdk::expression_node *expr) {
  if (auto tuple = dynamic_cast<og::tuple_node*>(expr)) {
    for (auto el : tuple->elements()) {
      if (!is_literal(static_cast<cdk::expression_node*>(el))) {
        return false;
      }
    }
    return true;
  }

  return dynamic_cast<cdk::integer_node*>(expr) || \
          dynamic_cast<cdk::string_node*>(expr) || \
          dynamic_cast<cdk::double_node*>(expr) || \
          dynamic_cast<og::nullptr_node*>(expr);
}

void og::type_checker::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  if (node->initializer() && node->qualifier() == tREQUIRE)
    THROW_ERROR("external(required) variables cannot be initialized");

  if (node->is_typed(cdk::TYPE_UNSPEC) && node->qualifier() == tREQUIRE)
    THROW_ERROR("external(required) variables must have a concrete type");

  if (node->initializer()) {
    node->initializer()->accept(this, lvl);

    if (node->is_typed(cdk::TYPE_UNSPEC) &&
        !(node->identifiers().size() == 1 ||
          (node->initializer()->is_typed(cdk::TYPE_STRUCT) && cdk::structured_type_cast(node->initializer()->type())->length() == node->identifiers().size())
          )
        ) {
        THROW_ERROR("number of identifiers does not match number of expressions");
      }

    if (!_function && !is_literal(node->initializer())) {
      THROW_ERROR("global variable declarations may only be initialized with literals");
    }
  }

  int qualifier = node->qualifier();
  auto typeHint = node->type();
  std::shared_ptr<cdk::basic_type> initType = nullptr;

  if (node->identifiers().size() == 1) {
    auto id = node->identifiers()[0];
    if (node->initializer()) {
      initType = node->initializer()->type();
    }

    auto sym = declare_var(qualifier, typeHint, id, initType);
    node->type(sym->type());

    // HACK: alloc gets its type from the left value
    if (auto alloc = dynamic_cast<og::stack_alloc_node*>(node->initializer())) {
      alloc->type(node->type());
    }

    // HACK: real a = input must work with doubles
    // if this isn't here, input will return an int which will be converted to a double (when a is a double)
    if (auto input = dynamic_cast<og::input_node*>(node->initializer()); input && node->is_typed(cdk::TYPE_DOUBLE)) {
      input->type(node->type());
    }
  } else {
    std::vector<std::shared_ptr<cdk::basic_type>> compTypes(node->identifiers().size());

    for (size_t i = 0; i < node->identifiers().size(); i++) {
      auto id = node->identifiers()[i];

      if (node->is_typed(cdk::TYPE_STRUCT)) {
        // fix typeHint
        typeHint = cdk::structured_type_cast(node->type())->component(i);
      }

      std::shared_ptr<cdk::basic_type> compInitType = nullptr;
      if (node->initializer()) {
        auto initializerType = cdk::structured_type_cast(node->initializer()->type());
        compInitType = initializerType->component(i);
      }

      auto sym = declare_var(qualifier, typeHint, id, compInitType);
      compTypes[i] = sym->type();
    }

    node->type(cdk::make_structured_type(compTypes));
  }
}

void og::type_checker::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);

  if (! node->base()->is_typed(cdk::TYPE_STRUCT)) {
    THROW_ERROR("can't index 1-tuples");
  }

  auto baseType = cdk::structured_type_cast(node->base()->type());

  if (node->index() < 1) {
    THROW_ERROR("invalid tuple index. remember that it starts with 1");
  }

  if (node->index() > baseType->length()) {
    THROW_ERROR("tuple index too big");
  }

  size_t idx = node->index() - 1;
  node->type(baseType->component(idx));
}

void og::type_checker::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->arguments()->accept(this, lvl + 2);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
