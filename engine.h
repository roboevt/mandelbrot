#include <SDL2/SDL.h>
#include <iostream>

class Engine {
    int width, height;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    double center_x = -0.5f;
    double center_y = 0.0f;
    double scale = 3.0f;

    static constexpr Uint32 pixel_format = SDL_PIXELFORMAT_ARGB32;

    uint64_t frames_rendered = 0;
    static constexpr uint64_t max_frames = 1000000000;

    Uint32 mapRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    Uint32 calculate_pixel(int x, int y);

public:
    Engine(int width, int height);

    void render();

    bool process_events();

    ~Engine();
};