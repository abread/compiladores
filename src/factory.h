#ifndef __OG_FACTORY_H__
#define __OG_FACTORY_H__

#include <memory>
#include <cdk/yy_factory.h>
#include "og_scanner.h"

namespace og {

  /**
   * This class implements the compiler factory for the Simple compiler.
   */
  class factory: public cdk::yy_factory<og_scanner> {
    /**
     * This object is automatically registered by the constructor in the
     * superclass' language registry.
     */
    static factory _self;

  protected:
    /**
     * @param language name of the language handled by this factory (see .cpp file)
     */
    factory(const std::string &language = "og") :
        cdk::yy_factory<og_scanner>(language) {
    }

  };

} // og

#endif
