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
#include <vector>
#include <memory>
#include "entities/EnemyFactory.hpp"
#include <deque>
#include "items/Item.hpp"
#include "items/ItemFactory.hpp"
#include "observers/StatsObserver.hpp"
#include "observers/LogObserver.hpp"

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
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::deque<std::string> messageLog_;
    std::vector<std::unique_ptr<Item>> items_;
    bool inventarioAberto_;
    StatsObserver statsObserver_;
    LogObserver   logObserver_;

    // --- Etapas do loop ---
    // Separar em métodos privados deixa run() limpo e legível
    void processInput();    // Lê o teclado e decide o que fazer
    void update();          // Atualiza o estado do mundo (movimento, combate, etc.)
    void render();          // Pede ao Renderer que desenhe o estado atual
    void pushMessage(const std::string& message);   // Adiciona mensagens para usuarios
    void coletarItem();     //coleta item para inventario do  player  
    void usarConsumivel();   //coleta intem no chão e consome sem subir ao inventario
    void usarConsumivelInventario(int index); //usa item (de 1 a 5) do inventario
    void descartarItem(int index); //desequipar 1 item por vez

    void equiparSelecionado(); //pega primeiro consumivel do inventario e equipa no slot correspondente, e notifica observers
    void desequiparSelecionado(); //tenta desequipar arma → armadura → acessório (o primeiro que encontrar), notifica observers

};
