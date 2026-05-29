#pragma once

// SpriteSheet.hpp — Identificadores dos arquivos de spritesheet do jogo
//
// Cada valor representa um PNG separado em assets/sprites/.
// O SDLRenderer mantém uma textura carregada por valor deste enum.
// O TileAtlas usa este enum para indicar em qual sheet cada TileID se encontra.

enum class SpriteSheet {
    Monsters,        // Monsters.png        — player, goblin, orc, troll, ...
    Items,           // Items.png           — pocoes, armas, armaduras, amuletos, ...
    Terrain,         // Terrain.png         — paredes, chao, escadas, ...
    TerrainObjects,  // Terrain_Objects.png — objetos decorativos do terreno
    Interface        // Interface.png       — UI: frames, slots, ícones de painel
};
