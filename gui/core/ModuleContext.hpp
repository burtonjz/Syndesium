#ifndef __MODULE_CONTEXT_HPP_
#define __MODULE_CONTEXT_HPP_

#include "core/ApiClient.hpp"
#include "core/StateManager.hpp"

struct ModuleContext {
    ApiClient* apiClient ;
    StateManager* state ;
    QString objectName ;
};

#endif // __MODULE_CONTEXT_HPP_