#include "ApolloEngine.hpp"
#include "config/Config.hpp"

// Program Entry Point
int main() {
    Config::load();
    ApolloEngine engine ;
    engine.initialize();
}