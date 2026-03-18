#pragma once

// Map.hpp — Declara o mapa do dungeon
//
// O mapa é uma grade 2D de tiles (células).
// Cada tile é representado por um char:
//   '#' = parede (não caminhável)
//   '.' = chão   (caminhável)
//
// Por que vector<vector<char>>?
//   É a estrutura mais simples para uma grade 2D em C++.
//   tiles_[y][x] acessa a linha y, coluna x.
//   Na Fase 2 vamos evoluir para um tipo Tile mais rico.

#include <vector>

class Map {
public:
    // Construtor: cria um mapa de dimensões width x height
    Map(int width, int height);

    // isWalkable(): retorna true se a posição (x,y) pode ser ocupada
    // Usado pelo Player antes de mover e pela IA dos inimigos
    bool isWalkable(int x, int y) const;

    // getTile(): retorna o char do tile na posição (x,y)
    // Usado pelo Renderer para desenhar o mapa
    char getTile(int x, int y) const;

    // Getters das dimensões — necessários para o Renderer iterar o mapa
    int getWidth()  const;
    int getHeight() const;

private:
    int width_;
    int height_;

    // Grade 2D de tiles
    // tiles_[y][x] — primeiro índice é a linha (y), segundo é a coluna (x)
    std::vector<std::vector<char>> tiles_;
};
