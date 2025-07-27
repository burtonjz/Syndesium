#include "Engine.hpp"
#include "config/Config.hpp"

// Program Entry Point
int main() {
    Config::load();
    Engine engine ;
    engine.initialize();
}