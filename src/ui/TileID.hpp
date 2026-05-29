#pragma once


enum class TileID {
    // Monsters.png
    PLAYER,
    GOBLIN,
    ORC,
    TROLL,

    // Terrain.png
    PAREDE,
    PAREDE_LATERAL,  // (0,2) — parede de corredor, sem face visível
    CHAO,
    ESCADA,

    // Items.png
    ITEM_POCAO,
    ITEM_ESPADA,
    ITEM_ARMADURA,
    ITEM_AMULETO,

    DESCONHECIDO
};