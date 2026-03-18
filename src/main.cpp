// main.cpp — Ponto de entrada do programa
//
// Em C++, todo programa começa na função main().
// Nossa responsabilidade aqui é mínima: criar o jogo e iniciar.
// Toda a lógica fica dentro da classe Game — isso é Single Responsibility.

#include "core/Game.hpp"

int main() {
    // Criamos o objeto Game na stack (sem new/delete — veremos isso mais adiante)
    Game game;

    // Iniciamos o loop principal do jogo
    // Quando run() retornar, o jogo terminou
    game.run();

    return 0;
}
