# Phase 3 — Inimigos, Factory Pattern e Combate por Colisão

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Adicionar inimigos (Goblin e Troll) com hierarquia de classes, Factory pattern, IA de perseguição simples e combate por colisão (bump attack), incluindo morte do jogador.

**Architecture:** `Enemy` é uma classe abstrata que herda de `Entity`. `Goblin` e `Troll` herdam de `Enemy` e implementam `update(Map&, Player&)`. `EnemyFactory::create()` é o único ponto de criação de inimigos. `Game` possui os inimigos em `vector<unique_ptr<Enemy>>`.

**Tech Stack:** C++17, MSVC, CMake. Sem dependências externas.

---

## Chunk 1: Classe Enemy (abstrata) + takeDamage/isAlive no Player

### Arquivos afetados

- Criar: `src/entities/Enemy.hpp`
- Criar: `src/entities/Enemy.cpp`
- Modificar: `src/entities/Player.hpp`
- Modificar: `src/entities/Player.cpp`

---

### Task 1: Adicionar `takeDamage()` e `isAlive()` ao Player

**Arquivos:**
- Modificar: `src/entities/Player.hpp`
- Modificar: `src/entities/Player.cpp`

Este é um pré-requisito para o combate. O jogador precisa poder receber dano e morrer.

- [ ] **Step 1: Abrir `src/entities/Player.hpp` e adicionar as declarações**

Adicione ao bloco `public`:

```cpp
void takeDamage(int amount);
bool isAlive() const;
```

- [ ] **Step 2: Implementar em `src/entities/Player.cpp`**

```cpp
void Player::takeDamage(int amount) {
    int dano = std::max(1, amount - defense_);
    hp_ -= dano;
}

bool Player::isAlive() const {
    return hp_ > 0;
}
```

Adicione `#include <algorithm>` no topo do arquivo (necessário para `std::max`).

- [ ] **Step 3: Compilar e verificar que não há erros**

No VS Code, pressione `F7`. Saída esperada: `Build finished with exit code 0`.  
O jogo deve rodar normalmente — nenhum comportamento mudou ainda.

---

### Task 2: Criar a classe abstrata `Enemy`

**Arquivos:**
- Criar: `src/entities/Enemy.hpp`
- Criar: `src/entities/Enemy.cpp`

`Enemy` é a base para todos os inimigos. Ela herda de `Entity`, adiciona stats de combate e declara `update(Map&, Player&)` como puro virtual.

- [ ] **Step 1: Criar `src/entities/Enemy.hpp`**

```cpp
#pragma once
#include "Entity.hpp"

class Map;
class Player;

class Enemy : public Entity {
public:
    Enemy(int x, int y, int hp, int attack, int defense);
    virtual ~Enemy() = default;

    // Satisfaz o contrato abstrato de Entity — inimigos não usam a versão sem argumento
    void update() override {}

    // IA do inimigo — chamada pelo Game a cada turno
    virtual void update(Map& map, Player& player) = 0;

    void takeDamage(int amount);
    bool isAlive() const;

    int getHp()      const;
    int getMaxHp()   const;
    int getAttack()  const;
    int getDefense() const;

protected:
    int hp_;
    int maxHp_;
    int attack_;
    int defense_;
};
```

- [ ] **Step 2: Criar `src/entities/Enemy.cpp`**

```cpp
#include "Enemy.hpp"
#include <algorithm>

Enemy::Enemy(int x, int y, int hp, int attack, int defense)
    : Entity(x, y), hp_(hp), maxHp_(hp), attack_(attack), defense_(defense)
{}

void Enemy::takeDamage(int amount) {
    int dano = std::max(1, amount - defense_);
    hp_ -= dano;
}

bool Enemy::isAlive() const {
    return hp_ > 0;
}

int Enemy::getHp()      const { return hp_; }
int Enemy::getMaxHp()   const { return maxHp_; }
int Enemy::getAttack()  const { return attack_; }
int Enemy::getDefense() const { return defense_; }
```

- [ ] **Step 3: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

## Chunk 2: Goblin, Troll e EnemyFactory

### Arquivos afetados

- Criar: `src/entities/Goblin.hpp`
- Criar: `src/entities/Goblin.cpp`
- Criar: `src/entities/Troll.hpp`
- Criar: `src/entities/Troll.cpp`
- Criar: `src/entities/EnemyFactory.hpp`
- Criar: `src/entities/EnemyFactory.cpp`

---

### Task 3: Implementar `Goblin`

**Arquivos:**
- Criar: `src/entities/Goblin.hpp`
- Criar: `src/entities/Goblin.cpp`

Goblin é um inimigo fraco. Stats: hp=6, atk=3, def=1, símbolo `g`.  
IA: perseguir o jogador se dentro de 8 tiles (Chebyshev); atacar se adjacente.

- [ ] **Step 1: Criar `src/entities/Goblin.hpp`**

```cpp
#pragma once
#include "Enemy.hpp"

class Goblin : public Enemy {
public:
    Goblin(int x, int y);
    void update(Map& map, Player& player) override;
};
```

- [ ] **Step 2: Criar `src/entities/Goblin.cpp`**

Este arquivo contém o esqueleto — **você deve implementar o corpo de `update()`**. O plano descreve exatamente o que fazer, mas a implementação é sua parte de aprendizado.

```cpp
#include "Goblin.hpp"
#include "map/Map.hpp"
#include "entities/Player.hpp"
#include <algorithm>  // std::max, std::abs

Goblin::Goblin(int x, int y)
    : Enemy(x, y, 6, 3, 1)
{
    symbol_ = 'g';
}

void Goblin::update(Map& map, Player& player) {
    int dx = player.getX() - x_;
    int dy = player.getY() - y_;
    int dist = std::max(std::abs(dx), std::abs(dy));  // Chebyshev

    if (dist == 1) {
        // Adjacente — atacar
        player.takeDamage(attack_);
    } else if (dist <= 8) {
        // Dentro do raio — mover um passo em direção ao jogador
        int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

        if (map.isWalkable(x_ + stepX, y_ + stepY)) {
            x_ += stepX;
            y_ += stepY;
        } else if (map.isWalkable(x_ + stepX, y_)) {
            x_ += stepX;
        } else if (map.isWalkable(x_, y_ + stepY)) {
            y_ += stepY;
        }
        // Se nenhuma direção for walkable, fica parado
    }
    // dist > 8: não faz nada
}
```

**Atenção:** A implementação acima é a solução de referência — tente implementar você mesmo primeiro, usando as dicas abaixo. Se travar, consulte aqui.

**Dicas para implementar por conta:**
1. Calcule `dx` e `dy` (diferença entre posição do jogador e do goblin)
2. Distância de Chebyshev: `max(abs(dx), abs(dy))`
3. Se `dist == 1`: adjacente → atacar o jogador com `player.takeDamage(attack_)`
4. Se `dist <= 8`: calcular direção (`stepX`, `stepY` com valores -1, 0 ou 1) e mover se walkable
5. Se não for walkable na diagonal, tentar apenas horizontal ou apenas vertical

- [ ] **Step 3: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

### Task 4: Implementar `Troll`

**Arquivos:**
- Criar: `src/entities/Troll.hpp`
- Criar: `src/entities/Troll.cpp`

Troll é um inimigo mais forte. Stats: hp=12, atk=5, def=3, símbolo `T`.  
IA: idêntica ao Goblin — você vai reutilizar a lógica que já escreveu.

- [ ] **Step 1: Criar `src/entities/Troll.hpp`**

```cpp
#pragma once
#include "Enemy.hpp"

class Troll : public Enemy {
public:
    Troll(int x, int y);
    void update(Map& map, Player& player) override;
};
```

- [ ] **Step 2: Criar `src/entities/Troll.cpp`**

```cpp
#include "Troll.hpp"
#include "map/Map.hpp"
#include "entities/Player.hpp"
#include <algorithm>

Troll::Troll(int x, int y)
    : Enemy(x, y, 12, 5, 3)
{
    symbol_ = 'T';
}

void Troll::update(Map& map, Player& player) {
    int dx = player.getX() - x_;
    int dy = player.getY() - y_;
    int dist = std::max(std::abs(dx), std::abs(dy));

    if (dist == 1) {
        player.takeDamage(attack_);
    } else if (dist <= 8) {
        int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

        if (map.isWalkable(x_ + stepX, y_ + stepY)) {
            x_ += stepX;
            y_ += stepY;
        } else if (map.isWalkable(x_ + stepX, y_)) {
            x_ += stepX;
        } else if (map.isWalkable(x_, y_ + stepY)) {
            y_ += stepY;
        }
    }
}
```

**Nota:** A lógica é idêntica ao Goblin — só os stats mudam. Em Phase 6 extrairemos isso para um método compartilhado ou Strategy pattern.

- [ ] **Step 3: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

### Task 5: Criar `EnemyFactory`

**Arquivos:**
- Criar: `src/entities/EnemyFactory.hpp`
- Criar: `src/entities/EnemyFactory.cpp`

A factory é o único lugar que conhece as classes concretas. `Game` só conhece `EnemyType`.

- [ ] **Step 1: Criar `src/entities/EnemyFactory.hpp`**

```cpp
#pragma once
#include <memory>
#include "Enemy.hpp"

enum class EnemyType { Goblin, Troll };

class EnemyFactory {
public:
    static std::unique_ptr<Enemy> create(EnemyType type, int x, int y);
};
```

- [ ] **Step 2: Criar `src/entities/EnemyFactory.cpp`**

```cpp
#include "EnemyFactory.hpp"
#include "Goblin.hpp"
#include "Troll.hpp"

std::unique_ptr<Enemy> EnemyFactory::create(EnemyType type, int x, int y) {
    switch (type) {
        case EnemyType::Goblin: return std::make_unique<Goblin>(x, y);
        case EnemyType::Troll:  return std::make_unique<Troll>(x, y);
        default:                return nullptr;
    }
}
```

- [ ] **Step 3: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

## Chunk 3: Integração no Game (spawn, update loop, remoção de mortos)

### Arquivos afetados

- Modificar: `src/core/Game.hpp`
- Modificar: `src/core/Game.cpp`

---

### Task 6: Adicionar `enemies_` ao Game e fazer o spawn

**Arquivos:**
- Modificar: `src/core/Game.hpp`
- Modificar: `src/core/Game.cpp`

- [ ] **Step 1: Atualizar `src/core/Game.hpp`**

Adicione os includes necessários e o membro `enemies_`:

```cpp
#include "entities/EnemyFactory.hpp"
#include <vector>
#include <memory>
```

No bloco `private`, adicione:

```cpp
std::vector<std::unique_ptr<Enemy>> enemies_;
```

- [ ] **Step 2: Adicionar spawn de inimigos no construtor `Game::Game()` em `Game.cpp`**

Após o bloco que posiciona o jogador, adicione:

```cpp
// Spawna inimigos — pula a sala 0 (sala do jogador)
std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);
std::uniform_int_distribution<int> randType(0, 1);

const auto& rooms = map_.getRooms();
for (size_t i = 1; i < rooms.size(); ++i) {
    Point center = rooms[i].center();
    EnemyType type = (randType(rng) == 0) ? EnemyType::Goblin : EnemyType::Troll;
    enemies_.push_back(EnemyFactory::create(type, center.x, center.y));
}
```

Adicione `#include <random>` e `#include <ctime>` no topo se ainda não estiverem presentes.

- [ ] **Step 3: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.  
O jogo ainda não mostra inimigos — isso vem no Chunk 4.

---

### Task 7: Atualizar o loop de update — rodar IA dos inimigos e remover mortos

**Arquivos:**
- Modificar: `src/core/Game.cpp`

- [ ] **Step 1: Substituir o placeholder em `Game::update()`**

Localize em `Game.cpp` o comentário dentro de `Game::update()` que menciona "Fase 3" e "inimigos" (algo como `// Na Fase 3 este método vai iterar por todos os inimigos também`). Substitua essa linha por:

```cpp
// Atualiza todos os inimigos vivos
for (auto& enemy : enemies_) {
    if (enemy->isAlive()) {
        enemy->update(map_, player_);
    }
}

// Remove inimigos mortos
enemies_.erase(
    std::remove_if(enemies_.begin(), enemies_.end(),
        [](const std::unique_ptr<Enemy>& e) { return !e->isAlive(); }),
    enemies_.end()
);

// Verifica morte do jogador
if (!player_.isAlive()) {
    isRunning_ = false;
}
```

Adicione `#include <algorithm>` no topo se ainda não estiver.

- [ ] **Step 2: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

## Chunk 4: Combate do jogador (bump attack) + Renderer

### Arquivos afetados

- Modificar: `src/entities/Player.hpp`
- Modificar: `src/entities/Player.cpp`
- Modificar: `src/core/Game.cpp`
- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`

---

### Task 8: Bump attack — Player detecta inimigo na tile destino

**Arquivos:**
- Modificar: `src/entities/Player.hpp`
- Modificar: `src/entities/Player.cpp`

- [ ] **Step 1: Atualizar assinatura de `move()` em `Player.hpp`**

Primeiro, adicione os includes no **topo** do arquivo `Player.hpp`, logo após o `#pragma once` e antes dos outros includes existentes:

```cpp
#include <vector>
#include <memory>
```

Depois, no bloco de declarações `public`, adicione a forward declaration antes da declaração de `move()`:

```cpp
// Forward declaration — evita include circular
```

No **início do arquivo** (antes da classe), adicione:

```cpp
class Enemy;  // forward declaration
```

Por fim, substitua a declaração de `move()`:

```cpp
void move(int dx, int dy, class Map& map);
```

Por:

```cpp
void move(int dx, int dy, class Map& map, std::vector<std::unique_ptr<Enemy>>& enemies);
```

**Atenção:** `#include <vector>` e `#include <memory>` vão no **topo do arquivo**, nunca dentro da classe. A forward declaration `class Enemy;` vai antes da definição da classe `Player`.

- [ ] **Step 2: Atualizar implementação em `Player.cpp`**

Substitua o corpo de `Player::move()` por:

```cpp
void Player::move(int dx, int dy, Map& map, std::vector<std::unique_ptr<Enemy>>& enemies) {
    int newX = x_ + dx;
    int newY = y_ + dy;

    // Verifica se há inimigo na tile destino
    for (auto& enemy : enemies) {
        if (enemy->isAlive() && enemy->getX() == newX && enemy->getY() == newY) {
            enemy->takeDamage(attack_);  // Bump attack!
            return;                      // Não move — o turno foi gasto atacando
        }
    }

    // Nenhum inimigo — move normalmente se walkable
    if (map.isWalkable(newX, newY)) {
        x_ = newX;
        y_ = newY;
    }
}
```

Adicione `#include "Enemy.hpp"` no topo de `Player.cpp`.

- [ ] **Step 3: Corrigir chamadas de `move()` em `Game.cpp`**

Em `Game::processInput()`, todas as chamadas `player_.move(dx, dy, map_)` precisam passar `enemies_` como quarto argumento:

```cpp
case 'w': player_.move( 0, -1, map_, enemies_); break;
case 's': player_.move( 0,  1, map_, enemies_); break;
case 'a': player_.move(-1,  0, map_, enemies_); break;
case 'd': player_.move( 1,  0, map_, enemies_); break;
```

- [ ] **Step 4: Compilar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

---

### Task 9: Atualizar o Renderer para desenhar inimigos

**Arquivos:**
- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`

- [ ] **Step 1: Atualizar `Renderer.hpp`**

Adicione includes e atualize a assinatura de `render()`:

```cpp
#include "entities/Enemy.hpp"
#include <vector>
#include <memory>
```

Substitua:

```cpp
void render(const Map& map, const Player& player);
```

Por:

```cpp
void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies);
```

- [ ] **Step 2: Atualizar `Renderer.cpp` — adicionar inimigos ao loop de desenho**

Substitua o loop de renderização do mapa em `Renderer::render()` por:

```cpp
void Renderer::render(const Map& map, const Player& player,
                      const std::vector<std::unique_ptr<Enemy>>& enemies) {
    clearScreen();
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            // Jogador tem prioridade máxima
            if (x == player.getX() && y == player.getY()) {
                std::cout << player.getSymbol();
                continue;
            }
            // Verifica se há inimigo nesta tile
            bool hadEnemy = false;
            for (const auto& enemy : enemies) {
                if (enemy->isAlive() && enemy->getX() == x && enemy->getY() == y) {
                    std::cout << enemy->getSymbol();
                    hadEnemy = true;
                    break;
                }
            }
            if (!hadEnemy) {
                std::cout << map.getTile(x, y);
            }
        }
        std::cout << '\n';
    }
    renderHUD(player);
}
```

- [ ] **Step 3: Corrigir chamada em `Game::render()`**

Em `Game.cpp`, substitua:

```cpp
renderer_.render(map_, player_);
```

Por:

```cpp
renderer_.render(map_, player_, enemies_);
```

- [ ] **Step 4: Compilar e rodar**

Pressione `F7`. Saída esperada: `Build finished with exit code 0`.

Execute: `.\build\Debug\roguelike.exe`

Você deve ver:
- Inimigos `g` (Goblin) e `T` (Troll) espalhados pelo mapa nas salas
- Inimigos perseguem o jogador quando próximos
- Colidir com um inimigo causa dano nele (não move o jogador)
- Inimigos adjacentes atacam o jogador automaticamente no turno deles
- Quando o HP chega a 0, o jogo termina

---

## Chunk 5: Mensagens de combate e morte

### Arquivos afetados

- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`
- Modificar: `src/core/Game.cpp`

---

### Task 10: Mensagem de derrota

**Arquivos:**
- Modificar: `src/core/Game.cpp`

- [ ] **Step 1: Atualizar o print de fim de jogo em `Game::run()`**

Substitua:

```cpp
std::cout << "\nAté a próxima aventura!\n";
```

Por:

```cpp
if (!player_.isAlive()) {
    std::cout << "\n  Você morreu! Fim de jogo.\n";
} else {
    std::cout << "\nAté a próxima aventura!\n";
}
```

- [ ] **Step 2: Compilar e rodar para verificar o jogo completo**

Pressione `F7`. Execute: `.\build\Debug\roguelike.exe`

Teste:
- [ ] Inimigos aparecem no mapa
- [ ] Inimigos se movem em direção ao jogador
- [ ] Colidir com inimigo reduz HP do inimigo (veja HUD não quebra — HP do jogador visível)
- [ ] Inimigo adjacente reduz HP do jogador
- [ ] Inimigo com HP zerado desaparece do mapa
- [ ] Jogador com HP zerado vê mensagem de derrota e jogo termina

---

## Resumo dos arquivos criados/modificados

| Arquivo | Ação |
|---|---|
| `src/entities/Enemy.hpp` | Criado |
| `src/entities/Enemy.cpp` | Criado |
| `src/entities/Goblin.hpp` | Criado |
| `src/entities/Goblin.cpp` | Criado — **você implementa `update()`** |
| `src/entities/Troll.hpp` | Criado |
| `src/entities/Troll.cpp` | Criado — **você implementa `update()`** |
| `src/entities/EnemyFactory.hpp` | Criado |
| `src/entities/EnemyFactory.cpp` | Criado |
| `src/entities/Player.hpp` | Modificado — `takeDamage`, `isAlive`, novo `move()` |
| `src/entities/Player.cpp` | Modificado |
| `src/core/Game.hpp` | Modificado — `enemies_` |
| `src/core/Game.cpp` | Modificado — spawn, update loop, bump attack, morte |
| `src/ui/Renderer.hpp` | Modificado — novo parâmetro |
| `src/ui/Renderer.cpp` | Modificado — desenha inimigos |
