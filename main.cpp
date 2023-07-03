#include <SDL2/SDL.h>
#include <iostream>
#include "chip8.h"
#include <chrono>
#include <thread>
#include <map>
#include <algorithm>

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 480;
std::map<SDL_Scancode, int> KEYMAP {
    {SDL_SCANCODE_1, 0x1}, {SDL_SCANCODE_2, 0x2}, {SDL_SCANCODE_3, 0x3}, {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_Q, 0x4}, {SDL_SCANCODE_W, 0x5}, {SDL_SCANCODE_E, 0x6}, {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_A, 0x7}, {SDL_SCANCODE_S, 0x9}, {SDL_SCANCODE_D, 0x9}, {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_Z, 0xA}, {SDL_SCANCODE_X, 0x0}, {SDL_SCANCODE_C, 0xB}, {SDL_SCANCODE_V, 0xF}
};
const int FRAME_DELAY = 16666666;
class Display {
    public:
        Display() {}
        ~Display() {
            SDL_DestroyWindow(window);
            SDL_DestroyRenderer(renderer);
            SDL_Quit();
        }
        bool init() {
            window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            if (!window) {
                std::cerr << "SDL failed to create window: " << SDL_GetError();
                return false;
            }
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer) {
                std::cerr << "SDL failed to create renderer: " << SDL_GetError();
                return false;
            }
            return true;
        }
        bool update_display(bool chip8[][64]) {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            if(SDL_RenderClear(renderer)) return false;
            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    if (chip8[y][x]) {
                        fillRect.x = x*(SCREEN_WIDTH/64);
                        fillRect.y = y*(SCREEN_HEIGHT/32);
                        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        if(SDL_RenderFillRect(renderer, &fillRect)) return false;
                    }
                }
            }
            SDL_RenderPresent(renderer);
            return true;
        }
    private:
        SDL_Rect fillRect = {0, 0, SCREEN_WIDTH / 64, SCREEN_HEIGHT / 32 };
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
};


int main(int argc, char* argv[]) {
    Chip8 myChip8;
    Display myDisplay;
    if (argc != 2) {
        if (argc < 2) std::cerr << "No ROM provided" << std::endl;
        if (argc > 2) std::cerr << "Too many arguments" << std::endl;
        return 1;
    }
    if (!myChip8.load(argv[1])) {
        std::cerr << "Failed to open ROM" << std::endl;
        return 1;
    }
    if(!myDisplay.init()) return 1;
    srand(time(0));
    bool quit = false;
    SDL_Event e;
    auto prev_time = std::chrono::high_resolution_clock::now();
    while (!quit) {
        while(SDL_PollEvent(&e) != 0) {
            bool down = true;
            switch(e.type) {
                case SDL_QUIT: {
                    quit = true;
                    break;
                }
                case SDL_KEYUP: {
                    down = false;
                }
                case SDL_KEYDOWN: {
                    if (KEYMAP.find(e.key.keysym.scancode) != KEYMAP.end())
                        myChip8.keys[KEYMAP[e.key.keysym.scancode]] = down;
                }
            }
        }
        myChip8.emulate_cycle();
        if(!myDisplay.update_display(myChip8.display)) {
            std::cerr << "Failed to update display" << std::endl;
            return 1;
        }
        auto cur_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - prev_time).count();
        std::this_thread::sleep_for(std::chrono::nanoseconds(std::max((long long)0, FRAME_DELAY - cur_duration)));
        prev_time = std::chrono::high_resolution_clock::now();
    }

}