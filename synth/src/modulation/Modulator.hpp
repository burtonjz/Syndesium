#ifndef __MODULATOR_HPP_
#define __MODULATOR_HPP_

#include "types/ModulatorType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"
#include <memory>

// forward declaration
class ParameterMap ; 

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;

/**
 * @brief base class for all Modulators.
*/
class Modulator {
protected:
    ModulatorType type_ ;
    std::unique_ptr<ParameterMap> parameters_ ;

public:
    Modulator(ModulatorType typ);

    virtual ~Modulator() = default ;

    virtual double modulate(double value, ModulationData* mdat ) const = 0 ;

    virtual ModulatorType getType() const ;

    ParameterMap* getParameters();

};

#endif // __MODULATOR_HPP_


#include "containers/RTMap.hpp"

