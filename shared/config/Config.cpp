#include "Config.hpp"
#include <fstream>
#include "filesystem"
#include <string_view>
#include <mutex>

std::string getExecutableDir(){ // TODO: this only works for linux currently
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count == -1) return {};
    return std::filesystem::path(std::string(result, count)).parent_path().string();
}

std::string Config::configPath_ = std::filesystem::path(getExecutableDir()) / ".." / "shared" / "config.json" ;
json Config::configData_ ;
std::shared_mutex Config::mutex_ ;

void Config::load(){
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath_ );
    }

    json temp;
    file >> temp;

    std::unique_lock lock(mutex_);
    configData_ = std::move(temp);
}

void Config::save(){
    std::shared_lock readLock(mutex_);
    std::ofstream file(configPath_);
    if (!file.is_open()){
        throw std::runtime_error("Could not open config file: " + configPath_ );
    }
    file << configData_.dump(4);
}

void Config::set(const std::string& dottedKey, const json& value){
    std::unique_lock lock(mutex_);

    // parse dotted json key, e.g., server.port -> json['server']['port']
    std::string_view keyView = dottedKey ;
    json* jsonPtr = &configData_ ;

    while (!keyView.empty()) {
        size_t dotPos = keyView.find('.');
        std::string_view segment = keyView.substr(0,dotPos);

        if (dotPos == std::string_view::npos) {
            // all dots have been parsed, assign value
            (*jsonPtr)[std::string(segment)] = value ;
        } else {
            // not at end dot, set the JSON pointer to the next level up
            std::string keyStr(segment) ;
            jsonPtr = &(*jsonPtr)[keyStr] ; 
        }   
    }
}