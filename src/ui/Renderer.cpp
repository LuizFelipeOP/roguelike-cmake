// Renderer.cpp — Implementação da renderização no console

#include "ui/Renderer.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
    // No Windows usamos system("cls") para limpar a tela
    #include <windows.h>
#endif

void Renderer::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    // No Linux/Mac usamos sequência ANSI
    std::cout << "\033[2J\033[H";
#endif
}

void Renderer::render(const Map& map, const Player& player, const Enemy& enemy) {
    clearScreen();

    // Iteramos o mapa linha por linha (y = linha, x = coluna)
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {

            // Se a posição do jogador coincide com este tile, desenhamos o jogador
            // Caso contrário, desenhamos o tile do mapa
            if (x == player.getX() && y == player.getY()) {
                std::cout << player.getSymbol();
            } else {
                std::cout << map.getTile(x, y);
            }
        }
        // Quebra de linha ao fim de cada linha do mapa
        std::cout << '\n';
    }

    // Desenha o HUD logo abaixo do mapa
    renderHUD(player);
}

void Renderer::renderHUD(const Player& player) {
    // Linha separadora
    std::cout << std::string(40, '-') << '\n';

    // HP em formato "HP: atual/maximo"
    std::cout << " HP: " << player.getHp() << "/" << player.getMaxHp();
    std::cout << "   ATK: " << player.getAttack();
    std::cout << "   DEF: " << player.getDefense();
    std::cout << '\n';

    std::cout << std::string(40, '-') << '\n';
    std::cout << " [W/A/S/D] Mover   [Q] Sair\n";
}
