#ifndef __MODULE_CONTEXT_HPP_
#define __MODULE_CONTEXT_HPP_

#include "ApiClient.hpp"
#include "StateManager.hpp"

struct ModuleContext {
    ApiClient* apiClient ;
    StateManager* state ;
    QString objectName ;
};

#endif // __MODULE_CONTEXT_HPP_