#include "engine.h"

#include <complex>

Engine::Engine(int width, int height) : width(width), height(height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL_Init failed");
    }

    // Create a window
    window = SDL_CreateWindow("Framebuffer Software Rendering",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer failed");
    }

    // Create a texture with the same size as the window
    texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateTexture failed");
    }
}

Engine::~Engine() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Engine::render() {
    Uint32* pixels = nullptr;
    int pitch = 0;

     // Lock the texture for direct access to its pixel data
    if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) != 0) {
        std::cerr << "SDL_LockTexture Error: " << SDL_GetError() << std::endl;
    }

    // Write directly to the texture's pixel buffer
    // Here we fill the texture with a red diagonal line.
#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Calculate the position in the texture's pixel array
            const int index = y * (pitch / sizeof(Uint32)) + x;

            const int pixel = calculate_pixel(x, y);
            pixels[index] = pixel;
        }
    }

    // Unlock the texture after updating pixels
    SDL_UnlockTexture(texture);

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Present the renderer's content to the screen
    SDL_RenderPresent(renderer);

    frames_rendered += !paused; // TODO this is a nasty hack
}

bool Engine::process_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return false;
        }
        if( e.type == SDL_KEYDOWN ) {
            if( e.key.keysym.sym == SDLK_ESCAPE )
                return false;
        }
        if (e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) { // scroll up
                scale *= 0.8f;
            } else if (e.wheel.y < 0) { // scroll down
                scale *= 1.2f;
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            if (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                center_x -= e.motion.xrel * (scale / width);
                center_y -= e.motion.yrel * (scale / height);
            }
        }
        if( e.type == SDL_KEYDOWN ) {
            if( e.key.keysym.sym == SDLK_f ) {
                center_x = -0.5;
                center_y = 0.0;
                scale = 3.0;
            }
        }
        if( e.type == SDL_KEYDOWN ) {
            if( e.key.keysym.sym == SDLK_SPACE )
                paused = !paused;
        }
        if( e.type == SDL_KEYDOWN ) {
            if( e.key.keysym.sym == SDLK_LEFT )
                frames_rendered -= 1;
        }
        if( e.type == SDL_KEYDOWN ) {
            if( e.key.keysym.sym == SDLK_RIGHT )
                frames_rendered += 1;
        }
        if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                width = e.window.data1;
                height = e.window.data2;

                // Recreate texture with new size
                SDL_DestroyTexture(texture);
                texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, width, height);
                if (!texture) {
                    std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
                    throw std::runtime_error("SDL_CreateTexture failed");
                }
            }
        }
    }
    return true;
}

Uint32 Engine::calculate_pixel(int x, int y) {
    // Map pixel coordinates to the complex plane
    const double real = center_x + (x - width / 2.0) * scale / width;
    const double imag = center_y + (y - height / 2.0) * scale / height;

    const std::complex<double> c(real, imag);
    std::complex<double> z(0, std::sin(frames_rendered * 0.01));

    const int max_iterations = 256;
    int i = 0;

    while(i++ < max_iterations) {
        z = z * z + c;
        if(std::norm(z) > 4.0) {
            break;
        }
    }

    const uint8_t color = static_cast<uint8_t>(255 * i / max_iterations);
    return mapRGBA(color, color, color, color);
}