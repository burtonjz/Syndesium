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
    std::unordered_set<Module::BaseModule*> outputNodes_ ;
    std::vector<Module::BaseModule*> topologicalOrder_ ;

    std::unordered_set<Module::BaseModule*> visited_  ;

public:
    SignalChain():
        outputNodes_()
    {
    }

    std::vector<Module::BaseModule*>& getModuleChain(){
        return topologicalOrder_ ;
    }

    std::unordered_set<Module::BaseModule*>& getSinks(){
        return outputNodes_ ;
    }

    void addSink(Module::BaseModule* output){
        if (!output){
            std::cerr << "WARN: attempted to add nullptr as a sink. " << std::endl ;
            return ;
        }
        outputNodes_.insert(output);
    }

    void removeSink(Module::BaseModule* output){
        outputNodes_.erase(output);
    }

    void calculateTopologicalOrder(){
        visited_.clear();
        topologicalOrder_.clear();
        
        // global post-order depth-first search
        for (Module::BaseModule* m : outputNodes_ ){
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
        Module::BaseModule* module, 
        std::unordered_set<Module::BaseModule*>& visited,
        std::vector<Module::BaseModule*>& result    
    ){
        if (visited.count(module)) return ;

        visited.insert(module);
        for (Module::BaseModule* m : module->getInputs()){
            topologicalSort(m, visited, result);
        }

        result.push_back(module); // post-order traversal (only insert once all inputs are  visited)
    }

};

#endif // __SIGNAL_CHAIN_HPP_