#ifndef __MODULATOR_HPP_
#define __MODULATOR_HPP_

#include "types/ModulatorType.hpp"
#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"

#include <memory>
#include <set>

// forward declaration
class ParameterMap ; 

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;

/**
 * @brief base class for all Modulators.
*/
class BaseModulator {
protected:
    ModulatorType type_ ;
    std::unique_ptr<ParameterMap> parameters_ ; 
    std::set<ModulationParameter> requiredParams_ ; 

public:
    BaseModulator(ModulatorType typ);

    virtual ~BaseModulator() = default ;

    virtual double modulate(double value, ModulationData* mdat ) const = 0 ;

    virtual ModulatorType getType() const ;

    ParameterMap* getParameters();

    virtual void setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d = {} );

    virtual const std::set<ModulationParameter>& getRequiredModulationParameters() const {
        return requiredParams_ ;
    }

    void tick();

};

#endif // __MODULATOR_HPP_


#include "containers/RTMap.hpp"

