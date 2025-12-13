# How to Add a New Component

## Prerequisites
- Understand the three base classes: `BaseModule` (signal processing), `BaseModulator` (modulation source), `MidiEventHandler` (MIDI input)
- Your component can inherit from any combination of these (they all inherit from `BaseComponent`)

---

## Step 1: Specify Component Type
**Location:** `shared/types/ComponentType.hpp`


## Step 2: Create Component Files
**Location:** `synth/src/components/`

Create `MyComponent.hpp` and `MyComponent.cpp`

**Constructor signature (required):**
```cpp
MyComponent(ComponentId id, const MyComponentConfig& config):
    BaseComponent(id, ComponentType::MyComponent)
```

---

## Step 3: Define Parameters
1. If using new parameter types, create them first (see: "How to Add a New Parameter Type")
2. All parameters must be added to the parameter map in your component's constructor
3. Set initial values using config data

---

## Step 4: Implement Base Class Requirements
All Components must inherit from BaseComponent, but it is generally expected that your component will inherit this indirectly through at least one of the following classes:
- `BaseModule` - for signal processing 
- `BaseModulator` - for modulation sources 
- `MidiEventHandler` - for MIDI input handling

In addition, it may be necessary to inherit from:
- `MidiEventListener` - for MIDI input listening 
  
Check each base class for necessary overrides.

---

## Step 5: Handle Child Components (If Applicable)
If your component will manage any child components, it is responsible for doing so through overrides -- there is no centralized pattern for managing child components.

See `PolyphonicOscillator` for an appropriate example example of implementing a component with children. However, the following is likely necessary: 
```cpp

void getParameterModulator(ParameterType p) override {
    // if children are modulatable, we need to be able to 
}

// from BaseComponent
void updateParameters() override {
    parameters_->modulate();  // Update this component's params first
    
    // Then update all children (example patterns):
    
    // Pattern A: children stored in an array
    for (auto& child : children_) {
        child.updateParameters();
    }
    
    // Pattern B: Polyphonic Children (using FixedPool)
    childPool_.forEachActive([](ChildType& child) {
        child.updateParameters();
    });
}

void onSetParameterModulation(ParameterType p, BaseModulator* m) override {
    if ( d.empty() && m ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d[mp];
        }
    }

    // store in parent arrays for reference
    modulators_[p] = m ;
    modulationData_[p] = d ;

    // apply to each child
    childPool_.forEachActive(&Oscillator::setParameterModulation, p, m, d);
}

void onRemoveParameterModulation(ParameterType param) override {
    BaseComponent::onRemoveParameterModulation(param);
    // Remove modulation from children if needed
}

// from BaseModule
void tick(){
    BaseModule::tick(); // you must run the parent tick function

    // perform any additional per-sample actions

    // Then, explicitly tick child objects

    // Pattern A: children stored in an array
    for (auto& child : children_) {
        child.tick()
    }
    
    // Pattern B: Polyphonic Children (using FixedPool)
    childPool_.forEachActive([](ChildType& child) {
        child.tick()
    });
}

```

If NO children, you can skip this step.

---

## Step 6: Create Component Config
**Location:** `synth/src/configs/`

Create `MyComponentConfig.hpp`:
```cpp
class MyComponent ; // forward declaration

struct MyComponentConfig {
    // Include all parameters needed for construction
    Waveform waveform = Waveform::SINE ;
    double frequency = 440.0f ;
};

template <> struct ComponentTypeTraits<ComponentType::MyComponent>{ 
    using type = MyComponent ;
    using config = MyComponentConfig ;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MyComponentConfig, waveform, frequency) // macro to serialize/deserialize json <-> structs
```

Update `ComponentConfig.hpp`:

1. Include your config header: `#include "configs/MyComponentConfig.hpp"`
2. Add macro to switch statement

---

## Step 7: Register with Factory System
**Location:** `synth/src/core/ComponentFactory.cpp`

1. Include your component header: `#include "components/MyComponent.hpp"`
2. Add to the macro in `ComponentFactory::createFromJson()`:

---

## Step 8: Register Component Metadata
**Location:** `shared/meta/ComponentRegistry.cpp`

Add an entry containing:
- Component type identifier
- Display name
- Parameter list
- I/O configuration
- Any other metadata needed by frontend/backend

This registry is shared between frontend and backend.
---

## Step 9: Frontend Integration (Optional)
**Location:** `gui/widgets/ComponentDetailWidget.hpp`

Only needed if you want custom parameter visualizations for a newly created parameter

Update `ComponentDetailWidget::createParameter()`:

---