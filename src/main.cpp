#include "core/Application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        mviz::Application app(800, 600, "MViz - 3D Visualization Tool");
        if (!app.initialize()) {
            return -1;
        }
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return -1;
    }
} 