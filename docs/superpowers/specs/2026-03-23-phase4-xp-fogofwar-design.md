# Phase 4 Design Spec — XP/Níveis + Fog of War

**Data:** 2026-03-23  
**Status:** Aprovado  

---

## Contexto

O projeto é um roguelike clássico em C++ (inspirado em Rogue 1980), sendo desenvolvido como exercício de aprendizado de SOLID, Design Patterns e Clean Code. A Phase 3 entregou inimigos (`Goblin`, `Troll`), combate bump-attack e spawn por sala. A Phase 4 adiciona progressão do jogador (XP e níveis) e exploração com Fog of War simplificado.

---

## Escopo

Dois sistemas independentes, implementados na mesma fase:

1. **XP e Níveis** — o jogador ganha XP ao matar inimigos e sobe de nível automaticamente
2. **Fog of War** — salas não visitadas ficam invisíveis; salas exploradas ficam visíveis para sempre

---

## Sistema 1 — XP e Níveis

### Decisões

- Stats aumentam **automaticamente** ao subir de nível (sem escolha do jogador)
- XP por inimigo: Goblin = 10, Troll = 25
- XP necessário por nível: começa em 20, multiplica por 1.5 a cada nível
  - Nível 2: 20 XP
  - Nível 3: 30 XP
  - Nível 4: 45 XP
  - Nível 5: 67 XP (arredondado para baixo com `int`)
- Ganhos por level-up: +2 `maxHp_` (e `hp_` sobe junto), +1 `attack_`
- Notificação: mensagem inserida no log de mensagens, permanece visível até ser empurrada por 3 mensagens mais novas

### Mudanças em `Player`

Novos campos privados:
```cpp
int xp_       = 0;
int level_    = 1;
int xpToNext_ = 20;
```

Novo método público:
```cpp
// Adiciona XP e faz level-up se necessário.
// Retorna string de notificação se subiu de nível, string vazia caso contrário.
std::string addXP(int amount);
```

Novos getters públicos:
```cpp
int getXP()       const;
int getLevel()    const;
int getXPToNext() const;
```

Lógica de `addXP`:
```
xp_ += amount
std::string notification = ""
while xp_ >= xpToNext_:
    xp_ -= xpToNext_
    xpToNext_ = int(xpToNext_ * 1.5)
    level_++
    maxHp_ += 2
    hp_    += 2
    attack_ += 1
    notification = "Nivel " + level_ + "! +2 HP, +1 ATK"
return notification
```

Nota: o loop acumula todos os level-ups possíveis em uma única chamada. A notificação retornada reflete o nível final atingido.

### Mudanças em `Enemy`

Novo método virtual puro na classe abstrata `Enemy`:
```cpp
virtual int getXPReward() const = 0;
```

Implementações:
- `Goblin::getXPReward()` → retorna `10`
- `Troll::getXPReward()`  → retorna `25`

### Mudanças em `Game`

Em `Game::update()`, **antes** do `erase`/`remove_if`:
```cpp
for (auto& enemy : enemies_) {
    if (!enemy->isAlive()) {
        std::string msg = player_.addXP(enemy->getXPReward());
        if (!msg.empty()) pushMessage(msg);
    }
}
// depois: erase/remove_if como antes
```

### Log de Mensagens

Novo campo em `Game`:
```cpp
std::deque<std::string> messageLog_;  // máximo 3 mensagens
```

Regra de inserção:
```cpp
void Game::pushMessage(const std::string& msg) {
    messageLog_.push_back(msg);
    if (messageLog_.size() > 3) messageLog_.pop_front();
}
```

`Renderer::render()` recebe o log e imprime as mensagens abaixo do HUD. A assinatura atualizada será:
```cpp
void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies,
            const std::deque<std::string>& messageLog);
```

### Mudanças no HUD

Linha atual:
```
HP: 2/2  ATK: 5  DEF: 2
```

Nova linha:
```
HP: 2/2  ATK: 5  DEF: 2  LVL: 1  XP: 0/20
```

Abaixo do HUD, até 3 linhas de log:
```
> Nivel 2! +2 HP, +1 ATK
```

---

## Sistema 2 — Fog of War

### Decisões

- **Dois estados apenas:** explorado (visível para sempre) e não explorado (espaço em branco)
- **Sem grid `visible_`** — apenas `explored_[][]`
- **Raio de visão:** sala inteira onde o jogador está + 1 tile de margem (paredes da sala)
- **Corredores:** tiles walkable num raio de 1 ao redor do jogador também são explorados
- **Inimigos:** só aparecem se o tile onde estão for `explored_` **E** o jogador estiver na mesma sala (Chebyshev ≤ raio da sala)

### Mudanças em `Map`

Novo campo privado:
```cpp
std::vector<std::vector<bool>> explored_;  // inicializado com false
```

Inicialização em `Map::Map()` (ou `generate()`): `explored_` tem as mesmas dimensões que `tiles_`, tudo `false`.

Novos métodos públicos:
```cpp
bool isExplored(int x, int y) const;
void updateVisibility(int px, int py);
```

Lógica de `updateVisibility(px, py)`:
1. Encontra a sala que contém `(px, py)` iterando `rooms_` com `room.contains(x, y)`
2. Se encontrou uma sala:
   - Itera o bounding box da sala + 1 tile de margem (paredes)
   - Marca cada tile dentro do bounds como `explored_ = true`
3. Também marca como `explored_` todos os tiles walkable num raio de 1 ao redor de `(px, py)` — cobre corredores

### Mudanças em `Renderer`

Em `Renderer::render()`, para cada tile `(x, y)`:

```
if (!map.isExplored(x, y)):
    desenha ' '  (espaço em branco)
else:
    desenha normalmente (tile, inimigo se visível, jogador)
```

Inimigo só é desenhado no tile `(x, y)` se:
- `map.isExplored(x, y)` é `true`
- **E** o inimigo está na mesma sala que o jogador (Chebyshev entre jogador e inimigo ≤ raio máximo de sala, ~10)

Simplificação aceitável: verificar se `map.isExplored(enemy.x, enemy.y)` **E** distância Chebyshev entre jogador e inimigo ≤ 10.

### Mudanças em `Game`

Em `Game::update()`, após processar input e mover o jogador:
```cpp
map_.updateVisibility(player_.getX(), player_.getY());
```

---

## Arquitetura — Resumo de Mudanças por Arquivo

| Arquivo | O que muda |
|---|---|
| `Player.hpp / .cpp` | +`xp_`, `level_`, `xpToNext_`; +`addXP()`; +getters |
| `Enemy.hpp` | +`getXPReward()` virtual puro |
| `Enemy.cpp` | sem mudanças — método é puro, sem implementação em Enemy |
| `Goblin.hpp / .cpp` | +`getXPReward()` retorna 10 |
| `Troll.hpp / .cpp` | +`getXPReward()` retorna 25 |
| `Map.hpp / .cpp` | +`explored_[][]`; +`isExplored()`; +`updateVisibility()` |
| `Renderer.hpp / .cpp` | render condicional por `isExplored`; log de mensagens no HUD |
| `Game.hpp / .cpp` | +`messageLog_`; +`pushMessage()`; chama `addXP` e `updateVisibility` |

---

## O que NÃO está no escopo desta fase

- Efeitos de status (fase posterior)
- Sistema de turnos robusto (avaliar se necessário)
- Itens e inventário (Phase 5)
- IA avançada com A* (Phase 6)

---

## Conceitos de C++ praticados

- `std::deque` para log com remoção eficiente da frente
- Método virtual puro em hierarquia de herança (`getXPReward`)
- `std::vector<std::vector<bool>>` para grid 2D
- Encapsulamento: `Map` dono da lógica de visibilidade
