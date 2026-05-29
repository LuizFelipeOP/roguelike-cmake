# Phase 10 — SDL2 + Sprites — Plano de Implementação

**Spec:** `docs/superpowers/specs/2026-05-21-phase10-sdl2-sprites.md`

---

## Chunk 1: SDL2 no CMake + janela vazia

### Task 1.1 — FetchContent: SDL2, SDL2_image, SDL2_ttf

**Arquivo:** `CMakeLists.txt`

Adicionar após o bloco do nlohmann/json:

```cmake
# ── SDL2 ──────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.28.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL2)

# ── SDL2_image ────────────────────────────────────────────────────────────────
FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-2.8.2
    GIT_SHALLOW    TRUE
)
set(SDL2IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(SDL2_image)

# ── SDL2_ttf ──────────────────────────────────────────────────────────────────
FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-2.22.0
    GIT_SHALLOW    TRUE
)
set(SDL2TTF_INSTALL OFF)
FetchContent_MakeAvailable(SDL2_ttf)
```

No target do executável, linkar:

```cmake
target_link_libraries(roguelike PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    nlohmann_json::nlohmann_json
)
```

> **Nota:** SDL2 via FetchContent no Windows/MSVC pode exigir copiar as DLLs para o diretório do executável. Adicionar ao CMakeLists.txt após o target:
> ```cmake
> add_custom_command(TARGET roguelike POST_BUILD
>     COMMAND ${CMAKE_COMMAND} -E copy_if_different
>         $<TARGET_FILE:SDL2::SDL2>
>         $<TARGET_FILE_DIR:roguelike>
> )
> ```

- [ ] Buildar (`cmake -S . -B build` + `cmake --build build`) — deve compilar sem erros.

---

### Task 1.2 — `main.cpp`: janela SDL vazia

Substituir o `main.cpp` atual por:

```cpp
// main.cpp — Ponto de entrada com SDL2
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Roguelike",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        960, 648,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

> O `Game` não é instanciado ainda. O objetivo é apenas confirmar que o SDL abre janela no seu ambiente.

- [ ] Executar `build\Debug\roguelike.exe` — janela preta deve abrir e fechar com ESC.

**Critério de aceite do Chunk 1:** janela preta abre, fecha com ESC, sem crash.

---

## Chunk 2: TileAtlas + TileID

### Task 2.1 — `TileID.hpp`

**Arquivo:** `src/ui/TileID.hpp` (criar)

```cpp
// TileID.hpp — Identificadores de tiles do spritesheet Oryx Roguelike 2.0
//
// Cada TileID corresponde a um sprite no arquivo oryx_roguelike_16x24.png.
// As coordenadas (col, row) são mapeadas em TileAtlas.cpp.
#pragma once

enum class TileID {
    // Terreno
    Wall,
    Floor,
    StairsDown,

    // Entidades — player
    Player,

    // Entidades — inimigos
    Goblin,
    Orc,
    Troll,

    // Itens — equipamentos
    Sword,
    SwordLarge,
    Armor,
    Amulet,

    // Itens — consumíveis
    PotionSmall,
    Potion,
    PotionStrength,
    PotionRegen,

    // Fallback
    Unknown
};
```

- [ ] Arquivo criado.

---

### Task 2.2 — `TileAtlas.hpp`

**Arquivo:** `src/ui/TileAtlas.hpp` (criar)

```cpp
// TileAtlas.hpp — Carrega o spritesheet e fornece acesso a tiles individuais
//
// Pattern: Facade sobre SDL_Texture + cálculo de SDL_Rect
// O atlas é responsável por:
//   1. Carregar o PNG via SDL_image
//   2. Calcular a posição de recorte de qualquer tile por (col, row)
//   3. Desenhar um tile numa posição de destino na tela
#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "ui/TileID.hpp"

class TileAtlas {
public:
    // tileW / tileH: dimensões de cada tile em pixels (16x24 para Oryx 2.0)
    TileAtlas(SDL_Renderer* renderer, const std::string& path, int tileW, int tileH);
    ~TileAtlas();

    // Não copiável — SDL_Texture é um recurso exclusivo
    TileAtlas(const TileAtlas&)            = delete;
    TileAtlas& operator=(const TileAtlas&) = delete;

    // Retorna o SDL_Rect de recorte para o tile na coluna col, linha row do spritesheet
    SDL_Rect getSourceRect(int col, int row) const;

    // Desenha o tile (col, row) na posição de destino (destX, destY)
    // colorR/G/B: modulação de cor — (255,255,255) = cor original; (80,80,80) = escurecido
    void draw(SDL_Renderer* renderer, int col, int row,
              int destX, int destY,
              Uint8 colorR = 255, Uint8 colorG = 255, Uint8 colorB = 255) const;

    // Atalho: recebe TileID diretamente
    void draw(SDL_Renderer* renderer, TileID id,
              int destX, int destY,
              Uint8 colorR = 255, Uint8 colorG = 255, Uint8 colorB = 255) const;

private:
    SDL_Texture* texture_;
    int tileW_;
    int tileH_;
};

// Retorna as coordenadas (col, row) no spritesheet para um TileID.
// ATENÇÃO: os valores precisam ser conferidos no preview_*.png do pack antes de usar.
std::pair<int,int> tileCoords(TileID id);
```

---

### Task 2.3 — `TileAtlas.cpp`

**Arquivo:** `src/ui/TileAtlas.cpp` (criar)

```cpp
#include "ui/TileAtlas.hpp"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Mapeamento TileID → (col, row) no spritesheet oryx_roguelike_16x24.png
//
// COMO DESCOBRIR AS COORDENADAS:
//   1. Abra o arquivo src/sprites/V1/preview_*.png correspondente.
//   2. Cada tile tem 16px de largura e 24px de altura no spritesheet.
//   3. Conte as colunas da esquerda (0, 1, 2...) e as linhas de cima (0, 1, 2...).
//   4. Atualize os valores abaixo com o que encontrar.
//
// Os valores atuais são PLACEHOLDERS — precisam ser corrigidos!
// ─────────────────────────────────────────────────────────────────────────────
std::pair<int,int> tileCoords(TileID id) {
    switch (id) {
        // Terreno — consultar preview_dungeons.png
        case TileID::Wall:        return {0,  0};  // TODO: confirmar
        case TileID::Floor:       return {1,  0};  // TODO: confirmar
        case TileID::StairsDown:  return {2,  0};  // TODO: confirmar

        // Player — consultar preview_classes.png ou Avatar.png
        case TileID::Player:      return {0,  1};  // TODO: confirmar

        // Inimigos — consultar preview_creatures.png
        case TileID::Goblin:      return {0,  2};  // TODO: confirmar
        case TileID::Orc:         return {1,  2};  // TODO: confirmar
        case TileID::Troll:       return {2,  2};  // TODO: confirmar

        // Itens — consultar preview_items.png
        case TileID::Sword:         return {0,  3};  // TODO: confirmar
        case TileID::SwordLarge:    return {1,  3};  // TODO: confirmar
        case TileID::Armor:         return {2,  3};  // TODO: confirmar
        case TileID::Amulet:        return {3,  3};  // TODO: confirmar
        case TileID::PotionSmall:   return {4,  3};  // TODO: confirmar
        case TileID::Potion:        return {5,  3};  // TODO: confirmar
        case TileID::PotionStrength:return {6,  3};  // TODO: confirmar
        case TileID::PotionRegen:   return {7,  3};  // TODO: confirmar

        default:                  return {0,  0};
    }
}

TileAtlas::TileAtlas(SDL_Renderer* renderer, const std::string& path, int tileW, int tileH)
    : texture_(nullptr), tileW_(tileW), tileH_(tileH)
{
    texture_ = IMG_LoadTexture(renderer, path.c_str());
    if (!texture_) {
        throw std::runtime_error(std::string("TileAtlas: falha ao carregar ") + path
                                 + " — " + IMG_GetError());
    }
}

TileAtlas::~TileAtlas() {
    if (texture_) SDL_DestroyTexture(texture_);
}

SDL_Rect TileAtlas::getSourceRect(int col, int row) const {
    return SDL_Rect{ col * tileW_, row * tileH_, tileW_, tileH_ };
}

void TileAtlas::draw(SDL_Renderer* renderer, int col, int row,
                     int destX, int destY,
                     Uint8 colorR, Uint8 colorG, Uint8 colorB) const {
    SDL_Rect src  = getSourceRect(col, row);
    SDL_Rect dest = { destX, destY, tileW_, tileH_ };
    SDL_SetTextureColorMod(texture_, colorR, colorG, colorB);
    SDL_RenderCopy(renderer, texture_, &src, &dest);
    SDL_SetTextureColorMod(texture_, 255, 255, 255);  // reset
}

void TileAtlas::draw(SDL_Renderer* renderer, TileID id,
                     int destX, int destY,
                     Uint8 colorR, Uint8 colorG, Uint8 colorB) const {
    auto [col, row] = tileCoords(id);
    draw(renderer, col, row, destX, destY, colorR, colorG, colorB);
}
```

- [ ] Adicionar `src/ui/TileAtlas.cpp` ao `CMakeLists.txt` (na lista de sources do target roguelike).
- [ ] Buildar — sem erros.
- [ ] **Tarefa manual:** abrir os arquivos `preview_*.png` e atualizar as coordenadas no `tileCoords()`.

**Critério de aceite do Chunk 2:** compila; `tileCoords()` tem valores reais (não apenas placeholders).

---

## Chunk 3: SDLRenderer — mapa + entidades

### Task 3.1 — `SDLRenderer.hpp`

**Arquivo:** `src/ui/SDLRenderer.hpp` (criar)

```cpp
// SDLRenderer.hpp — Renderer gráfico usando SDL2 + sprites Oryx Roguelike 2.0
//
// Substitui completamente o Renderer de console.
// Recebe SDL_Renderer* externamente (criado em main.cpp) — não é dono do renderer.
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <memory>
#include <deque>
#include <string>
#include "ui/TileAtlas.hpp"
#include "ui/TileID.hpp"
#include "map/Map.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "items/Item.hpp"
#include "items/Inventario.hpp"

static constexpr int TILE_W    = 16;
static constexpr int TILE_H    = 24;
static constexpr int MAP_COLS  = 60;
static constexpr int MAP_ROWS  = 22;
static constexpr int HUD_Y     = MAP_ROWS * TILE_H;   // 528
static constexpr int WIN_W     = MAP_COLS * TILE_W;   // 960
static constexpr int WIN_H     = HUD_Y + 120;         // 648

class SDLRenderer {
public:
    // renderer: ponteiro não-owning para o SDL_Renderer criado em main.cpp
    // spritePath: caminho para oryx_roguelike_16x24.png
    // fontPath: caminho para o arquivo .ttf do HUD
    SDLRenderer(SDL_Renderer* renderer,
                const std::string& spritePath,
                const std::string& fontPath);
    ~SDLRenderer();

    SDLRenderer(const SDLRenderer&)            = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

    // Renderização principal do jogo
    void render(const Map& map, const Player& player,
                const std::vector<std::unique_ptr<Enemy>>& enemies,
                const std::vector<std::unique_ptr<Item>>& items,
                const std::deque<std::string>& messageLog,
                bool inventarioAberto, int andarAtual);

    // Telas especiais
    void renderMenu(bool temSave, SDL_Texture* background);
    void renderTelaDerrota(int andar, int nivel, int xp, int inimigos);
    void renderHistorico(const std::vector<std::string>& historico);

private:
    SDL_Renderer* renderer_;   // não-owning
    TileAtlas     atlas_;
    TTF_Font*     font_;
    TTF_Font*     fontSmall_;

    // Helpers de renderização
    void renderMapa(const Map& map,
                    const std::vector<std::unique_ptr<Enemy>>& enemies,
                    const std::vector<std::unique_ptr<Item>>& items,
                    const Player& player);

    void renderHUD(const Player& player,
                   const std::deque<std::string>& messageLog,
                   int andarAtual);

    void renderInventario(const Inventario& inv);

    // Desenha um tile do mapa na posição de tela (tileX, tileY)
    // com modulação de cor baseada na visibilidade
    void drawTerrain(char tile, int tileX, int tileY, bool visivel, bool explorado);

    // Retorna o TileID correspondente ao tipo do inimigo
    TileID tileIDForEnemy(const Enemy& e) const;

    // Retorna o TileID correspondente ao tipo do item
    TileID tileIDForItem(const Item& item) const;

    // Renderiza texto com SDL_ttf numa posição de tela
    void drawText(const std::string& text, int x, int y,
                  SDL_Color color, TTF_Font* font = nullptr);

    // Desenha um retângulo semi-transparente (overlay)
    void drawRect(int x, int y, int w, int h, SDL_Color color);
};
```

---

### Task 3.2 — `SDLRenderer.cpp` (renderização do mapa)

**Arquivo:** `src/ui/SDLRenderer.cpp` (criar)

Implementar:

1. **Construtor:** inicializar `TileAtlas`, `TTF_OpenFont` para HUD
2. **`renderMapa()`:** loop por todos os tiles do mapa aplicando fog of war:

```cpp
void SDLRenderer::renderMapa(...) {
    for (int y = 0; y < MAP_ROWS; ++y) {
        for (int x = 0; x < MAP_COLS; ++x) {
            int destX = x * TILE_W;
            int destY = y * TILE_H;

            bool explorado = map.isExplored(x, y);
            bool visivel   = map.isVisible(x, y);   // se Map tiver esse método

            if (!explorado) {
                // Tile preto — não desenha nada (fundo já é preto)
                continue;
            }

            // Desenha terreno
            drawTerrain(map.getTile(x, y), x, y, visivel, explorado);

            if (!visivel) continue;  // inimigos/itens só aparecem se visíveis

            // Inimigo na posição?
            for (const auto& enemy : enemies) {
                if (enemy->isAlive() && enemy->getX() == x && enemy->getY() == y) {
                    atlas_.draw(renderer_, tileIDForEnemy(*enemy), destX, destY);
                }
            }

            // Item na posição?
            for (const auto& item : items) {
                if (item->getX() == x && item->getY() == y) {
                    atlas_.draw(renderer_, tileIDForItem(*item), destX, destY);
                }
            }
        }
    }

    // Player sempre desenhado
    atlas_.draw(renderer_, TileID::Player,
                player.getX() * TILE_W,
                player.getY() * TILE_H);
}
```

> **Nota sobre `isVisible()`:** o `Map` atual tem `isExplored()` mas pode não ter `isVisible()` separado. Verificar e adicionar se necessário, ou usar a lógica de visibilidade do campo de visão existente.

3. **`tileIDForEnemy()`:** switch pelo `getTipoNome()` do inimigo
4. **`tileIDForItem()`:** switch pelo `getTipo()` do item

- [ ] Adicionar `SDLRenderer.cpp` ao CMakeLists.txt.
- [ ] Buildar.
- [ ] Testar: instanciar `SDLRenderer` em `main.cpp` e chamar `render()` com dados fictícios.

**Critério de aceite do Chunk 3:** mapa renderizado com sprites, fog of war funcionando.

---

## Chunk 4: HUD com SDL_ttf

### Task 4.1 — Fonte TTF

- [ ] Baixar [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P) (licença OFL) ou outra fonte pixel art.
- [ ] Salvar em `assets/fonts/PressStart2P.ttf`.
- [ ] Adicionar ao `CMakeLists.txt` a cópia dos assets para o diretório de build:

```cmake
add_custom_command(TARGET roguelike POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:roguelike>/assets
)
```

---

### Task 4.2 — `renderHUD()`

Área Y = 528 a Y = 648. Desenhar:

- Linha 1 (Y=535): HP, ATK, DEF, Andar, Level, XP
- Linha 2 (Y=558): Efeitos ativos com cor correspondente
- Linhas 3-5 (Y=578+): últimas 3 mensagens do log

```cpp
void SDLRenderer::renderHUD(const Player& player,
                             const std::deque<std::string>& messageLog,
                             int andarAtual) {
    // Separador
    drawRect(0, HUD_Y, WIN_W, 2, {60, 60, 60, 255});

    SDL_Color branco = {255, 255, 255, 255};
    SDL_Color cinza  = {180, 180, 180, 255};

    std::string linha1 = "HP:" + std::to_string(player.getHp()) + "/" +
                         std::to_string(player.getMaxHp()) +
                         "  ATK:" + std::to_string(player.getAttack()) +
                         "  DEF:" + std::to_string(player.getDefense()) +
                         "  Andar:" + std::to_string(andarAtual) +
                         "  Lv:" + std::to_string(player.getLevel()) +
                         "  XP:" + std::to_string(player.getXP()) + "/" +
                         std::to_string(player.getXPProxLevel());

    drawText(linha1, 8, HUD_Y + 8, branco);

    // Efeitos com cores
    // ...

    // Log de mensagens
    int logY = HUD_Y + 55;
    for (const auto& msg : messageLog) {
        drawText(msg, 8, logY, cinza);
        logY += 20;
    }
}
```

---

### Task 4.3 — `renderInventario()` (painel SDL)

Painel semi-transparente centralizado sobre o mapa:
- `drawRect(200, 100, 560, 350, {0, 0, 0, 200})` — fundo escuro
- Texto dos slots com `drawText()`

- [ ] Buildar + testar visualmente.

**Critério de aceite do Chunk 4:** HUD legível abaixo do mapa; inventário abre como painel.

---

## Chunk 5: Event loop SDL + integração com Game

### Task 5.1 — `Game.hpp`: substituir `Renderer` por `SDLRenderer`

```cpp
// Remover:
#include "ui/Renderer.hpp"
// Adicionar:
#include "ui/SDLRenderer.hpp"
#include <SDL2/SDL.h>

// Remover:
Renderer renderer_;
// Adicionar:
SDLRenderer* renderer_;   // ponteiro não-owning — criado em main.cpp

// Adicionar ao construtor (ou criar construtor alternativo):
Game(SDL_Renderer* sdlRenderer,
     const std::string& spritePath,
     const std::string& fontPath);
```

---

### Task 5.2 — `Game.cpp`: loop com SDL_PollEvent

```cpp
void Game::run() {
    while (isRunning_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { isRunning_ = false; break; }
            if (event.type == SDL_KEYDOWN) processInput(event.key.keysym.sym);
        }
        render();
        SDL_RenderPresent(sdlRenderer_);  // apresenta o frame
        if (estado_ == EstadoJogo::Jogando)
            update();
        SDL_Delay(16);  // ~60 fps
    }
    std::cout << "\nAte a proxima aventura!\n";
}
```

`processInput()` muda de `char key` para `SDL_Keycode key`:

```cpp
void Game::processInput(SDL_Keycode key) {
    switch (estado_) {
        case EstadoJogo::Menu:
            if (key == SDLK_ESCAPE) { isRunning_ = false; return; }
            if (key == SDLK_n) { /* nova partida */ }
            if (key == SDLK_c) { /* carregar */     }
            return;

        case EstadoJogo::Jogando:
            if (key == SDLK_ESCAPE) { isRunning_ = false; return; }
            if (key == SDLK_s && SDL_GetModState() & KMOD_SHIFT) { salvar(); return; }
            if (key == SDLK_l && SDL_GetModState() & KMOD_SHIFT) { carregar(); return; }
            if (key == SDLK_h) { estado_ = EstadoJogo::Historico; return; }
            switch (key) {
                case SDLK_w: player_.move(0, -1, map_, enemies_); break;
                case SDLK_s: player_.move(0,  1, map_, enemies_); break;
                case SDLK_a: player_.move(-1, 0, map_, enemies_); break;
                case SDLK_d: player_.move(1,  0, map_, enemies_); break;
                // ...
            }
            return;
        // ...
    }
}
```

> **Atenção para os testes:** `GameTeste` sobrescreve `render()` como noop. O `SDL_Renderer*` passado ao `SDLRenderer` pode ser `nullptr` em contexto de teste — o `SDLRenderer` deve verificar isso antes de qualquer chamada SDL. Alternativa: `GameTeste` passa `nullptr` e `SDLRenderer` só renderiza se o ponteiro não for nulo.

---

### Task 5.3 — `main.cpp`: instanciar `Game` com SDL

```cpp
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window*   window   = SDL_CreateWindow("Roguelike", ..., 960, 648, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Carregar background do menu
    SDL_Texture* bgMenu = IMG_LoadTexture(renderer,
        "assets/sprites/Backgrounds/oryx_roguelike_b_dungeon.png");

    Game game(renderer,
              "assets/sprites/V1/oryx_roguelike_16x24.png",
              "assets/fonts/PressStart2P.ttf");
    game.run();

    SDL_DestroyTexture(bgMenu);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
```

> Mover os sprites para `assets/sprites/` e atualizar o CMakeLists.txt para copiar `assets/` para o diretório de build.

---

### Task 5.4 — Remover arquivos antigos

- [ ] Deletar `src/ui/Renderer.hpp` e `src/ui/Renderer.cpp`
- [ ] Deletar `src/ui/ConsoleFacade.hpp` e `src/ui/ConsoleFacade.cpp`
- [ ] Remover essas entradas do `CMakeLists.txt`
- [ ] Buildar — sem erros.
- [ ] Rodar testes (`roguelike_tests.exe`) — 86 testes devem continuar passando.

**Critério de aceite do Chunk 5:** jogo completo jogável na janela SDL com todas as teclas funcionando.

---

## Chunk 6: Telas especiais

### Task 6.1 — `renderMenu()`

```cpp
void SDLRenderer::renderMenu(bool temSave, SDL_Texture* background) {
    // Fundo: oryx_roguelike_b_dungeon.png esticado para 960x648
    SDL_Rect fullscreen = {0, 0, WIN_W, WIN_H};
    SDL_RenderCopy(renderer_, background, nullptr, &fullscreen);

    // Painel central semi-transparente
    drawRect(280, 200, 400, 200, {0, 0, 0, 180});

    SDL_Color branco  = {255, 255, 255, 255};
    SDL_Color amarelo = {255, 220, 0,   255};
    SDL_Color cinza   = {120, 120, 120, 255};

    drawText("ROGUELIKE v0.10", 330, 220, amarelo);
    drawText("[N] Nova Partida",  310, 270, branco);
    drawText(temSave ? "[C] Carregar Jogo" : "[C] Sem save",
             310, 300, temSave ? branco : cinza);
    drawText("[ESC] Sair",       310, 330, branco);
}
```

---

### Task 6.2 — `renderTelaDerrota()` e `renderHistorico()`

Seguir o mesmo padrão: overlay semi-transparente + texto com `drawText()`.

- [ ] Buildar + testar manualmente todos os fluxos.
- [ ] Rodar testes — 86+ passando.

**Critério de aceite do Chunk 6:** todas as telas renderizadas corretamente com SDL.

---

## Resumo de arquivos

| Arquivo | Ação |
|---|---|
| `CMakeLists.txt` | Modificar — SDL2/image/ttf via FetchContent; copiar assets |
| `main.cpp` | Modificar — init SDL, criar janela/renderer, instanciar Game |
| `src/ui/TileID.hpp` | **Criar** |
| `src/ui/TileAtlas.hpp` | **Criar** |
| `src/ui/TileAtlas.cpp` | **Criar** |
| `src/ui/SDLRenderer.hpp` | **Criar** |
| `src/ui/SDLRenderer.cpp` | **Criar** |
| `src/ui/Renderer.hpp/.cpp` | **Remover** (Chunk 5) |
| `src/ui/ConsoleFacade.hpp/.cpp` | **Remover** (Chunk 5) |
| `src/core/Game.hpp` | Modificar — `SDLRenderer*`, `processInput(SDL_Keycode)` |
| `src/core/Game.cpp` | Modificar — `run()` com SDL_PollEvent |
| `assets/fonts/PressStart2P.ttf` | **Criar** — baixar e incluir |
| `tests/test_game.cpp` | Verificar compatibilidade com `nullptr` renderer |

---

## Ordem de execução recomendada

```
Chunk 1  →  janela SDL abrindo
Chunk 2  →  atlas carregando, coordenadas confirmadas
Chunk 3  →  mapa + entidades visíveis com sprites
Chunk 4  →  HUD com texto SDL_ttf
Chunk 5  →  Game jogável via SDL (remover console)
Chunk 6  →  telas de menu/morte/histórico
```

Cada chunk termina com **build limpo + jogo testável** antes de avançar.
