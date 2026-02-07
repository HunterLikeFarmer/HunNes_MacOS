#include "../include/Common/SDLCompat.hpp"

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#if defined(__has_include)
#if __has_include(<filesystem>)
#include <filesystem>
#endif
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "No usable filesystem API found. Use C++17 or provide <experimental/filesystem>."
#endif
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include "../include/6502.hpp"
#include "../include/Controller.hpp"
#include "../include/Mapper/Mapper.hpp"
#include "../include/PPU.hpp"
#include "../include/ROM.hpp"

namespace {

bool toControllerButtonFromKey(SDL_Keycode key, HunNes::ControllerButton& outButton) {
    switch (key) {
        case SDLK_a:
            outButton = HunNes::ControllerButton::A;
            return true;
        case SDLK_b:
            outButton = HunNes::ControllerButton::B;
            return true;
        case SDLK_SPACE:
            outButton = HunNes::ControllerButton::Select;
            return true;
        case SDLK_RETURN:
            outButton = HunNes::ControllerButton::Start;
            return true;
        case SDLK_UP:
            outButton = HunNes::ControllerButton::Up;
            return true;
        case SDLK_DOWN:
            outButton = HunNes::ControllerButton::Down;
            return true;
        case SDLK_LEFT:
            outButton = HunNes::ControllerButton::Left;
            return true;
        case SDLK_RIGHT:
            outButton = HunNes::ControllerButton::Right;
            return true;
        default:
            return false;
    }
}

bool toControllerButtonFromGamepadButton(Uint8 button, HunNes::ControllerButton& outButton) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A:
            outButton = HunNes::ControllerButton::A;
            return true;
        case SDL_CONTROLLER_BUTTON_B:
            outButton = HunNes::ControllerButton::B;
            return true;
        case SDL_CONTROLLER_BUTTON_START:
            outButton = HunNes::ControllerButton::Start;
            return true;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            outButton = HunNes::ControllerButton::Up;
            return true;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            outButton = HunNes::ControllerButton::Down;
            return true;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            outButton = HunNes::ControllerButton::Left;
            return true;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            outButton = HunNes::ControllerButton::Right;
            return true;
        default:
            return false;
    }
}

}  // namespace

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    std::string romPath = "./rom";
    std::vector<std::string> romFiles;

    // Check if the rom directory exists and gather all files
    if (!fs::exists(romPath)) {
        std::cout << "ROM directory not found. Please create a './rom' directory and place ROM files inside." << std::endl;
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(romPath)) {
        if (fs::is_regular_file(entry.path())) {
            std::string s = entry.path().string();
            romFiles.push_back(s);
        }
    }

    if (romFiles.empty()) {
        std::cout << "No ROM files found in the './rom' directory. Please add some ROM files to play." << std::endl;
        return 1;
    }

    // Display the available ROM files to the user
    std::cout << "Available ROMs:" << std::endl;
    for (size_t i = 0; i < romFiles.size(); ++i) {
        std::string s = romFiles[i];
        size_t found = s.find(".nes");
        std::string s_trimmed = s.substr(6, found + 1);

        std::cout << i + 1 << ". " << s_trimmed << std::endl;
    }

    // Prompt the user to select a ROM
    int selection = 0;
    const int romCount = static_cast<int>(romFiles.size());
    while (selection < 1 || selection > romCount) {
        std::cout << "Select a ROM to play (1-" << romCount << "): ";
        std::cin >> selection;
        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            selection = 0; // Reset selection
        }
    }

    std::string selectedRom = romFiles[selection - 1];
    std::cout << "Selected ROM: " << selectedRom << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cout << "SDL could not initialize. " << SDL_GetError() << std::endl;
        return 1;
    }
    // This is from extern library
    [[maybe_unused]] SDL_GameController* con = nullptr;

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            con = SDL_GameControllerOpen(i);
            std::cout << "Controller detected." << std::endl;
            break;
        }
    }

    SDL_Window* window;
    std::string window_title = "NES Emulator";

    window = SDL_CreateWindow(
        window_title.c_str(),     // window title
        SDL_WINDOWPOS_UNDEFINED,  // initial x position
        SDL_WINDOWPOS_UNDEFINED,  // initial y position
        512,                      // width, in pixels
        480,                      // height, in pixels
        SDL_WINDOW_SHOWN          // flags - see below
    );

    if (window == nullptr) {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* s = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    HunNes::ROM rom;
    rom.open(selectedRom);
    rom.printHeader();
    HunNes::Mapper* mapper = rom.getMapper();   // This is where we choose the mapper. It came with the ROM

    if (mapper == nullptr) {
        std::cout << "Unknown mapper." << std::endl;
        return 1;
    }

    auto ppu = HunNes::PPU(mapper);
    HunNes::Controller controller;
    auto cpu = HunNes::CPU6502(mapper, &ppu, &controller);
    cpu.reset();    // Move the PC to 
    SDL_Texture* texture = SDL_CreateTexture(s, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 240);

    // Performance variables
    int nmiCounter = 0;
    float duration = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    bool is_running = true;

    SDL_Event event;

    while (is_running) {
        cpu.step();

        if (ppu.generateFrame) {
            // Poll controller
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_CONTROLLERBUTTONDOWN:
                    {
                        HunNes::ControllerButton button;
                        if (toControllerButtonFromGamepadButton(event.cbutton.button, button)) {
                            controller.setButtonPressed(button, true);
                        }
                    }
                    break;
                case SDL_CONTROLLERBUTTONUP:
                    {
                        HunNes::ControllerButton button;
                        if (toControllerButtonFromGamepadButton(event.cbutton.button, button)) {
                            controller.setButtonPressed(button, false);
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    {
                        HunNes::ControllerButton button;
                        if (toControllerButtonFromKey(event.key.keysym.sym, button)) {
                            controller.setButtonPressed(button, true);
                        }
                    }
                    break;
                case SDL_KEYUP:
                    {
                        HunNes::ControllerButton button;
                        if (toControllerButtonFromKey(event.key.keysym.sym, button)) {
                            controller.setButtonPressed(button, false);
                        }
                    }
                    break;
                case SDL_QUIT:
                    is_running = false;
                    break;
                default:
                    break;
                }
            }

            // Measure FPS
            nmiCounter++;
            auto t2 = std::chrono::high_resolution_clock::now();
            duration += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            t1 = std::chrono::high_resolution_clock::now();

            if (nmiCounter == 10) {
                float avgFps = 1000 / (duration / nmiCounter);
                std::string fpsTitle = window_title + " (FPS: " + std::to_string((int)avgFps) + ")";
                SDL_SetWindowTitle(window, fpsTitle.c_str());
                nmiCounter = 0;
                duration = 0;
            }

            // Draw frame
            ppu.generateFrame = false;
            SDL_RenderSetScale(s, 2, 2);
            // This is how the PPU buffer (much like MMIO) gets mapped to the window
            SDL_UpdateTexture(texture, NULL, ppu.buffer, 256 * sizeof(Uint32));
            SDL_RenderClear(s);
            SDL_RenderCopy(s, texture, NULL, NULL);
            SDL_RenderPresent(s);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
