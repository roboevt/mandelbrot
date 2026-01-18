//g++ -O3 -o main main.cpp engine.cpp -lSDL2 -fopenmp

#include <SDL2/SDL.h>
#include <iostream>

#include "engine.h"

const int WIDTH = 1920;
const int HEIGHT = 1080;

int main(int argc, char* argv[]) {
    
    Engine engine(WIDTH, HEIGHT);
    
    // Main loop
    while (engine.process_events()) {

        engine.render();

    }

    return 0;
}
