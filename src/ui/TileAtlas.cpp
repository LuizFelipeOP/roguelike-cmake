#include "ui/TileAtlas.hpp"

SDL_Rect TileAtlas::fromGrid(int col, int linha) {
    return { col * TILE_W, linha * TILE_H, TILE_W, TILE_H };
}

TileInfo TileAtlas::getInfo(TileID id) {
    switch (id) {
        // ── Monsters.png ────────────────────────────────────────────────────
        case TileID::PLAYER:         return { SpriteSheet::Monsters, fromGrid( 2,   0) }; // guerreiro com espada
        case TileID::GOBLIN:         return { SpriteSheet::Monsters, fromGrid( 0,  10) }; // criatura pequena
        case TileID::ORC:            return { SpriteSheet::Monsters, fromGrid(15,  14) }; // humanoide grande
        case TileID::TROLL:          return { SpriteSheet::Monsters, fromGrid( 7,   8) }; // criatura robusta

        // ── Terrain.png ─────────────────────────────────────────────────────
        case TileID::PAREDE:         return { SpriteSheet::Terrain,  fromGrid( 0,  3) }; // parede frente — face visível (tijolo)
        case TileID::PAREDE_LATERAL: return { SpriteSheet::Terrain,  fromGrid( 0,  2) }; // parede corredor — sem cap
        case TileID::CHAO:           return { SpriteSheet::Terrain,  fromGrid(13,  0) }; // pedra/piso
        case TileID::ESCADA:         return { SpriteSheet::Terrain,  fromGrid(14,  1) }; // barras ascendentes

        // ── Items.png ───────────────────────────────────────────────────────
        case TileID::ITEM_POCAO:     return { SpriteSheet::Items,    fromGrid( 0,  3) }; // flask/béquer
        case TileID::ITEM_ESPADA:    return { SpriteSheet::Items,    fromGrid( 0,  0) }; // adaga/espada
        case TileID::ITEM_ARMADURA:  return { SpriteSheet::Items,    fromGrid( 0,  2) }; // túnica/armadura (row 2 início)
        case TileID::ITEM_AMULETO:   return { SpriteSheet::Items,    fromGrid(10,  2) }; // anel/amuleto

        // ── Fallback ─────────────────────────────────────────────────────────
        default:                     return { SpriteSheet::Terrain,  fromGrid(14,  5) };
    }
}
