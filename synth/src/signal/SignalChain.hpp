/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __SIGNAL_CHAIN_HPP_
#define __SIGNAL_CHAIN_HPP_

#include "core/BaseModule.hpp"

#include <unordered_set>
#include <vector>
#include <spdlog/spdlog.h>

// This class will store information regarding tracing a signal back to it's source
// (either a generator module or eventually an audio input), and handle order of  operations of ticking
// through modules
class SignalChain {
private:
    std::unordered_set<SignalConnection, ConnectionHash> outputNodes_ ;
    std::vector<BaseModule*> topologicalOrder_ ;
    std::unordered_set<BaseModule*> visited_  ;

    std::unordered_set<BaseModule*> modulatorOnly_ ;

public:
    SignalChain():
        outputNodes_()
    {
    }

    std::vector<BaseModule*>& getModuleChain(){
        return topologicalOrder_ ;
    }

    const std::unordered_set<SignalConnection, ConnectionHash>& getSinks() const {
        return outputNodes_ ;
    }

    void addSink(BaseModule* output, size_t index){
        if (!output){
            SPDLOG_WARN("Not adding a nullptr as a sink.");
            return ;
        }
        if ( index > output->getNumOutputs() ){
            SPDLOG_WARN("output index out of bounds for module. Cannot add requested sink.");
            return ;
        }
        outputNodes_.insert({output, index});
    }

    void removeSink(BaseModule* output, size_t index){
        if ( !output || index > output->getNumOutputs() ){
            SPDLOG_WARN("output index out of bounds for specified module. Cannot remove sink.");
            return ; 
        }

        outputNodes_.erase({output, index});
    }

    void calculateTopologicalOrder(){
        visited_.clear();
        topologicalOrder_.clear();
        
        // global post-order depth-first search
        for ( const auto& conn : outputNodes_ ){
            topologicalSort(conn.module, visited_, topologicalOrder_);
        }
    }

    void reset(){
        outputNodes_.clear();
        visited_.clear();
        topologicalOrder_.clear();
    }

    

private:
    /**
     * @brief calculate the DAG via depth-first traversal from module to its inputs
     * 
     * @param module module pointer
     * @param visited tracks visits during traversal
     * @param result resulting ordered vector
     */
    void topologicalSort(
        BaseModule* module, 
        std::unordered_set<BaseModule*>& visited,
        std::vector<BaseModule*>& result    
    ){
        if (visited.count(module)) return ;
        visited.insert(module);

        // Process stateful modulators in signal chain (e.g., Oscillator)
        for (BaseModule* m : module->getModulationInputs() ){
            topologicalSort(m, visited, result);
        }

        // Now process normal signal chain
        for ( size_t i = 0; i < module->getNumInputs(); ++i ){
            for ( const auto& conn : module->getInputs(i)){
                topologicalSort(conn.module, visited, result);
            }
        }
        
        result.push_back(module); // post-order traversal (only insert once all inputs are  visited)
    }

};

#endif // __SIGNAL_CHAIN_HPP_