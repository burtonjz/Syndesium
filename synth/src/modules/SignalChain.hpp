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

#include "modules/BaseModule.hpp"

#include <unordered_set>
#include <vector>
#include <algorithm>

// This class will store information regarding tracing a signal back to it's source
// (either a generator module or eventually an audio input), and handle order of  operations of ticking
// through modules
class SignalChain {
private:
    std::unordered_set<BaseModule*> outputNodes_ ;
    std::vector<BaseModule*> topologicalOrder_ ;

    std::unordered_set<BaseModule*> visited_  ;

public:
    SignalChain():
        outputNodes_()
    {
    }

    std::vector<BaseModule*>& getModuleChain(){
        return topologicalOrder_ ;
    }

    std::unordered_set<BaseModule*>& getSinks(){
        return outputNodes_ ;
    }

    void addSink(BaseModule* output){
        if (!output){
            std::cerr << "WARN: attempted to add nullptr as a sink. " << std::endl ;
            return ;
        }
        outputNodes_.insert(output);
    }

    void removeSink(BaseModule* output){
        outputNodes_.erase(output);
    }

    void calculateTopologicalOrder(){
        visited_.clear();
        topologicalOrder_.clear();
        
        // global post-order depth-first search
        for (BaseModule* m : outputNodes_ ){
            topologicalSort(m, visited_, topologicalOrder_);
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
        for (BaseModule* m : module->getInputs()){
            topologicalSort(m, visited, result);
        }

        result.push_back(module); // post-order traversal (only insert once all inputs are  visited)
    }

};

#endif // __SIGNAL_CHAIN_HPP_