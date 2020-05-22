#ifndef __OG_TARGETS_XML_WRITER_H__
#define __OG_TARGETS_XML_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include <cdk/ast/basic_node.h>
#include <cdk/types/types.h>

#ifndef tPUBLIC
#include "og_parser.tab.h"
#endif

namespace og {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class xml_writer: public basic_ast_visitor {
    cdk::symbol_table<og::symbol> &_symtab;
    std::shared_ptr<og::symbol> _function = nullptr;

  public:
    xml_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab) {
      // ensure builtin functions are available
      auto int_t = cdk::make_primitive_type(4, cdk::TYPE_INT);
      auto str_t = cdk::make_primitive_type(4, cdk::TYPE_STRING);
      auto tuple_empty_t = cdk::make_structured_type(std::vector<std::shared_ptr<cdk::basic_type>>());
      auto tuple_int_t = cdk::make_structured_type(std::vector<std::shared_ptr<cdk::basic_type>>(1, int_t));

      auto argc = std::make_shared<og::symbol>(tQUALIFIERUNSPEC, int_t, "argc", tuple_empty_t);
      auto argv = std::make_shared<og::symbol>(tQUALIFIERUNSPEC, str_t, "argv", tuple_int_t);
      auto envp = std::make_shared<og::symbol>(tQUALIFIERUNSPEC, str_t, "envp", tuple_int_t);

      for (auto sym : {argc, argv, envp}) {
        _symtab.insert(sym->name(), sym);
      }
    }

  public:
    ~xml_writer() {
      os().flush();
    }

  private:
    void openTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') << "<" << tag << ">" << std::endl;
    }
    void openTag(const cdk::basic_node *node, int lvl) {
      openTag(node->label(), lvl);
    }
    void closeTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') << "</" << tag << ">" << std::endl;
    }
    void closeTag(const cdk::basic_node *node, int lvl) {
      closeTag(node->label(), lvl);
    }
    void emptyTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') << '<' << tag << " />";
    }
    void emptyTag(const cdk::basic_node *node, int lvl) {
      emptyTag(node->label(), lvl);
    }

  protected:
    void do_binary_operation(cdk::binary_operation_node *const node, int lvl);
    void do_unary_operation(cdk::unary_operation_node *const node, int lvl);

    std::string xml_escape(int val) {
      return xml_escape(std::to_string(val));
    }

    std::string xml_escape(double val) {
      return xml_escape(std::to_string(val));
    }

    std::string xml_escape(std::string val) {
      std::string res = "";

      for (char c : val) {
        switch(c) {
          case '<':
            res += "&lt;";
            break;
          case '>':
            res += "&gt;";
            break;
          case '\'':
            res += "&apos;";
            break;
          case '"':
            res += "&quote;";
            break;
          case '&':
            res += "&amp;";
            break;
          default:
            res += c;
            break;
        }
      }

      return res;
    }

    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
      os() << std::string(lvl, ' ') << "<" << node->label() << " value='" << xml_escape(node->value()) << "' />" << std::endl;
    }

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

} // og

#endif
