#include <string>
#include <type_traits>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

static bool is_ID(cdk::typed_node *const node) {
  return node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE);
}

static bool is_PID(cdk::typed_node *const node) {
  return is_ID(node) || node->is_typed(cdk::TYPE_POINTER);
}

// Different operations have different notions of allowed types
class TypeCompatOptions {
public:
  TypeCompatOptions(bool id = true, bool di = true, bool unspec = false, bool ptrassign = false)
    : acceptID(id), acceptDI(di), acceptUnspec(unspec), ptrAssignment(ptrassign) {}
  bool acceptID; // accept (TYPE_INT, TYPE_DOUBLE), return TYPE_DOUBLE
  bool acceptDI; // accept (TYPE_DOUBLE, TYPE_INT), return TYPE_DOUBLE
  bool acceptUnspec; // accept TYPE_UNSPEC in one or more arguments, return the one that isn't TYPE_UNSPEC if it exists
  bool ptrAssignment; // (ptr<auto>, ptr<anything>) returns ptr<auto>
};

const TypeCompatOptions DEFAULT_TYPE_COMPAT = TypeCompatOptions();
const TypeCompatOptions ASSIGNMENT_TYPE_COMPAT = TypeCompatOptions(false, true, false, true);
const TypeCompatOptions INITIALIZER_TYPE_COMPAT = TypeCompatOptions(false, true, true, true);

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

    components.push_back(comp);
  }

  if (components.size() == 1) {
    // tuple with 1 element is the same as that element
    return components[0];
  }

  return cdk::make_structured_type(components);
}

static std::shared_ptr<cdk::basic_type> compatible_types_ptr(std::shared_ptr<cdk::reference_type> a, std::shared_ptr<cdk::reference_type> b, TypeCompatOptions opts) {
  if (b->referenced()->name() == cdk::TYPE_UNSPEC) {
    return a;
  } else if (a->referenced()->name() == cdk::TYPE_UNSPEC) {
    if (opts.ptrAssignment) {
      return a;
    } else {
      return b;
    }
  } else {
    // ptr<int> != ptr<double> always
    opts.acceptID = false;
    opts.acceptDI = false;

    auto referenced = compatible_types(a->referenced(), b->referenced(), opts);
    if (referenced == nullptr) {
      return nullptr;
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
  } else if (a->name() != b->name()) { // structs with different sizes may be alright, pointers are all 4 bytes
    return nullptr;
  }

  if (a->name() == cdk::TYPE_STRUCT) {
    return compatible_types_struct(cdk::structured_type_cast(a), cdk::structured_type_cast(a), opts);
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
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(0, cdk::TYPE_UNSPEC)));
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
    throw std::string("invalid type");
  }
}

void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);

  if (!node->is_typed(cdk::TYPE_INT)) {
    throw std::string("invalid type");
  }
}

void og::type_checker::do_identity_node(og::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);

  if (!is_ID(node)) {
    throw std::string("invalid type");
  }
}

void og::type_checker::processPIDBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (auto type = compatible_types(node->left(), node->right()); type != nullptr && is_PID(node->left()) && is_PID(node->right())) {
    node->type(type);
  } else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    node->type(node->right()->type());
  } else {
    throw std::string("invalid types in binary expr");
  }
}

void og::type_checker::processIDBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (auto type = compatible_types(node->left(), node->right()); type != nullptr && is_ID(node->left()) && is_ID(node->right())) {
    node->type(type);
  } else {
    throw std::string("invalid type for int/double binary expr");
  }
}

void og::type_checker::processIBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (!(node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))) {
    throw std::string("invalid type for int binary expr");
  }

  node->type(node->left()->type());
}

void og::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processPIDBinaryExpression(node, lvl);
}
void og::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processPIDBinaryExpression(node, lvl);
}
void og::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
}
void og::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
}
void og::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processIBinaryExpression(node, lvl);
}
void og::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processPIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processPIDBinaryExpression(node, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processIDBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol) {
    node->type(symbol->type());
  } else {
    throw std::string("undefined variable: " + id);
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

  if (compatible_types(node->lvalue(), node->rvalue(), ASSIGNMENT_TYPE_COMPAT) == nullptr) {
    throw std::string("incompatible types in assignment");
  }

  node->type(node->lvalue()->type());
}

//---------------------------------------------------------------------------

template <typename T>
std::shared_ptr<og::symbol> og::type_checker::declare_function(T *const node, int lvl) {
  static_assert(std::is_same<T, og::function_declaration_node>::value || std::is_same<T, og::function_definition_node>::value, "trying to declare functions from a weird kind of node");

  int qualifier = node->qualifier();
  auto ret_type = node->type();
  auto id = node->identifier();
  std::shared_ptr<cdk::basic_type> args_type;

  // "fix" naming issues
  if (id == "og") {
    id = "_main";
  } else if (id == "_main") {
    id = "._main";
  }

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
    if (false /* TODO: check conflicts properly */) {
      throw std::string("conflicting declarations for " + id);
    }

    return old_sym;
  } else {
    _symtab.insert(id, sym);
    _parent->set_new_symbol(sym);
    return sym;
  }
}

void og::type_checker::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  if (node->qualifier() == tREQUIRE)
    throw std::string("can't require a function definition");

  auto sym = declare_function(node, lvl);

  // Mark function as defined (throw if redefinition)
  if (sym && sym->definedOrInitialized()) {
    throw std::string("function redefinition: " + sym->name());
  }
  sym->definedOrInitialized() = true;
}

void og::type_checker::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;

  auto id = node->identifier();

  // "fix" naming issues
  if (id == "og") {
    id = "_main";
  } else if (id == "_main") {
    id = "._main";
  }

  auto sym = _symtab.find(id);
  if (!sym) {
    throw std::string("tried to use undeclared function: " + id);
  }

  if (node->arguments()) {
    node->arguments()->accept(this, lvl + 2);
  }

  // TODO check arguments

  if (sym->is_typed(cdk::TYPE_UNSPEC)) {
    throw std::string("called function does not have a known return type: " + id);
  }

  node->type(sym->type());
}

void og::type_checker::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  declare_function(node, lvl);
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
}

void og::type_checker::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  if (node->initializers()) {
    node->initializers()->accept(this, lvl + 2);
  }

  if (node->conditions()) {
    node->conditions()->accept(this, lvl + 2);
    auto t = node->conditions()->type();

    if (t->name() == cdk::TYPE_STRUCT) {
      for (auto comp : cdk::structured_type_cast(t)->components())
        if (comp->name() != cdk::TYPE_INT)
          throw std::string("invalid type for for-loop conditional expression: " + cdk::to_string(comp));
    } else if (t->name() != cdk::TYPE_INT) {
      throw std::string("invalid type for for-loop conditions: " + cdk::to_string(t));
    }
  }

  if (node->increments()) {
    node->increments()->accept(this, lvl + 2);
  }

  node->block()->accept(this, lvl + 2);
}


//---------------------------------------------------------------------------

void og::type_checker::do_if_node(og::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);

  if (! node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("invalid type for condition" + cdk::to_string(node->condition()->type()));
}

void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);

  if (! node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("invalid type for condition" + cdk::to_string(node->condition()->type()));
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

void og::type_checker::declare_var(int qualifier, std::shared_ptr<cdk::basic_type> typeHint, const std::string &id, std::shared_ptr<cdk::basic_type> initializerType = nullptr) {
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
    _parent->set_new_symbol(sym);
  } else {
    throw std::string("variable redeclaration: " + id);
  }
}

void og::type_checker::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  if (node->initializer() && node->qualifier() == tREQUIRE)
    throw std::string("external(required) variables cannot be initialized");

  if (node->is_typed(cdk::TYPE_UNSPEC) && node->qualifier() == tREQUIRE)
    throw std::string("external(required) variables must have a concrete type");

  if (node->initializer()) {
    node->initializer()->accept(this, lvl);

    if (node->is_typed(cdk::TYPE_UNSPEC) &&
        !(node->identifiers().size() == 1 ||
          (node->initializer()->is_typed(cdk::TYPE_STRUCT) && cdk::structured_type_cast(node->initializer()->type())->length() == node->identifiers().size())
          )
        ) {
        throw std::string("number of identifiers does not match number of expressions");
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

    declare_var(qualifier, typeHint, id, initType);
  } else {
    for (size_t i = 0; i < node->identifiers().size(); i++) {
      auto id = node->identifiers()[i];

      std::shared_ptr<cdk::basic_type> compInitType = nullptr;
      if (node->initializer()) {
        auto initializerType = cdk::structured_type_cast(node->initializer()->type());
        compInitType = initializerType->component(i);
      }

      declare_var(qualifier, typeHint, id, compInitType);
    }
  }
}

void og::type_checker::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);

  if (! node->base()->is_typed(cdk::TYPE_STRUCT)) {
    throw std::string("tuple index base is not a tuple");
  }

  auto baseType = cdk::structured_type_cast(node->base()->type());

  if (node->index() < 1) {
    throw std::string("invalid tuple index. remember that it starts with 1");
  }

  if (node->index() > baseType->length()) {
    throw std::string("tuple index too big");
  }

  size_t idx = node->index() - 1;
  node->type(baseType->component(idx));
}

void og::type_checker::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->arguments()->accept(this, lvl + 2);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}
