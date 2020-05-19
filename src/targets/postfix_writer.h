#ifndef __OG_TARGETS_POSTFIX_WRITER_H__
#define __OG_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <stack>
#include <set>
#include <cdk/emitters/basic_postfix_emitter.h>

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
    std::set<std::string> _functions_to_declare;
    bool _inFunctionBody = false;
    bool _inFunctionArgs = false;
    int _offset = 0;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
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

    void processIDBinaryExpression(cdk::binary_operation_node *const node, int lvl);
    void processIDComparison(cdk::binary_operation_node *const node, int lvl);
    void set_declaration_offsets(og::variable_declaration_node * const node);
    void store_complex_ret(std::shared_ptr<cdk::basic_type> lvalType, std::shared_ptr<cdk::basic_type> rvalType, int offset);
    void define_variable(std::string& id, cdk::expression_node * init, int qualifier, int lvl);
    void store_local(std::shared_ptr<cdk::basic_type> lvalType, std::shared_ptr<cdk::basic_type> rvalType, int offset);

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // og

#endif
