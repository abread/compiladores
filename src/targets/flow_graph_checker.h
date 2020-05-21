#ifndef __OG_TARGET_FLOW_GRAPH_CHECKER_H__
#define __OG_TARGET_FLOW_GRAPH_CHECKER_H__

#include "targets/basic_ast_visitor.h"

namespace og {

  class flow_graph_checker: public basic_ast_visitor {
    bool _returning = false;
    bool _jumping_in_cycle = false;

    ssize_t _cycle_depth = 0;

  public:
    flow_graph_checker(std::shared_ptr<cdk::compiler> compiler) :
      basic_ast_visitor(compiler) {}

  public:
    ~flow_graph_checker() {}

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // og

#endif
