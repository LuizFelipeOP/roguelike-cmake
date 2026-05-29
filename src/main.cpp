// main.cpp — Ponto de entrada do programa

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include "core/Game.hpp"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << '\n';
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init falhou: " << TTF_GetError() << '\n';
        SDL_Quit();
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init falhou: " << IMG_GetError() << '\n';
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* janela = SDL_CreateWindow(
        "Roguelike",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        960, 648,
        SDL_WINDOW_SHOWN
    );
    if (!janela) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << '\n';
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(janela);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    {
        Game game(renderer,
                  "assets/sprites/",
                  "assets/fonts/PressStart2P.ttf");
        game.run();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(janela);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
