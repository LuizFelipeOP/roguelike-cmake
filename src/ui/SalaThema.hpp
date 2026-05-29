#pragma once
#include <SDL.h>

// ═══════════════════════════════════════════════════════════════════════════
// SalaThema.hpp — Define o visual de um andar do dungeon
//
// Cada SalaThema descreve os sprites e a cor usados para renderizar as
// paredes e o chão de TODAS as salas de um andar.
// Corredores sempre usam PAREDE_LATERAL (tile fixo, sem thema).
//
// Campo corR/corG/corB: multiplicador de cor via SDL_SetTextureColorMod.
//   255, 255, 255 = sem alteração de cor (padrão)
//   180, 120,  80 = tom avermelhado / enferrujado
//   100, 160, 200 = tom azulado / gélido
// ═══════════════════════════════════════════════════════════════════════════

struct SalaThema {
    SDL_Rect chao;               // tile de chão renderizado dentro da sala
    SDL_Rect paredeFrente;       // parede com chão abaixo — face visível ao jogador
    SDL_Rect paredeLateral;      // parede sem face — cercada de outras paredes
    SDL_Rect variantes[6];       // variantes aleatórias (rachaduras, detalhes)
    int      numVariantes;       // quantas variantes estão definidas (0–6)
    Uint8    corR, corG, corB;   // multiplicador de cor (255,255,255 = sem alteração)
};

// ─────────────────────────────────────────────────────────────────────────────
// SalaThemaFactory — retorna o thema pelo índice do estilo (1–6)
//
// Proteção de range: índices fora de 1–6 são mapeados ciclicamente.
// ─────────────────────────────────────────────────────────────────────────────
namespace SalaThemaFactory {
    const SalaThema& getThema(int estilo);
    int totalEstilos();
}
