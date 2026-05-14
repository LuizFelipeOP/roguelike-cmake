# Plano de Implementação — Phase 7b: IA Strategy + Orc + Fuga

**Data:** 2026-04-30
**Spec:** `docs/superpowers/specs/2026-04-30-phase7b-ia-strategy.md`
**Status:** Pendente

---

## Visão geral

3 chunks sequenciais. Cada chunk deve compilar e passar nos 71 testes antes de avançar.

---

## Chunk 1 — Interface `IAStrategy` e `PerseguirStrategy`

**Conceito:** Criar a pasta `src/ia/` com a interface e a primeira implementação concreta. Em seguida, refatorar `Enemy`, `Goblin` e `Troll` para usar a Strategy.

### Passo 1 — `src/ia/IAStrategy.hpp`

Criar interface pura com forward declarations de `Enemy`, `Map` e `Player`:

```cpp
#pragma once
class Enemy;
class Map;
class Player;

class IAStrategy {
public:
    virtual void agir(Enemy& self, Map& map, Player& player) = 0;
    virtual ~IAStrategy() = default;
};
```

> Usar forward declarations evita includes circulares. O `.cpp` da implementação concreta fará os includes reais.

### Passo 2 — `src/ia/PerseguirStrategy.hpp`

```cpp
#pragma once
#include "IAStrategy.hpp"
#include <functional>

class Player;

class PerseguirStrategy : public IAStrategy {
    std::function<void(Player&)> onAtaque_;
public:
    explicit PerseguirStrategy(std::function<void(Player&)> onAtaque = nullptr);
    void agir(Enemy& self, Map& map, Player& player) override;
};
```

### Passo 3 — `src/ia/PerseguirStrategy.cpp`

- Incluir `Enemy.hpp`, `Map.hpp`, `Player.hpp`
- `agir()` contém exatamente o código hoje em `Goblin::update()` / `Troll::update()`:
  - Calcular `distX`, `distY`, `distancia`
  - Se `distancia == 1`: chamar `player.takeDamage(self.getAttack())` e, se `onAtaque_` existir, chamar `onAtaque_(player)`
  - Se `distancia <= 8`: calcular step e chamar `self.moverPara(novoX, novoY)`

> **Atenção:** `moverPara()` ainda não existe em `Enemy` — você vai criá-lo no passo seguinte. Por enquanto, declare-o como se já existisse; o compilador vai reclamar até que o Passo 4 esteja feito.

### Passo 4 — Modificar `Enemy.hpp`

Adicionar:

```cpp
#include "ia/IAStrategy.hpp"
#include <memory>
#include <functional>

// Campo privado:
std::unique_ptr<IAStrategy> ia_;

// Métodos públicos:
void setIA(std::unique_ptr<IAStrategy> ia);

// Método protegido (acessível pela Strategy via Enemy&):
protected:
    void moverPara(int x, int y);
```

Remover `virtual void update(Map& map, Player& player) = 0` — passa a ter implementação padrão em `Enemy.cpp`.

### Passo 5 — Modificar `Enemy.cpp`

Implementar:

```cpp
void Enemy::setIA(std::unique_ptr<IAStrategy> ia) {
    ia_ = std::move(ia);
}

void Enemy::moverPara(int x, int y) {
    x_ = x;
    y_ = y;
}

void Enemy::update(Map& map, Player& player) {
    if (isParalisado()) return;
    if (ia_) ia_->agir(*this, map, player);
}
```

> **Atenção:** `x_` e `y_` são privados em `Entity`. `moverPara()` está em `Enemy`, que é subclasse de `Entity` — então pode chamar `x_ = x` diretamente? Não — campos privados não são acessíveis nem por subclasses. Você precisa verificar se `Entity` expõe `setX()`/`setY()`, ou adicioná-los como `protected`. Leia `Entity.hpp` antes de implementar este passo.

### Passo 6 — Modificar `Goblin.cpp`

- Remover o método `update()` inteiro
- No construtor, após `symbol_ = 'g'`, adicionar:

```cpp
static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
    std::uniform_int_distribution<int> chance(0, 1);
    if (chance(rng) == 1) {
        p.adicionarEfeito(std::make_unique<VenenoEffect>(1, 3));
    }
}));
```

> **Atenção:** o `rng` capturado no lambda precisa ser `static` para ser acessível. Verifique o escopo — pode ser necessário defini-lo fora do lambda ou capturar por referência.

### Passo 7 — Modificar `Troll.cpp`

Mesma abordagem que o Goblin:

```cpp
setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> chance(1, 100);
    if (chance(rng) <= 20) {
        p.adicionarEfeito(std::make_unique<ParalisiaEffect>(2));
    }
}));
```

### Verificação do Chunk 1

```
cmake -S . -B build && cmake --build build
```

Comportamento do jogo deve ser **idêntico** ao antes — Goblin ainda venena, Troll ainda paralisa, movimento igual.

---

## Chunk 2 — Orc com `EnfraquecimentoEffect`

**Conceito:** Novo inimigo sem tocar em nenhuma subclasse existente.

### Passo 1 — `src/entities/Orc.hpp`

```cpp
#pragma once
#include "Enemy.hpp"

class Orc : public Enemy {
public:
    Orc(int x, int y, int andar);
    int getXPReward() override;
};
```

### Passo 2 — `src/entities/Orc.cpp`

- Stats: `Enemy(x, y, 9 + (andar-1)*4, 4 + (andar-1)*2, 2, 18)`
- Símbolo: `'O'`
- `setIA(make_unique<PerseguirStrategy>(...))` com callback de 30% `EnfraquecimentoEffect(2, 3)`:

```cpp
setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> chance(1, 100);
    if (chance(rng) <= 30) {
        p.adicionarEfeito(std::make_unique<EnfraquecimentoEffect>(2, 3));
    }
}));
```

### Passo 3 — `EnemyFactory.hpp`

Adicionar `Orc` ao enum `EnemyType`:

```cpp
enum class EnemyType { Goblin, Troll, Orc };
```

### Passo 4 — `EnemyFactory.cpp`

Na `tabelaSpawn`, adicionar entrada para Orc com pesos:

| Andar | Goblin | Troll | Orc |
|---|---|---|---|
| 1–2 | 90 | 10 | 0 |
| 3–4 | 60 | 20 | 20 |
| 5–6 | 40 | 30 | 30 |
| 7+ | 20 | 40 | 40 |

Adicionar `case EnemyType::Orc` no `switch` de `sortear()`.

### Verificação do Chunk 2

Rodar o jogo e ir até o andar 3+. Orc deve aparecer e aplicar `[Enfraquecimento]` visível no HUD.

---

## Chunk 3 — `FugirStrategy` e fuga do Goblin

**Conceito:** Goblin com HP baixo troca de Strategy em tempo de execução.

### Passo 1 — `src/ia/FugirStrategy.hpp`

```cpp
#pragma once
#include "IAStrategy.hpp"

class FugirStrategy : public IAStrategy {
public:
    void agir(Enemy& self, Map& map, Player& player) override;
};
```

### Passo 2 — `src/ia/FugirStrategy.cpp`

Lógica de fuga:
- Calcular `distX` e `distY` em relação ao player
- O step de fuga é o **inverso** da perseguição: `-stepX`, `-stepY`
- Testar tile `(x - stepX, y - stepY)` — se walkable, mover; senão tentar eixos alternativos
- **Não atacar** — não chamar `player.takeDamage()`

### Passo 3 — Modificar `Goblin.hpp`

Adicionar override de `takeDamage()`:

```cpp
void takeDamage(int amount) override;
```

### Passo 4 — Modificar `Goblin.cpp`

Implementar `takeDamage()`:

```cpp
void Goblin::takeDamage(int amount) {
    Enemy::takeDamage(amount);  // chama o takeDamage base
    // Ativa fuga se HP caiu abaixo de 30% do máximo
    if (isAlive() && getHp() < getMaxHp() * 0.3f) {
        setIA(std::make_unique<FugirStrategy>());
    }
}
```

> **Atenção:** Para calcular 30%, você precisa de `getMaxHp()` em `Enemy`. Verifique se já existe — se não, precisará ser adicionado. Leia `Enemy.hpp` e `Entity.hpp` antes deste passo.

### Verificação do Chunk 3

Atacar um Goblin repetidamente. Quando ele estiver com pouco HP, deve começar a fugir (se afastar do player ao invés de perseguir). O `[Veneno]` não deve mais ser aplicado após ativar a fuga.

---

## Checklist de conclusão

- [ ] Chunk 1: `IAStrategy` + `PerseguirStrategy` — comportamento de Goblin/Troll inalterado, 71 testes passando
- [ ] Chunk 2: Orc aparece no andar 3+, aplica Enfraquecimento, HUD exibe `[Enfraquecimento]`
- [ ] Chunk 3: Goblin foge quando HP < 30%, deixa de aplicar Veneno ao fugir
- [ ] Nenhum crash com múltiplos inimigos simultâneos em estados diferentes
- [ ] 71+ testes passando ao final (novos testes para Orc e fuga são bem-vindos)

---

## Dicas e armadilhas

### Includes circulares
`PerseguirStrategy.cpp` inclui `Enemy.hpp`, que inclui `IAStrategy.hpp`. Isso é seguro porque `IAStrategy.hpp` usa apenas forward declaration de `Enemy`. Nunca coloque `#include "Enemy.hpp"` dentro de `IAStrategy.hpp`.

### `static rng` no lambda
O gerador `mt19937` dentro do lambda do construtor precisa ser `static` para não ser recriado a cada chamada. Como lambdas em construtores podem ser tricky, a forma mais segura é definir o `rng` como `static` local dentro da função `agir()` da `PerseguirStrategy`, não dentro do lambda — e passar o resultado do sorteio pelo callback.

Alternativa mais simples: mover o `rng` para um campo da `PerseguirStrategy` ou da subclasse.

### `moverPara()` vs campos privados de `Entity`
`x_` e `y_` são `protected` ou `private` em `Entity`? Leia antes de implementar. Se forem `private`, `Enemy::moverPara()` não pode acessá-los diretamente — precisará de `setX()`/`setY()` em `Entity`, ou promover para `protected`.

### Ordem de leitura recomendada antes de cada chunk
- Chunk 1: `Entity.hpp`, `Enemy.hpp`, `Goblin.cpp`, `Troll.cpp`
- Chunk 2: `EnemyFactory.hpp/.cpp`
- Chunk 3: `Enemy.hpp` (verificar `getMaxHp()`), `Goblin.hpp`
