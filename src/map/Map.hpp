#pragma once

// Map.hpp — Declara o mapa do dungeon
//
// Fase 2: adicionamos geração procedural com salas e corredores.
// O mapa agora guarda uma lista de salas geradas e expõe generate().

#include <vector>
#include <random>
#include "map/Room.hpp"

class Map {
public:
    Map(int width, int height);

    // generate(): apaga o mapa atual e gera um novo dungeon
    // seed: número que determina o dungeon gerado (mesma seed = mesmo mapa)
    void generate(unsigned int seed = 0);

    // Acesso aos tiles
    bool isWalkable(int x, int y) const;
    char getTile(int x, int y)    const;

    // Dimensões
    int getWidth()  const;
    int getHeight() const;

    // Retorna a lista de salas geradas (útil para posicionar jogador/inimigos)
    const std::vector<Room>& getRooms() const;

    //metodos do Fog of war
    bool isExplored(int x, int y) const;
    void updateVisibility(int px, int py);

private:
    int width_;
    int height_;
    std::vector<std::vector<char>> tiles_;
    std::vector<Room> rooms_;   // salas geradas pelo algoritmo

    // Preenche todo o mapa com '#' — chamado no início de generate()
    void fill(char tile);

    // Escava uma sala: preenche a área interna com '.'
    void carveRoom(const Room& room);

    // Corredores em L entre dois pontos
    // carveHorizontalCorridor: escava da coluna x1 até x2 na linha y
    // carveVerticalCorridor:   escava da linha y1 até y2 na coluna x
    void carveHorizontalCorridor(int x1, int x2, int y);
    void carveVerticalCorridor(int y1, int y2, int x);

    //campo do Fog of war
    std::vector<std::vector<bool>> explored_;
    
};
