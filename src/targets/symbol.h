#ifndef __OG_TARGETS_SYMBOL_H__
#define __OG_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace og {

  class symbol {
    int _qualifier;
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    std::shared_ptr<cdk::basic_type> _argsType; // nullptr for variables

    int _offset = 0; // 0 means global
    bool _definedOrInitialized = false;
    bool _autoType; // tracks if function/variable was originally auto

  public:
    symbol(int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &name, std::shared_ptr<cdk::basic_type> argsType = nullptr) :
        _qualifier(qualifier), _type(type), _name(name), _argsType(argsType) {
          _autoType = type->name() == cdk::TYPE_UNSPEC;
        }

    virtual ~symbol() {
      // EMPTY
    }

    int qualifier() const {
      return _qualifier;
    }
    void type(std::shared_ptr<cdk::basic_type> type) {
      _type = type;
    }
    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    int offset() const {
      return _offset;
    }
    int &offset() {
      return _offset;
    }
    bool global() const {
      return _offset == 0;
    }
    bool isFunction() const {
      return _argsType != nullptr;
    }
    bool isVariable() const {
      return _argsType == nullptr;
    }
    std::shared_ptr<cdk::basic_type> argsType() const {
      return _argsType;
    }
    bool definedOrInitialized() const {
      return _definedOrInitialized;
    }
    bool &definedOrInitialized() {
      return _definedOrInitialized;
    }
    bool autoType() const {
      return _autoType;
    }
  };

} // og

#endif
