# Phase 4 — XP/Níveis + Fog of War — Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Adicionar progressão do jogador (XP, níveis, log de mensagens) e Fog of War simplificado (salas exploradas ficam visíveis para sempre).

**Architecture:** O sistema de XP vive em `Player` (campos + `addXP()`), com `Enemy` expondo `getXPReward()` como método virtual puro. O Fog of War vive em `Map` (grid `explored_[][]` + `updateVisibility()`), com `Renderer` consultando `isExplored()` para decidir o que desenhar. O log de mensagens é um `std::deque<std::string>` em `Game`, passado ao `Renderer` na assinatura de `render()`.

**Tech Stack:** C++17, MSVC 2022, CMake, console Windows/Linux.

**Spec:** `docs/superpowers/specs/2026-03-23-phase4-xp-fogofwar-design.md`

---

## Chunk 1: XP, Níveis e Log de Mensagens

### Task 1: Adicionar `getXPReward()` à hierarquia de `Enemy`

**Conceito ensinado:** método virtual puro — força toda subclasse a implementar o comportamento, sem permitir instâncias de `Enemy` sem a implementação.

**Arquivos:**
- Modificar: `src/entities/Enemy.hpp`
- Modificar: `src/entities/Goblin.hpp` e `src/entities/Goblin.cpp`
- Modificar: `src/entities/Troll.hpp` e `src/entities/Troll.cpp`

- [ ] **Passo 1: Adicionar declaração pura em `Enemy.hpp`**

Abra `src/entities/Enemy.hpp`. Adicione a declaração após `bool isAlive() const;`:

```cpp
// Retorna a recompensa de XP ao matar este inimigo
virtual int getXPReward() const = 0;
```

- [ ] **Passo 2: Implementar em `Goblin`**

Em `src/entities/Goblin.hpp`, adicione na seção `public`:
```cpp
int getXPReward() const override;
```

Em `src/entities/Goblin.cpp`, adicione a implementação:
```cpp
int Goblin::getXPReward() const { return 10; }
```

- [ ] **Passo 3: Implementar em `Troll`**

Em `src/entities/Troll.hpp`, adicione na seção `public`:
```cpp
int getXPReward() const override;
```

Em `src/entities/Troll.cpp`, adicione a implementação:
```cpp
int Troll::getXPReward() const { return 25; }
```

- [ ] **Passo 4: Compilar e verificar**

No VS Code: `Ctrl+Shift+P` → `CMake: Build` (ou `F7`).

Resultado esperado: **Build succeeded** sem erros. Se o compilador reclamar que `Goblin` ou `Troll` não implementam `getXPReward`, verifique se os `override` foram adicionados corretamente.

- [ ] **Passo 5: Commit**

```
git add src/entities/Enemy.hpp src/entities/Goblin.hpp src/entities/Goblin.cpp src/entities/Troll.hpp src/entities/Troll.cpp
git commit -m "feat: add getXPReward() pure virtual to Enemy hierarchy"
```

---

### Task 2: Adicionar XP e nível ao `Player`

**Conceito ensinado:** encapsulamento — lógica de progressão dentro da própria classe que a possui, com interface pública mínima.

**Arquivos:**
- Modificar: `src/entities/Player.hpp`
- Modificar: `src/entities/Player.cpp`

- [ ] **Passo 1: Adicionar campos e declarações em `Player.hpp`**

Abra `src/entities/Player.hpp`. Adicione `#include <string>` no topo (após os includes existentes).

Na seção `public`, adicione após `bool isAlive() const;`:
```cpp
// Adiciona XP. Faz level-up automaticamente se atingir o limite.
// Retorna string de notificação ("Nivel 2! +2 HP, +1 ATK") ou "" se não subiu.
std::string addXP(int amount);

int getXP()       const;
int getLevel()    const;
int getXPToNext() const;
```

Na seção `private`, adicione após `int defense_;`:
```cpp
int xp_       = 0;
int level_    = 1;
int xpToNext_ = 20;
```

- [ ] **Passo 2: Implementar `addXP()` em `Player.cpp`**

Abra `src/entities/Player.cpp`. Adicione `#include <string>` se não estiver presente.

Adicione ao final do arquivo:
```cpp
std::string Player::addXP(int amount) {
    xp_ += amount;
    std::string notification = "";
    while (xp_ >= xpToNext_) {
        xp_       -= xpToNext_;
        xpToNext_  = static_cast<int>(xpToNext_ * 1.5);
        level_++;
        maxHp_  += 2;
        hp_     += 2;
        attack_ += 1;
        notification = "Nivel " + std::to_string(level_) + "! +2 HP, +1 ATK";
    }
    return notification;
}

int Player::getXP()       const { return xp_; }
int Player::getLevel()    const { return level_; }
int Player::getXPToNext() const { return xpToNext_; }
```

- [ ] **Passo 3: Compilar e verificar**

`CMake: Build`. Resultado esperado: **Build succeeded**.

Dica de debug: se o compilador reclamar de `maxHp_` ou `attack_` inacessíveis, verifique se esses campos estão em `private` em `Player.hpp` — eles devem estar, e `addXP()` é membro de `Player`, então tem acesso.

- [ ] **Passo 4: Commit**

```
git add src/entities/Player.hpp src/entities/Player.cpp
git commit -m "feat: add XP and level system to Player"
```

---

### Task 3: Log de mensagens em `Game`

**Conceito ensinado:** `std::deque` — fila dupla que permite inserção no final e remoção no início com custo O(1), ideal para um log de tamanho fixo.

**Arquivos:**
- Modificar: `src/core/Game.hpp`
- Modificar: `src/core/Game.cpp`

- [ ] **Passo 1: Adicionar `messageLog_` e `pushMessage()` em `Game.hpp`**

Abra `src/core/Game.hpp`. Adicione no topo, após os includes existentes:
```cpp
#include <deque>
#include <string>
```

Na seção `private`, adicione após `std::vector<std::unique_ptr<Enemy>> enemies_;`:
```cpp
std::deque<std::string> messageLog_;  // até 3 mensagens visíveis
```

Na seção `private`, adicione após `void render();`:
```cpp
void pushMessage(const std::string& msg);
```

- [ ] **Passo 2: Implementar `pushMessage()` em `Game.cpp`**

Abra `src/core/Game.cpp`. Adicione ao final:
```cpp
void Game::pushMessage(const std::string& msg) {
    messageLog_.push_back(msg);
    if (messageLog_.size() > 3)
        messageLog_.pop_front();
}
```

- [ ] **Passo 3: Premiar XP na morte do inimigo**

Em `Game.cpp`, localize o método `Game::update()`. Ele começa assim:

```cpp
void Game::update() {
    player_.update();

    for (auto& enemy : enemies_) {
        if(enemy->isAlive()){
            enemy->update(map_, player_);
        }
    }
    enemies_.erase(   // ← ANTES deste erase
```

Adicione o loop de XP **antes** do `enemies_.erase(...)`:

```cpp
    // Premia XP pelos inimigos mortos neste turno
    for (auto& enemy : enemies_) {
        if (!enemy->isAlive()) {
            std::string msg = player_.addXP(enemy->getXPReward());
            if (!msg.empty()) pushMessage(msg);
        }
    }
```

O `update()` completo deve ficar:
```cpp
void Game::update() {
    player_.update();

    for (auto& enemy : enemies_) {
        if (enemy->isAlive()) {
            enemy->update(map_, player_);
        }
    }

    // Premia XP pelos inimigos mortos neste turno
    for (auto& enemy : enemies_) {
        if (!enemy->isAlive()) {
            std::string msg = player_.addXP(enemy->getXPReward());
            if (!msg.empty()) pushMessage(msg);
        }
    }

    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<Enemy>& e) {
                return !e->isAlive();
            }),
        enemies_.end()
    );

    if (!player_.isAlive()) {
        isRunning_ = false;
    }
}
```

- [ ] **Passo 4: Compilar e verificar**

`CMake: Build`. Resultado esperado: **Build succeeded**.

- [ ] **Passo 5: Commit**

```
git add src/core/Game.hpp src/core/Game.cpp
git commit -m "feat: add message log and XP reward on enemy death"
```

---

### Task 4: Atualizar `Renderer` com HUD expandido e log de mensagens

**Conceito ensinado:** separação de responsabilidades — `Renderer` só exibe, nunca calcula estado de jogo.

**Arquivos:**
- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`
- Modificar: `src/core/Game.cpp` (chamada de `render()`)

- [ ] **Passo 1: Atualizar assinatura de `render()` em `Renderer.hpp`**

Abra `src/ui/Renderer.hpp`. Adicione o include:
```cpp
#include <deque>
#include <string>
```

Troque a assinatura de `render()`:
```cpp
// DE:
void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies);

// PARA:
void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies,
            const std::deque<std::string>& messageLog);
```

Também atualize a assinatura de `renderHUD()` — ela receberá o player mas o log será tratado separadamente:
```cpp
// renderHUD permanece igual:
void renderHUD(const Player& player);
// Adicione método novo para o log:
void renderMessageLog(const std::deque<std::string>& messageLog);
```

- [ ] **Passo 2: Atualizar `Renderer.cpp`**

Abra `src/ui/Renderer.cpp`.

**2a.** Atualize a assinatura de `render()`:
```cpp
void Renderer::render(const Map& map, const Player& player,
                      const std::vector<std::unique_ptr<Enemy>>& enemies,
                      const std::deque<std::string>& messageLog) {
```

**2b.** Troque a chamada de `renderHUD` no final de `render()` — adicione o log depois:
```cpp
    renderHUD(player);
    renderMessageLog(messageLog);
}
```

**2c.** Atualize `renderHUD()` para mostrar nível e XP:
```cpp
void Renderer::renderHUD(const Player& player) {
    std::cout << std::string(40, '-') << '\n';
    std::cout << " HP: "  << player.getHp()    << "/" << player.getMaxHp();
    std::cout << "   ATK: " << player.getAttack();
    std::cout << "   DEF: " << player.getDefense();
    std::cout << "   LVL: " << player.getLevel();
    std::cout << "   XP: "  << player.getXP() << "/" << player.getXPToNext();
    std::cout << '\n';
    std::cout << std::string(40, '-') << '\n';
    std::cout << " [W/A/S/D] Mover   [Q] Sair\n";
}
```

**2d.** Adicione a implementação de `renderMessageLog()` ao final do arquivo:
```cpp
void Renderer::renderMessageLog(const std::deque<std::string>& messageLog) {
    for (const auto& msg : messageLog) {
        std::cout << "> " << msg << '\n';
    }
}
```

- [ ] **Passo 3: Atualizar a chamada em `Game::render()`**

Abra `src/core/Game.cpp`. Localize `Game::render()`:
```cpp
void Game::render() {
    renderer_.render(map_, player_, enemies_);
}
```

Troque para:
```cpp
void Game::render() {
    renderer_.render(map_, player_, enemies_, messageLog_);
}
```

- [ ] **Passo 4: Compilar e testar manualmente**

`CMake: Build`. Resultado esperado: **Build succeeded**.

Execute o jogo (`build/Debug/roguelike.exe`). Verifique:
- HUD mostra `LVL: 1  XP: 0/20`
- Ao matar um Goblin: XP sobe 10, aparece no HUD
- Ao matar inimigos suficientes: mensagem `> Nivel 2! +2 HP, +1 ATK` aparece abaixo do HUD
- HP e ATK aumentam no HUD após level-up

- [ ] **Passo 5: Commit**

```
git add src/ui/Renderer.hpp src/ui/Renderer.cpp src/core/Game.cpp
git commit -m "feat: update HUD with level/XP and add message log display"
```

---

## Chunk 2: Fog of War

### Task 5: Adicionar `explored_[][]` e `updateVisibility()` ao `Map`

**Conceito ensinado:** grid 2D com `std::vector<std::vector<bool>>` — estrutura de dados para estado por tile; encapsulamento da lógica de visibilidade dentro do dono do estado do mundo.

**Arquivos:**
- Modificar: `src/map/Map.hpp`
- Modificar: `src/map/Map.cpp`

- [ ] **Passo 1: Adicionar campos e declarações em `Map.hpp`**

Abra `src/map/Map.hpp`. Na seção `public`, adicione após `const std::vector<Room>& getRooms() const;`:

```cpp
// Fog of War — visibilidade
bool isExplored(int x, int y) const;
void updateVisibility(int px, int py);  // px, py = posição do jogador
```

Na seção `private`, adicione após `std::vector<Room> rooms_;`:
```cpp
std::vector<std::vector<bool>> explored_;  // true = tile já foi visto pelo jogador
```

- [ ] **Passo 2: Inicializar `explored_` no construtor de `Map`**

Abra `src/map/Map.cpp`. No construtor `Map::Map(int width, int height)`, adicione após a inicialização de `tiles_`:

```cpp
explored_.assign(height, std::vector<bool>(width, false));
```

O construtor completo ficará:
```cpp
Map::Map(int width, int height)
    : width_(width), height_(height) {
    tiles_.assign(height, std::vector<char>(width, '.'));
    explored_.assign(height, std::vector<bool>(width, false));
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1) {
                tiles_[y][x] = '#';
            }
        }
    }
}
```

Atenção: `generate()` chama `fill('#')` mas **não** reseta `explored_`. Isso é correto — ao gerar um novo mapa, você deve também resetar `explored_`. Adicione no início de `generate()`, após `rooms_.clear();`:

```cpp
// Reseta a visibilidade junto com o mapa
for (auto& row : explored_)
    std::fill(row.begin(), row.end(), false);
```

- [ ] **Passo 3: Implementar `isExplored()` em `Map.cpp`**

Adicione ao final de `Map.cpp`:
```cpp
bool Map::isExplored(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    return explored_[y][x];
}
```

- [ ] **Passo 4: Implementar `updateVisibility()` em `Map.cpp`**

Adicione ao final de `Map.cpp`:

```cpp
void Map::updateVisibility(int px, int py) {
    // Passo 1: encontra a sala que contém o jogador
    const Room* currentRoom = nullptr;
    for (const Room& room : rooms_) {
        if (room.contains(px, py)) {
            currentRoom = &room;
            break;
        }
    }

    // Passo 2: se estiver numa sala, revela toda a sala + 1 tile de margem (paredes)
    if (currentRoom != nullptr) {
        int x0 = std::max(0, currentRoom->x - 1);
        int y0 = std::max(0, currentRoom->y - 1);
        int x1 = std::min(width_  - 1, currentRoom->x + currentRoom->width);
        int y1 = std::min(height_ - 1, currentRoom->y + currentRoom->height);

        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x)
                explored_[y][x] = true;
    }

    // Passo 3: revela corredor — tiles walkable num raio de 1 ao redor do jogador
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = px + dx, ny = py + dy;
            if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
                if (isWalkable(nx, ny))
                    explored_[ny][nx] = true;
            }
        }
    }
}
```

- [ ] **Passo 5: Compilar e verificar**

`CMake: Build`. Resultado esperado: **Build succeeded**.

- [ ] **Passo 6: Commit**

```
git add src/map/Map.hpp src/map/Map.cpp
git commit -m "feat: add explored grid and updateVisibility to Map"
```

---

### Task 6: Integrar `updateVisibility()` no `Game` e aplicar FOW no `Renderer`

**Conceito ensinado:** orquestração no `Game` (chama `updateVisibility` a cada turno), renderização condicional no `Renderer` (consulta `isExplored` antes de desenhar).

**Arquivos:**
- Modificar: `src/core/Game.cpp`
- Modificar: `src/ui/Renderer.cpp`

- [ ] **Passo 1: Chamar `updateVisibility()` em `Game::update()`**

Abra `src/core/Game.cpp`. No método `Game::update()`, adicione a chamada logo após `player_.update();`:

```cpp
void Game::update() {
    player_.update();
    map_.updateVisibility(player_.getX(), player_.getY());  // ← adicione aqui

    // ... resto do método permanece igual
```

Também chame no construtor `Game::Game()`, após posicionar o jogador, para revelar a sala inicial imediatamente:

```cpp
    if (!map_.getRooms().empty()) {
        Point start = map_.getRooms().front().center();
        player_ = Player(start.x, start.y);
        map_.updateVisibility(player_.getX(), player_.getY());  // ← adicione aqui

        // ... loop de spawn de inimigos permanece igual
```

- [ ] **Passo 2: Aplicar FOW no `Renderer::render()`**

Abra `src/ui/Renderer.cpp`. Localize o loop principal de renderização em `render()`:

```cpp
for (int y = 0; y < map.getHeight(); ++y) {
    for (int x = 0; x < map.getWidth(); ++x) {
        if (x == player.getX() && y == player.getY()) {
            std::cout << player.getSymbol();
            continue;
        }
        // ... desenha inimigos e tile
    }
    std::cout << '\n';
}
```

Troque pelo loop com FOW:

```cpp
for (int y = 0; y < map.getHeight(); ++y) {
    for (int x = 0; x < map.getWidth(); ++x) {

        // Tile não explorado: espaço em branco
        if (!map.isExplored(x, y)) {
            std::cout << ' ';
            continue;
        }

        // Jogador tem prioridade máxima
        if (x == player.getX() && y == player.getY()) {
            std::cout << player.getSymbol();
            continue;
        }

        // Inimigos só aparecem se estiverem perto do jogador (mesma área explorada visível)
        bool hadEnemy = false;
        for (const auto& enemy : enemies) {
            if (enemy->isAlive() && enemy->getX() == x && enemy->getY() == y) {
                // Chebyshev: inimigo visível se distância <= 10 do jogador
                int dx = std::abs(enemy->getX() - player.getX());
                int dy = std::abs(enemy->getY() - player.getY());
                if (std::max(dx, dy) <= 10) {
                    std::cout << enemy->getSymbol();
                    hadEnemy = true;
                    break;
                }
            }
        }
        if (!hadEnemy) {
            std::cout << map.getTile(x, y);
        }
    }
    std::cout << '\n';
}
```

Adicione `#include <cmath>` ou `#include <algorithm>` no topo de `Renderer.cpp` se `std::abs` e `std::max` não estiverem disponíveis (provavelmente já estão via `<string>`; se o compilador reclamar, adicione `#include <algorithm>`).

- [ ] **Passo 3: Compilar e testar manualmente**

`CMake: Build`. Resultado esperado: **Build succeeded**.

Execute `build/Debug/roguelike.exe`. Verifique:
- Ao iniciar, apenas a primeira sala é visível; o resto do mapa é espaço em branco
- Ao mover para outra sala, ela se revela permanentemente
- Tiles já visitados ficam visíveis mesmo ao sair da sala
- Inimigos em salas não visitadas não aparecem
- Inimigos na mesma sala aparecem normalmente
- Corredores se revelam tile a tile conforme o jogador anda

- [ ] **Passo 4: Commit**

```
git add src/core/Game.cpp src/ui/Renderer.cpp
git commit -m "feat: integrate fog of war — explored rooms stay visible forever"
```

---

### Task 7: Verificação final e commit de fechamento

- [ ] **Passo 1: Build limpo**

No VS Code: `Ctrl+Shift+P` → `CMake: Delete Cache and Reconfigure`, depois `CMake: Build`.

Resultado esperado: **Build succeeded**, zero warnings críticos.

- [ ] **Passo 2: Teste completo**

Execute o jogo e confirme todos os comportamentos juntos:

| Comportamento | Esperado |
|---|---|
| HUD inicial | `HP: 2/2  ATK: 5  DEF: 2  LVL: 1  XP: 0/20` |
| Matar Goblin | XP sobe 10 no HUD |
| Matar Troll | XP sobe 25 no HUD |
| Atingir 20 XP | Mensagem `> Nivel 2! +2 HP, +1 ATK` aparece, stats sobem |
| Log com 3+ mensagens | A mais antiga desaparece |
| Sala inicial | Visível desde o começo |
| Nova sala | Revela ao entrar |
| Sala antiga | Permanece visível ao sair |
| Área não visitada | Espaço em branco |
| Inimigo em sala distante | Não aparece |
| Inimigo na mesma área | Aparece normalmente |

- [ ] **Passo 3: Commit de fechamento**

```
git add .
git commit -m "chore: phase 4 complete — XP/levels and fog of war"
```

---

## Resumo dos arquivos modificados

| Arquivo | O que muda |
|---|---|
| `src/entities/Enemy.hpp` | +`getXPReward()` virtual puro |
| `src/entities/Goblin.hpp/.cpp` | +`getXPReward()` → 10 |
| `src/entities/Troll.hpp/.cpp` | +`getXPReward()` → 25 |
| `src/entities/Player.hpp/.cpp` | +`xp_`, `level_`, `xpToNext_`; +`addXP()`; +getters |
| `src/core/Game.hpp` | +`messageLog_`; +`pushMessage()` |
| `src/core/Game.cpp` | loop de XP antes do erase; chama `updateVisibility()`; `render()` passa log |
| `src/map/Map.hpp` | +`explored_[][]`; +`isExplored()`; +`updateVisibility()` |
| `src/map/Map.cpp` | inicializa `explored_`; implementa os dois métodos novos |
| `src/ui/Renderer.hpp` | assinatura de `render()` + `renderMessageLog()` |
| `src/ui/Renderer.cpp` | HUD com LVL/XP; loop de FOW; `renderMessageLog()` |
