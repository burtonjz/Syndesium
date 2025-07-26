#ifndef __CONFIG_HPP_
#define __CONFIG_HPP_

#include <string>
#include <shared_mutex>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

class Config {
private:
    static std::string configPath_ ;
    static json configData_ ;
    static std::shared_mutex mutex_ ;

public:
    static void load();
    static void save();

    static void set(const std::string& dottedKey, const json& value);

    template <typename T>
    static std::optional<T> get(const std::string& dottedKey){
        std::shared_lock lock(mutex_);
        
        std::string_view keyView = dottedKey ;
        const json* jsonPtr = &configData_ ;

        while (!keyView.empty()){
            size_t dotPos = keyView.find('.');
            std::string_view segment = keyView.substr(0,dotPos);
            std::string keyStr(segment);
            
            // loop through curent json object for our current key
            auto it = jsonPtr->find(keyStr);
            
            // make sure property exists
            if (it == jsonPtr->end() ){
                return std::nullopt ;
            }

            // Move the pointer to either the next object or the end value
            jsonPtr = &(*it) ; 

            if (dotPos == std::string_view::npos) break ;

            keyView.remove_prefix(dotPos + 1);
        }

    if (jsonPtr->is_null()) {
        return std::nullopt ;
    }

    try {
        return jsonPtr->get<T>() ;
    } catch (const json::exception& e){
        return std::nullopt ;
    } 
}
};

#endif // __CONFIG_HPP_