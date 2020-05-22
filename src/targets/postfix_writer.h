#ifndef __OG_TARGETS_POSTFIX_WRITER_H__
#define __OG_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <functional>
#include <sstream>
#include <stack>
#include <set>
#include <cdk/emitters/basic_postfix_emitter.h>
#include <cdk/types/types.h>

#ifndef tREQUIRE
#include "og_parser.tab.h"
#endif

namespace og {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<og::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    std::stack<int> _forIni, _forIncr, _forEnd;

    std::shared_ptr<og::symbol> _function = nullptr;
    std::set<std::string> _extern_functions;
    bool _inFunctionBody = false;
    bool _inFunctionArgs = false;
    int _offset = 0;
    std::map<const cdk::basic_node*, int> _unsharedTempOffsetTab;
    int _callTempOffset;
    int _returnTempOffset;

    bool _needTupleAddr = false; // true = needs tuple to be stored somewhere and evaluate to its base address
    bool _evaledTupleAddr = false; // true = tuple was evaluated to base address
    // if the parent activates _needTupleAddr, _evaledTupleAddr must be set to true by child (child must eval address)

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
      // ensure builtin functions are available
      auto int_t = cdk::make_primitive_type(4, cdk::TYPE_INT);
      auto str_t = cdk::make_primitive_type(4, cdk::TYPE_STRING);
      auto tuple_empty_t = cdk::make_structured_type(std::vector<std::shared_ptr<cdk::basic_type>>());
      auto tuple_int_t = cdk::make_structured_type(std::vector<std::shared_ptr<cdk::basic_type>>(1, int_t));

      auto argc = std::make_shared<og::symbol>(tPUBLIC, int_t, "argc", tuple_empty_t);
      auto argv = std::make_shared<og::symbol>(tPUBLIC, str_t, "argv", tuple_int_t);
      auto envp = std::make_shared<og::symbol>(tPUBLIC, str_t, "envp", tuple_int_t);

      for (auto sym : {argc, argv, envp}) {
        _symtab.insert(sym->name(), sym);
        _extern_functions.insert(sym->name());
      }
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

    const std::set<std::string> &extern_functions() const {
      return _extern_functions;
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    inline int tempOffsetForNode(const cdk::basic_node* node) const {
      auto it = _unsharedTempOffsetTab.find(node);
      if (it == _unsharedTempOffsetTab.end()) {
        return 0;
      }

      return it->second;
    }

    void processIDBinaryExpression(cdk::binary_operation_node *const node, int lvl);
    void processIDComparison(cdk::binary_operation_node *const node, int lvl);
    void load_from_base(std::shared_ptr<cdk::basic_type> type, std::function<void()> baseSupplier, int offset = 0);
    void load(cdk::typed_node *const node, int lvl, int tempOffset = 0);
    void set_declaration_offsets(og::variable_declaration_node * const node);
    void define_global_variable(const std::string& id, cdk::expression_node * init, int qualifier, int lvl);
    void store(std::shared_ptr<cdk::basic_type> lvalType, std::shared_ptr<cdk::basic_type> rvalType, std::function<void()> baseSupplier, int offset = 0);

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // og

#endif
