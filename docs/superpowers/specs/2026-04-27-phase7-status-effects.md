# Spec — Phase 7a: Status Effects

**Data:** 2026-04-27  
**Status:** Aprovada  

---

## Objetivo

Adicionar um sistema de efeitos de status temporários ao combate. Efeitos duram N turnos, são processados a cada turno e podem se acumular. Tanto o player quanto os inimigos podem receber efeitos.

---

## Decisões de design

### Interface `StatusEffect`

Interface pura em `src/effects/StatusEffect.hpp`:

```cpp
class StatusEffect {
public:
    virtual void tick(Entity& alvo) = 0;
    virtual bool isExpired() const = 0;
    virtual std::string getNome() const = 0;
    virtual ~StatusEffect() = default;
};
```

- `tick(Entity&)` — executado uma vez por turno; aplica o efeito e decrementa a duração restante
- `isExpired()` — retorna `true` quando `duracaoRestante_ <= 0`
- `getNome()` — usado pelo log de mensagens para descrever o efeito

### Efeitos concretos

| Classe | Arquivo | Efeito por tick | Parâmetros do construtor |
|---|---|---|---|
| `VenenoEffect` | `effects/VenenoEffect.hpp/.cpp` | `alvo.takeDamage(dano_)` | `int dano, int duracao` |
| `ParalisiaEffect` | `effects/ParalisiaEffect.hpp/.cpp` | seta flag `paralisado_` em `Entity` | `int duracao` |
| `RegeneracaoEffect` | `effects/RegeneracaoEffect.hpp/.cpp` | `alvo.curar(cura_)` | `int cura, int duracao` |
| `EnfraquecimentoEffect` | `effects/EnfraquecimentoEffect.hpp/.cpp` | reduz `attackBonus_` temporariamente | `int reducao, int duracao` |

Todos herdam de `StatusEffect` e mantêm `int duracaoRestante_` decrementado a cada `tick()`.

### Paralisia — flag em `Entity`

`Entity` ganha um campo `bool paralisado_` (default `false`). `ParalisiaEffect::tick()` seta `paralisado_ = true` a cada turno enquanto ativo; quando o efeito expira, `paralisado_` volta a `false` no próximo `tickEfeitos()`.

- `Enemy::update()` checa `isParalisado()` antes de agir — se `true`, pula o turno
- `Player::move()` checa `isParalisado()` antes de processar input — se `true`, registra mensagem no log e não move

### Lista de efeitos em `Entity`

```cpp
// Entity.hpp
std::vector<std::unique_ptr<StatusEffect>> efeitos_;

void adicionarEfeito(std::unique_ptr<StatusEffect> efeito);
void tickEfeitos();       // chama tick() em cada efeito e remove os expirados
bool isParalisado() const;
```

`tickEfeitos()` é chamado uma vez por turno, **antes** da ação da entidade, em `Game::update()`:

- Para o player: `player_.tickEfeitos()` antes de `player_.update()`
- Para cada inimigo: `enemy->tickEfeitos()` antes de `enemy->update()`

Ao remover efeitos expirados, `ParalisiaEffect` vencido também reseta `paralisado_ = false` via flag — ou `tickEfeitos()` reseta o campo após processar todos os efeitos.

### Fontes dos efeitos

**Inimigos aplicam efeitos ao atacar:**
- Goblin: aplica `VenenoEffect(2, 3)` ao atacar (2 de dano por turno, 3 turnos)
- Troll: aplica `ParalisiaEffect(2)` ao atacar (paralisia por 2 turnos)

**Itens aplicam efeitos ao usar:**
- Nova `ItemStrategy` concreta: `AplicarEfeitoStrategy` — recebe um `StatusEffect` e o adiciona ao player via `player.adicionarEfeito()`
- `ItemFactory` cria novos itens: `PocaoDeRegeneracao` (regen 5 HP por turno, 3 turnos) e `PocaoDeEnfraquecimento` (debuffa inimigo — tratado futuramente)

### HUD — exibir efeitos ativos

A linha de status do HUD exibe os nomes dos efeitos ativos do player:

```
HP: 30/50   ATK: 6   DEF: 2   [Veneno, Paralisia]
```

`Renderer::renderHUD()` recebe os efeitos ativos via `player.getEfeitos()`.

---

## Fluxo de turno com efeitos

```
Game::update()
    → player_.tickEfeitos()          // processa efeitos do player, remove expirados
    → player_.update()               // XP, etc.
    → for each enemy:
        → enemy->tickEfeitos()       // processa efeitos do inimigo
        → enemy->update(map, player) // só age se !isParalisado()
    → (restante do update existente)
```

---

## Arquivos afetados

| Arquivo | Mudança |
|---|---|
| `src/effects/StatusEffect.hpp` | nova interface pura |
| `src/effects/VenenoEffect.hpp/.cpp` | efeito concreto |
| `src/effects/ParalisiaEffect.hpp/.cpp` | efeito concreto |
| `src/effects/RegeneracaoEffect.hpp/.cpp` | efeito concreto |
| `src/effects/EnfraquecimentoEffect.hpp/.cpp` | efeito concreto |
| `src/entities/Entity.hpp/.cpp` | + `efeitos_`, `paralisado_`, `adicionarEfeito()`, `tickEfeitos()`, `isParalisado()`, `getEfeitos()` |
| `src/entities/Player.hpp/.cpp` | checa `isParalisado()` em `move()`; + `curar()` se não existir |
| `src/entities/Goblin.cpp` | aplica `VenenoEffect` ao atacar |
| `src/entities/Troll.cpp` | aplica `ParalisiaEffect` ao atacar |
| `src/items/AplicarEfeitoStrategy.hpp/.cpp` | nova ItemStrategy |
| `src/items/ItemFactory.cpp` | novos itens: `PocaoDeRegeneracao` |
| `src/core/Game.cpp` | `update()` chama `tickEfeitos()` antes de cada entidade agir |
| `src/ui/Renderer.cpp` | HUD exibe efeitos ativos do player |

---

## Patterns praticados nesta fase

- **Decorator** (espírito): cada `StatusEffect` "decora" o comportamento de turno da entidade — `VenenoEffect` envolve o tick com dano adicional, `ParalisiaEffect` suprime a ação
- **Strategy**: `AplicarEfeitoStrategy` é uma nova Strategy de item que delega para um efeito de status em vez de aplicar um valor fixo
- **Single Responsibility**: `tickEfeitos()` encapsula toda a lógica de processamento e remoção de efeitos, sem poluir `update()` das entidades
