#pragma once
#include <SDL.h>
#include "TileID.hpp"
#include "SpriteSheet.hpp"

// TileInfo — resultado de TileAtlas::getInfo()
// Informa em qual spritesheet o tile se encontra e sua posição no grid.
struct TileInfo {
    SpriteSheet sheet;
    SDL_Rect    rect;
};

class TileAtlas {
public:
    static constexpr int TILE_W = 16;
    static constexpr int TILE_H = 24;

    // Retorna sheet + SDL_Rect para um dado TileID
    static TileInfo getInfo(TileID id);

private:
    static SDL_Rect fromGrid(int col, int linha);
};
