#ifndef __SHARED_COMPONENT_REGISTRY_HPP_
#define __SHARED_COMPONENT_REGISTRY_HPP_

#include "meta/ComponentDescriptor.hpp"
#include <unordered_map>

namespace ComponentRegistry {

const ComponentDescriptor& getComponentDescriptor(ComponentType type);
const ComponentDescriptor& getComponentDescriptor(ModuleType type);
const ComponentDescriptor& getComponentDescriptor(ModulatorType type);

const std::unordered_map<ComponentType, ComponentDescriptor>& getAllComponentDescriptors();

};


#endif // __SHARED_COMPONENT_REGISTRY_HPP_