// ═══════════════════════════════════════════════════════════════════════════
// SalaThema.cpp — Factory de estilos visuais por andar
// ═══════════════════════════════════════════════════════════════════════════
//
// COMO ADICIONAR UM NOVO ESTILO:
//
// 1. Abra tools/calibrate_sprites.html no browser
//    (inicie o servidor: tools\servidor.bat → http://localhost:8766)
//
// 2. Na seção Terrain.png, clique nos tiles desejados e anote col/row.
//    Cada tile tem 16×24 pixels. A posição no atlas é:
//       SDL_Rect = { col*16, row*24, 16, 24 }
//
//    Identifique:
//      chao         → tile de chão da sala
//      paredeFrente → parede com chão abaixo (face visível ao jogador)
//      paredeLateral→ parede cercada de paredes (sem cap/topo)
//      variantes[]  → rachaduras ou detalhes (opcional, até 6 tiles)
//
// 3. Escolha uma cor multiplicadora (255,255,255 = sem alteração):
//    - Use a seção "Paleta de Cores" do calibrador para referência
//    - Exemplos:
//        {255, 255, 255} = cor original do sprite
//        {180, 120,  80} = tom avermelhado / enferrujado
//        {100, 160, 200} = tom azulado / gélido
//        {160, 200, 140} = tom esverdeado / musgo
//
// 4. Adicione a entrada no array THEMAS[] abaixo:
//
//    THEMAS[N] = {
//        tg(col, row),         // chao
//        tg(col, row),         // paredeFrente
//        tg(col, row),         // paredeLateral
//        {                     // variantes[] — pode ter 0 a 6 entradas
//            tg(col, row),     //   variante 0
//            tg(col, row),     //   variante 1
//        },
//        2,                    // numVariantes — quantas entradas acima
//        255, 200, 180         // corR, corG, corB
//    };
//
// 5. Em Map.cpp, atualize o range do sorteio:
//    estilo_ = 1 + (rng() % N);  ← N = total de estilos
//
// 6. Atualize totalEstilos() abaixo para retornar N.
//
// ═══════════════════════════════════════════════════════════════════════════

#include "ui/SalaThema.hpp"

// Helper local — converte coluna/linha do grid para SDL_Rect (tiles 16×24)
static SDL_Rect tg(int col, int row) {
    return { col * 16, row * 24, 16, 24 };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tabela de estilos — índice 0 reservado (não usado, getThema começa em 1)
// ─────────────────────────────────────────────────────────────────────────────
static const SalaThema THEMAS[] = {

    // ── Estilo 0 — reservado (não sorteado) ──────────────────────────────────
    {},

    // ── Estilo 1 — Pedra cinza com borda de tijolos ───────────────────────────
    // Parede única (frente e lateral usam o mesmo tile: (0,3))
    // Variantes: rachaduras em (1,3) a (6,3)
    {
        tg(13, 0),  // chao
        tg( 0, 3),  // paredeFrente
        tg( 0, 3),  // paredeLateral
        { tg(1,3), tg(2,3), tg(3,3), tg(4,3), tg(5,3), tg(6,3) },
        6,
        200, 170, 140  // tom bege/pedra quente
    },

    // ── Estilo 2 — Pedra escura com parede de arco ────────────────────────────
    // Parede única (0,5) — variantes (1,5) a (6,5)
    {
        tg(13, 0),  // chao
        tg( 0, 5),  // paredeFrente
        tg( 0, 5),  // paredeLateral
        { tg(1,5), tg(2,5), tg(3,5), tg(4,5), tg(5,5), tg(6,5) },
        6,
        160, 140, 180  // tom arroxeado / masmorra profunda
    },

    // ── Estilo 3 — Caverna com pedra irregular ────────────────────────────────
    // Parede única (0,7) — variantes (1,7) a (6,7)
    {
        tg(14, 0),  // chao (textura diferente)
        tg( 0, 7),  // paredeFrente
        tg( 0, 7),  // paredeLateral
        { tg(1,7), tg(2,7), tg(3,7), tg(4,7), tg(5,7), tg(6,7) },
        6,
        140, 160, 140  // tom esverdeado / úmido
    },

    // ── Estilo 4 — Tijolo com distinção interna/externa ───────────────────────
    // Frente: (9,3) — Lateral: (8,3) — Variantes: (10,3) a (14,3)
    {
        tg(13, 0),  // chao
        tg( 9, 3),  // paredeFrente  (face externa com tijolo)
        tg( 8, 3),  // paredeLateral (face interna)
        { tg(10,3), tg(11,3), tg(12,3), tg(13,3), tg(14,3) },
        5,
        200, 120,  80  // tom avermelhado / tijolo queimado
    },

    // ── Estilo 5 — Pedra trabalhada com distinção interna/externa ─────────────
    // Frente: (9,5) — Lateral: (8,5) — Variantes: (10,5) a (14,5)
    {
        tg(13, 0),  // chao
        tg( 9, 5),  // paredeFrente
        tg( 8, 5),  // paredeLateral
        { tg(10,5), tg(11,5), tg(12,5), tg(13,5), tg(14,5) },
        5,
        100, 150, 200  // tom azulado / câmara de gelo
    },

    // ── Estilo 6 — Ruínas com textura pesada ─────────────────────────────────
    // Frente: (9,7) — Lateral: (8,7) — Variantes: (10,7) a (14,7)
    {
        tg(14, 0),  // chao (textura diferente)
        tg( 9, 7),  // paredeFrente
        tg( 8, 7),  // paredeLateral
        { tg(10,7), tg(11,7), tg(12,7), tg(13,7), tg(14,7) },
        5,
        180, 160,  80  // tom dourado / ruínas antigas
    },
};

static constexpr int TOTAL_ESTILOS = 6;

// ─────────────────────────────────────────────────────────────────────────────

const SalaThema& SalaThemaFactory::getThema(int estilo) {
    // Proteção de range: mapeia ciclicamente para 1–TOTAL_ESTILOS
    int idx = ((estilo - 1) % TOTAL_ESTILOS) + 1;
    return THEMAS[idx];
}

int SalaThemaFactory::totalEstilos() {
    return TOTAL_ESTILOS;
}
