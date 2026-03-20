# Phase 3 — Inimigos, Factory Pattern e Combate por Colisão

**Data:** 2026-03-19  
**Status:** Aprovado  

---

## Objetivo

Adicionar inimigos ao jogo com hierarquia de classes correta, Factory pattern para criação, IA simples de perseguição e combate por colisão (bump attack).

---

## Hierarquia de Classes

```
Entity  (abstrata, x_, y_, symbol_, virtual update() = 0)
├── Player  (hp_, maxHp_, attack_, defense_, move(), takeDamage(), isAlive())
└── Enemy   (abstrata, hp_, maxHp_, attack_, defense_, takeDamage(), isAlive(), virtual update(Map&, Player&) = 0)
    ├── Goblin   (hp=6,  atk=3, def=1, symbol='g')
    └── Troll    (hp=12, atk=5, def=3, symbol='T')
```

**Decisões:**
- `Enemy::update(Map&, Player&)` — versão com dois argumentos chamada pelo `Game`. Recebe mapa (para checar walkability) e jogador (para posição e combate).
- `Entity::update()` sem argumentos permanece na base como no-op em `Enemy` (satisfaz o contrato abstrato existente).
- `takeDamage(int amount)` adicionado em `Player` e `Enemy`. Fórmula: `max(1, amount - defense_)`.
- `isAlive()` retorna `hp_ > 0`. Usado pelo `Game` para remover mortos.

---

## Factory Pattern

```cpp
enum class EnemyType { Goblin, Troll };

class EnemyFactory {
public:
    static std::unique_ptr<Enemy> create(EnemyType type, int x, int y);
};
```

- Método estático — não requer instância.
- Retorna `unique_ptr<Enemy>` — Game é dono exclusivo, destruição automática.
- Game nunca chama `new Goblin(...)` diretamente.
- Adicionar novo tipo no futuro: adicionar valor no enum + um `case` na factory. Nada mais muda.

**Spawn no Game:**
- Após `map_.generate()`, iterar sobre `map_.getRooms()` pulando o índice 0 (sala do jogador).
- Para cada sala: sortear `EnemyType` aleatório, spawnar no `room.center()`.
- Guardar em `std::vector<std::unique_ptr<Enemy>> enemies_` em `Game`.

---

## Combate por Colisão (Bump Attack)

**Jogador ataca inimigo:**
- `Player::move(dx, dy, Map&, vector<unique_ptr<Enemy>>&)` ganha o vetor de inimigos.
- Antes de mover: verificar se há inimigo vivo na tile destino.
- Se sim: chamar `enemy->takeDamage(attack_)` e não mover. Turno consumido.
- Se não: mover normalmente (comportamento atual).

**Inimigo ataca jogador:**
- Dentro de `Enemy::update(Map&, Player&)`:
  - Se adjacente ao jogador (distância de Chebyshev == 1): chamar `player.takeDamage(attack_)`.
  - Se dentro do raio de detecção (Chebyshev <= 8) mas não adjacente: mover um passo em direção ao jogador se a tile for walkable.
  - Caso contrário: ficar parado.

**Fórmula de dano:**
```
dano = max(1, atacante.attack_ - defensor.defense_)
```
Mínimo 1 — combate sempre progride.

**Morte:**
- Inimigos com `hp_ <= 0` são removidos de `enemies_` no final de `Game::update()` com `erase` + `remove_if`.
- Morte do jogador: `isRunning_ = false` + mensagem de derrota.

---

## Renderer

`Renderer::render()` recebe um terceiro parâmetro:

```cpp
void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies);
```

Ordem de prioridade de desenho na mesma tile:
1. Tile do mapa (base)
2. Símbolo do inimigo (sobrescreve mapa)
3. Símbolo do jogador (sobrescreve tudo)

---

## Arquivos Afetados

| Arquivo | Mudança |
|---|---|
| `src/entities/Enemy.hpp/.cpp` | Novo — classe abstrata Enemy |
| `src/entities/Goblin.hpp/.cpp` | Novo — inimigo concreto |
| `src/entities/Troll.hpp/.cpp` | Novo — inimigo concreto |
| `src/entities/EnemyFactory.hpp/.cpp` | Novo — Factory |
| `src/entities/Player.hpp/.cpp` | Adicionar `takeDamage()`, `isAlive()`, novo parâmetro em `move()` |
| `src/core/Game.hpp/.cpp` | Adicionar `enemies_`, spawn, update loop, remoção de mortos |
| `src/ui/Renderer.hpp/.cpp` | Adicionar parâmetro de enemies em `render()` |

---

## O que NÃO entra nesta fase

- Strategy pattern para IA (Phase 6)
- IRenderer interface (Phase 5/6)
- Tecla de ataque explícita (avaliar após sentir o bump attack)
- A* pathfinding (Phase 6)
- Drops de itens (Phase 5)
