# Spec — Phase 7b: IA Strategy + Orc + Fuga

**Data:** 2026-04-30
**Status:** Aprovada

---

## Objetivo

Refatorar a IA dos inimigos para o padrão **Strategy**, introduzir um novo inimigo (**Orc**) e adicionar **comportamento de fuga** ao Goblin quando com pouco HP.

---

## Motivação

Atualmente `Goblin::update()` e `Troll::update()` contêm código de IA duplicado (perseguição + bump attack). Qualquer novo comportamento exige copiar e modificar esse código em cada subclasse, violando OCP e DRY. Extraindo a lógica para `IAStrategy`, novos comportamentos (fuga, patrulha, ataque à distância) podem ser criados e combinados sem tocar nos inimigos existentes.

---

## Decisões de design

### Interface `IAStrategy`

Interface pura em `src/ia/IAStrategy.hpp`:

```cpp
class IAStrategy {
public:
    virtual void agir(Enemy& self, Map& map, Player& player) = 0;
    virtual ~IAStrategy() = default;
};
```

- `agir()` recebe `Enemy& self` para poder ler e modificar o estado do inimigo
- `Map&` para checar tiles walkable
- `Player&` para calcular distância e aplicar dano/efeitos

### `PerseguirStrategy`

Encapsula o comportamento atual de Goblin e Troll:
- Se `distancia == 1`: ataca o player com `player.takeDamage(self.getAttack())` e dispara callback `onAtaque`
- Se `distancia <= 8`: move-se em direção ao player (step a step, com fallback de eixo)
- Se `distancia > 8`: fica parado

```cpp
// Construtor aceita callback opcional para efeitos ao atacar
PerseguirStrategy(std::function<void(Player&)> onAtaque = nullptr);
```

Goblin passa: `[](Player& p){ /* 50% VenenoEffect */ }`
Troll passa: `[](Player& p){ /* 20% ParalisiaEffect */ }`

### `moverPara()` em `Enemy`

`agir()` precisa modificar `x_` e `y_` do inimigo, que são privados em `Entity`. A solução é adicionar um método protegido em `Enemy`:

```cpp
protected:
    void moverPara(int x, int y);
```

Assim `IAStrategy` chama `self.moverPara(x, y)` sem violar o encapsulamento de `Entity`.

### Delegação em `Enemy::update()`

`Enemy` passa a ter `update(Map&, Player&)` não-abstrato, que:
1. Retorna se `isParalisado()`
2. Chama `ia_->agir(*this, map, player)` se `ia_` existir

```cpp
// Enemy.hpp
std::unique_ptr<IAStrategy> ia_;
void setIA(std::unique_ptr<IAStrategy> ia);
```

As subclasses **não precisam mais sobrescrever `update(Map&, Player&)`** — apenas injetam a strategy no construtor.

---

## Orc — novo inimigo

| Atributo | Valor |
|---|---|
| Símbolo | `O` |
| HP | `9 + (andar-1) * 4` |
| ATK | `4 + (andar-1) * 2` |
| DEF | `2` |
| XP | `18` |
| Efeito ao atacar | 30% de chance de `EnfraquecimentoEffect(2, 3)` |
| IA | `PerseguirStrategy` com callback de enfraquecimento |
| Spawn | A partir do andar 3, peso crescente |

O Orc demonstra que adicionar um novo inimigo não requer modificar nenhuma subclasse existente — apenas criar `Orc.hpp/.cpp` e adicionar na `tabelaSpawn`.

---

## `FugirStrategy`

Comportamento:
- Calcula o tile adjacente walkable **mais distante** do player
- Move-se para esse tile
- **Não ataca** enquanto foge

Ativação:
- Exclusiva do **Goblin** — ele sobrescreve `takeDamage()` e, ao cair abaixo de **30% do HP máximo**, chama `setIA(make_unique<FugirStrategy>())`
- Uma vez ativada, a fuga não reverte (Goblin não volta a perseguir)
- Troll e Orc continuam perseguindo até a morte

```cpp
// src/ia/FugirStrategy.hpp
class FugirStrategy : public IAStrategy {
public:
    void agir(Enemy& self, Map& map, Player& player) override;
};
```

---

## Fluxo de turno com IAStrategy

```
Game::update()
    → player_.tickEfeitos()
    → player_.update()
    → for each enemy:
        → enemy->tickEfeitos()
        → enemy->update(map_, player_)   ← delega para ia_->agir()
```

Nada muda no `Game` — a interface permanece a mesma.

---

## Arquivos afetados

| Arquivo | Ação |
|---|---|
| `src/ia/IAStrategy.hpp` | CRIAR — interface pura |
| `src/ia/PerseguirStrategy.hpp/.cpp` | CRIAR — perseguição + callback onAtaque |
| `src/ia/FugirStrategy.hpp/.cpp` | CRIAR — fuga do player |
| `src/entities/Enemy.hpp` | EDITAR — + `ia_`, `setIA()`, `moverPara()` protegido; `update()` não-abstrato |
| `src/entities/Enemy.cpp` | EDITAR — `update()` delega para `ia_`; `moverPara()` implementado |
| `src/entities/Goblin.hpp/.cpp` | EDITAR — injeta `PerseguirStrategy` no construtor; sobrescreve `takeDamage()` para fuga; remove `update()` |
| `src/entities/Troll.hpp/.cpp` | EDITAR — injeta `PerseguirStrategy` no construtor; remove `update()` |
| `src/entities/Orc.hpp/.cpp` | CRIAR — novo inimigo |
| `src/entities/EnemyFactory.hpp/.cpp` | EDITAR — `EnemyType::Orc` + `tabelaSpawn` |

---

## Patterns praticados

- **Strategy** — `IAStrategy` / `PerseguirStrategy` / `FugirStrategy`
- **Open/Closed Principle** — Orc adicionado sem tocar em Goblin/Troll
- **Single Responsibility** — cada Strategy tem uma única responsabilidade de comportamento
- **Dependency Injection** — IA injetada no construtor via `setIA()`
