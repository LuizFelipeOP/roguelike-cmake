// Map.cpp — Implementação do mapa
//
// =========================================================
//  DESAFIO PARA VOCE — leia com atenção antes de codificar
// =========================================================
//
// Implemente os métodos abaixo. Use os comentários como guia.
// Quando terminar, me mostre o código para revisarmos juntos!
//
// DICA: o construtor é o mais importante. Os outros são simples.

#include "map/Map.hpp"

// -------------------------------------------------------------
// constroi o mapa, onde temos y e x, onde os indices 0 e Max 
// são paredes (#) e o resto é chão '.'
// -------------------------------------------------------------
Map::Map(int width, int height) 
    : width_(width)
    , height_(height){
    tiles_.assign(height, std::vector<char>(width, '.'));
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        if(x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1){
            tiles_[y][x] = '#';
        }
          // verifique se (x,y) está na borda
      }
  }
}

// -------------------------------------------------------------
// Checa se o chão é '.', faz checagem de bounds antes (se o chão)
// esta dentro dos limites pisaveis
// -------------------------------------------------------------
bool Map::isWalkable(int x, int y) const {
    if(y >= 0  && y < height_ && x >= 0  && x < width_){
        return tiles_[y][x] != '#';
    }else return false;
}

// -------------------------------------------------------------
// retorna o piso solicitado, dentro dos bounds corretos
// -------------------------------------------------------------
char Map::getTile(int x, int y) const {
    if(y >= 0  && y < height_ && x >= 0  && x < width_){
        return tiles_[y][x];
    }else return '#';
}

// Estes dois são simples — pode implementar agora:
int Map::getWidth()  const { return width_; }
int Map::getHeight() const { return height_; }
