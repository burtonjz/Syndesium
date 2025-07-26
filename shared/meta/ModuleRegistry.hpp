#ifndef __SHARED_MODULE_REGISTRY_HPP_
#define __SHARED_MODULE_REGISTRY_HPP_

#include "meta/ModuleDescriptor.hpp"
#include "types/ModuleType.hpp"
#include <unordered_map>

namespace ModuleRegistry {

const ModuleDescriptor& getModuleDescriptor(ModuleType);
const std::unordered_map<ModuleType, ModuleDescriptor>& getAllModuleDescriptors();

};


#endif // __SHARED_MODULE_REGISTRY_HPP_