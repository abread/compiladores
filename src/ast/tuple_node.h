#ifndef __OG_AST_TUPLE_NODE_H__
#define __OG_AST_TUPLE_NODE_H__

#include <vector>
#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace og {

  /**
   * Class representing a tuple.
   */
  class tuple_node: public cdk::expression_node {

    typedef std::vector<cdk::expression_node*> tuple_type;
    tuple_type _elements;

  public:
    /**
     * Constructor for the empty tuple.
     */
    tuple_node(int lineno) :
        cdk::expression_node(lineno) {}

    /**
     * Example: constructor for a left recursive production node:
     * <pre>
     * tuple: item {$$ = new Tuple(LINE, $1);}* | tuple item {$$ = new Tuple(LINE, $2, $1);}*
     * </pre>
     * The constructor of a tuple node takes the same first two
     * arguments as any other node.
     * The third argument is the previous tuple_node (from which all elements/nodes will be imported).
     *
     * @param lineno the source code line number that originated the node
     * @param el is the single element to be added to the tuple
     * @param prev is a previous tuple (nodes will be imported)
     */
    tuple_node(int lineno, cdk::expression_node *el, tuple_node *prev = nullptr) :
      cdk::expression_node(lineno)
    {
      if (prev != nullptr)
        _elements = prev->elements();
      _elements.push_back(el);
    }

  public:
    /**
     * This is the destructor for tuple nodes. Note that this
     * destructor also causes the destruction of the node's
     * children.
     */
    ~tuple_node() {
      for (auto elptr : _elements)
        delete elptr;
      _elements.clear();
    }

  public:
    cdk::expression_node *element(size_t i) {
      return _elements[i];
    }
    tuple_type &elements() {
      return _elements;
    }
    size_t size() {
      return _elements.size();
    }

    /**
     * @param av basic AST visitor
     * @param level syntactic tree level
     */
    void accept(basic_ast_visitor *av, int level) {
      av->do_tuple_node(this, level);
    }

  };

} // og

#endif
