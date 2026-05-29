# Phase 10 — SDL2 + Sprites — Spec

**Data:** 2026-05-21  
**Atualizado:** 2026-05-26  
**Status:** Em andamento — Chunks 1–5 concluídos, coordenadas dos tiles em calibração

---

## Objetivo

Substituir o renderer de console (texto/caracteres) por uma janela gráfica SDL2 que renderiza sprites do pack **Oryx Roguelike Ultimate** (16×24 px por tile). O resultado visual deve representar o player, inimigos, itens e terreno com sprites reconhecíveis em vez de letras.

Toda a lógica de jogo (`Game`, `Map`, `Player`, `Enemy`, `Item`, efeitos, save/load) permanece **intacta**. A mudança fica contida em:
- `src/ui/` — arquivos SDL
- `src/core/Game.cpp` — loop SDL + input por keycode
- `main.cpp` — inicialização SDL
- `CMakeLists.txt` — dependências e cópia de assets

---

## Decisões de design

| Decisão | Escolha | Motivo |
|---|---|---|
| Renderer antigo | Substituído completamente | Sem duplicação de código |
| Spritesheets | Múltiplos PNGs por categoria (ver tabela abaixo) | Fundo transparente; sprites isolados por assunto |
| Tamanho do tile | 16×24 px | Dimensão nativa do pack |
| Fonte para HUD | `assets/fonts/PressStart2P.ttf` | Pixel art, licença OFL |
| Fog of war | `SDL_SetTextureColorMod` para escurecer tiles visitados | Clássico roguelike — **pendente de implementar** |
| SDL2 no CMake | FetchContent | Mesmo padrão do projeto (nlohmann/json) |
| Input | `SDL_PollEvent` com `SDL_Keycode` como `int` | Compatível com testes sem SDL |
| Multi-sheet | `SpriteSheet` enum + `TileInfo` struct | Cada categoria de tile usa seu próprio PNG |
| Responsabilidade de cor | Pixel do PNG (cor base) + `SDL_SetTextureColorMod` (modulação) | Separação: arte vs. estado do jogo |

---

## Arquitetura de sprites (implementada)

```
TileID (enum)
  └─► TileAtlas::getInfo(TileID) → TileInfo { SpriteSheet, SDL_Rect }
            │
            ├── SpriteSheet enum (qual arquivo PNG)
            └── SDL_Rect (posição no grid do PNG)

SDLRenderer
  ├── sheets_: map<SpriteSheet, SDL_Texture*>   ← uma textura por arquivo
  └── desenharTile(TileID, pixelX, pixelY)
        ├── consulta TileAtlas::getInfo()
        ├── busca sheets_[info.sheet]
        ├── SDL_SetTextureColorMod(tex, r, g, b)   ← modulação de brilho
        └── SDL_RenderCopy(renderer_, tex, &src, &dst)
```

### Onde fica a decisão de cor

| Aspecto | Onde | Como |
|---|---|---|
| Cor do sprite | Pixels do PNG | `SDL_RenderCopy` renderiza as cores do arquivo como estão |
| Brilho / fog of war | `desenharTile` em `SDLRenderer.cpp` | `SDL_SetTextureColorMod(tex, r, g, b)` antes do `RenderCopy` |
| Fallback (sem textura) | `desenharTile`, bloco `if (!tex)` | `SDL_SetRenderDrawColor` + `SDL_RenderFillRect` com cor fixa por `TileID` |

---

## Spritesheets utilizados

| Enum `SpriteSheet` | Arquivo | Conteúdo |
|---|---|---|
| `Monsters` | `assets/sprites/Monsters.png` | Player, Goblin, Orc, Troll, todos os monstros |
| `Items` | `assets/sprites/Items.png` | Poções, espadas, armaduras, amuletos |
| `Terrain` | `assets/sprites/Terrain.png` | Paredes, chão, escadas |
| `TerrainObjects` | `assets/sprites/Terrain_Objects.png` | Objetos decorativos do terreno |

Outros arquivos disponíveis (fora do escopo atual):
- `Interface.png`, `Interface_Icons.png` — UI futura
- `FX_Blood.png`, `FX_General.png`, `FX_Projectiles.png` — efeitos visuais
- `Avatar.png`, `Avatar_Equipment.png` — retrato do jogador
- `Backgrounds/` — 21 fundos temáticos (dungeon, caverna, floresta, etc.)

---

## Estrutura de arquivos (atual)

```
src/ui/
  SpriteSheet.hpp         ← enum class { Monsters, Items, Terrain, TerrainObjects }
  TileID.hpp              ← enum class com todos os tiles usados no jogo
  TileAtlas.hpp           ← TileInfo struct + getInfo(TileID) → TileInfo
  TileAtlas.cpp           ← tabela TileID → (SpriteSheet, col, row) — TODO: confirmar coords
  SDLRenderer.hpp         ← map<SpriteSheet, SDL_Texture*>; construtor recebe spriteDir
  SDLRenderer.cpp         ← load múltiplo, desenharTile, fog of war pendente

assets/
  fonts/
    PressStart2P.ttf

src/sprites/
  Monsters.png
  Items.png
  Terrain.png
  Terrain_Objects.png
  Backgrounds/
    oryx_roguelike_b_dungeon.png
  V1/
    preview_creatures.png   ← usar para confirmar coords de Monsters.png
    preview_dungeons.png    ← usar para confirmar coords de Terrain.png
    preview_items.png       ← usar para confirmar coords de Items.png
```

---

## TileIDs implementados

| TileID | SpriteSheet | Arquivo preview para calibrar |
|---|---|---|
| `PLAYER` | `Monsters` | `preview_creatures.png` ou `Avatar.png` |
| `GOBLIN` | `Monsters` | `preview_creatures.png` |
| `ORC` | `Monsters` | `preview_creatures.png` |
| `TROLL` | `Monsters` | `preview_creatures.png` |
| `PAREDE` | `Terrain` | `preview_dungeons.png` |
| `CHAO` | `Terrain` | `preview_dungeons.png` |
| `ESCADA` | `Terrain` | `preview_dungeons.png` |
| `ITEM_POCAO` | `Items` | `preview_items.png` |
| `ITEM_ESPADA` | `Items` | `preview_items.png` |
| `ITEM_ARMADURA` | `Items` | `preview_items.png` |
| `ITEM_AMULETO` | `Items` | `preview_items.png` |

> Todos os mapeamentos em `TileAtlas.cpp` têm `// TODO: confirmar` — as coordenadas reais precisam ser confirmadas abrindo os PNGs de preview.

---

## Dimensões da janela

```
Largura:  60 tiles × 16 px = 960 px
Altura:   22 tiles × 24 px = 528 px  (mapa)
        + 120 px                      (HUD)
        = 648 px total
```

---

## Dependências

| Biblioteca | Versão | Para quê |
|---|---|---|
| SDL2 | 2.30.3 | Janela, renderer, input, loop |
| SDL2_image | 2.8.2 | Carregar PNG via `IMG_LoadTexture` |
| SDL2_ttf | 2.22.0 | Renderizar texto no HUD |

Todas via `FetchContent` no `CMakeLists.txt`.

---

## Comportamento de fog of war

| Estado do tile | Visual | Implementado? |
|---|---|---|
| Nunca visto | Preto sólido (não desenha) | ✅ |
| Visitado (fora da visão atual) | Tile de terreno com `colorMod(80, 80, 80)` | ⏳ pendente |
| Visível agora | Tile de terreno na cor normal `(255, 255, 255)` | ⏳ pendente (sem mod = 255 atualmente) |
| Entidade em tile visível | Tile do terreno + entidade por cima | ✅ |
| Entidade em tile visitado | Não desenhada | ✅ |

---

## Interface do HUD

Área de 120px abaixo do mapa (Y = 528 a Y = 648):

```
Linha 1: HP: 45/50  ATK: 7  DEF: 3  Andar: 2  Lv: 3  XP: 30
Linhas 2-4: Log de mensagens (últimas 3)
```

Quando `inventarioAberto_ == true`: painel a implementar no Chunk 6.

---

## Telas especiais (estado atual)

### Menu ✅
- Fundo preto
- Texto: ROGUELIKE, [N] Nova Partida, [C] Continuar, [ESC] Sair

### Tela de derrota ✅
- Fundo preto + estatísticas: andar, nível, XP, inimigos
- `[H] Historico  [M] Menu`

### Histórico ✅
- Lista de mensagens da partida (mais recentes primeiro)
- `[H / ESC] Fechar`

---

## Construtor do SDLRenderer

```cpp
SDLRenderer(SDL_Renderer* renderer,
            const std::string& spriteDir,   // ex: "assets/sprites/"
            const std::string& fontPath);   // ex: "assets/fonts/PressStart2P.ttf"
```

O construtor monta os caminhos internamente:
- `spriteDir + "Monsters.png"` → `SpriteSheet::Monsters`
- `spriteDir + "Items.png"` → `SpriteSheet::Items`
- etc.

---

## Notas sobre os testes

- `SDLRenderer` NÃO é instanciado nos testes (excluído do build de testes no CMakeLists)
- `Game` usa `#ifdef ROGUELIKE_SDL` para guardar toda dependência SDL — macro definida apenas para o target `roguelike`
- `GameTeste` passa `nullptr` como `SDL_Renderer*` — `SDLRenderer` e `Game` verificam antes de qualquer chamada SDL

---

## Fora do escopo desta phase

- Animações de sprites (spritesheet com frames)
- Sons / música
- Mouse input
- Redimensionamento de janela
- Suporte a Linux/Mac (foco em Windows MSVC)
- Fundo de imagem no menu (futuro: `Backgrounds/oryx_roguelike_b_dungeon.png`)
