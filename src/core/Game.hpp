#pragma once

// Game.hpp — Declaração da classe principal do jogo
//
// #pragma once é um "include guard" — garante que este header
// seja incluído apenas uma vez mesmo que vários arquivos o importem.
// É equivalente ao padrão #ifndef/#define/#endif mas mais simples.
//
// SOLID aplicado aqui:
//   - Single Responsibility: Game só coordena o loop. Não desenha, não gera mapa.
//   - Dependency Inversion: Game depende de abstrações (Map, Player, Renderer),
//     não de implementações concretas de console ou gráficos.

#include "entities/Player.hpp"
#include "map/Map.hpp"
#include "ui/Renderer.hpp"

class Game {
public:
    // Construtor: inicializa todos os subsistemas do jogo
    Game();

    // run(): inicia e mantém o loop principal até o jogador sair
    void run();

private:
    // --- Estado do jogo ---
    bool isRunning_;   // Controla se o loop deve continuar

    // --- Subsistemas ---
    // Usamos composição: Game "tem um" mapa, "tem um" jogador, "tem um" renderer
    // Isso é preferível à herança quando não há relação "é um"
    Map      map_;
    Player   player_;
    Renderer renderer_;

    // --- Etapas do loop ---
    // Separar em métodos privados deixa run() limpo e legível
    void processInput();  // Lê o teclado e decide o que fazer
    void update();        // Atualiza o estado do mundo (movimento, combate, etc.)
    void render();        // Pede ao Renderer que desenhe o estado atual
};
