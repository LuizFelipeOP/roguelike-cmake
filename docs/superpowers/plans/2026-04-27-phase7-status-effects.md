# Plano de Implementação — Phase 7a: Status Effects

**Data:** 2026-04-27  
**Spec:** `docs/superpowers/specs/2026-04-27-phase7-status-effects.md`  
**Status:** Pendente  

---

## Visão geral

Implementar em 5 chunks sequenciais. Cada chunk compila antes de avançar.

---

## Chunk 1 — Interface e efeitos concretos em `src/effects/`

**Conceito:** Criar a pasta `src/effects/` com a interface `StatusEffect` e as quatro implementações concretas. Neste chunk nenhum código existente é alterado — só arquivos novos.

### `src/effects/StatusEffect.hpp`
1. Criar interface pura com três métodos virtuais:
   - `virtual void tick(Entity& alvo) = 0`
   - `virtual bool isExpired() const = 0`
   - `virtual std::string getNome() const = 0`
   - `virtual ~StatusEffect() = default`
   - Forward declare `Entity` para evitar include circular

### `src/effects/VenenoEffect.hpp/.cpp`
2. Construtor: `VenenoEffect(int dano, int duracao)`
3. `tick()`: chama `alvo.takeDamage(dano_)` e decrementa `duracaoRestante_`
4. `isExpired()`: retorna `duracaoRestante_ <= 0`
5. `getNome()`: retorna `"Veneno"`

### `src/effects/ParalisiaEffect.hpp/.cpp`
6. Construtor: `ParalisiaEffect(int duracao)`
7. `tick()`: chama `alvo.setParalisado(true)` e decrementa `duracaoRestante_`
8. `isExpired()`: retorna `duracaoRestante_ <= 0`
9. `getNome()`: retorna `"Paralisia"`

### `src/effects/RegeneracaoEffect.hpp/.cpp`
10. Construtor: `RegeneracaoEffect(int cura, int duracao)`
11. `tick()`: chama `alvo.curar(cura_)` e decrementa `duracaoRestante_`
12. `isExpired()`: retorna `duracaoRestante_ <= 0`
13. `getNome()`: retorna `"Regeneracao"`

### `src/effects/EnfraquecimentoEffect.hpp/.cpp`
14. Construtor: `EnfraquecimentoEffect(int reducao, int duracao)`
15. `tick()`: chama `alvo.reduzirAtaqueBonus(reducao_)` e decrementa `duracaoRestante_`
16. `isExpired()`: retorna `duracaoRestante_ <= 0`
17. `getNome()`: retorna `"Enfraquecimento"`

> **Atenção:** `Entity` precisa expor `takeDamage()`, `curar()`, `setParalisado()` e `reduzirAtaqueBonus()` — alguns podem não existir ainda. O Chunk 2 os adiciona. Por ora basta declarar os headers com forward declare.

**Teste:** `cmake -S . -B build && cmake --build build` — deve compilar sem erros.

---

## Chunk 2 — Suporte em `Entity`

**Conceito:** `Entity` passa a carregar a lista de efeitos ativos e o flag de paralisia. É a única mudança estrutural no código existente.

### `src/entities/Entity.hpp`
1. Adicionar include: `#include "effects/StatusEffect.hpp"` e `#include <vector>` e `#include <memory>`
2. Adicionar campos privados:
   ```cpp
   std::vector<std::unique_ptr<StatusEffect>> efeitos_;
   bool paralisado_ = false;
   ```
3. Declarar métodos públicos:
   ```cpp
   void adicionarEfeito(std::unique_ptr<StatusEffect> efeito);
   void tickEfeitos();
   bool isParalisado() const;
   void setParalisado(bool valor);
   const std::vector<std::unique_ptr<StatusEffect>>& getEfeitos() const;
   ```
4. Se `curar()` ainda não existir em `Entity`, declarar:
   ```cpp
   virtual void curar(int quantidade) {}  // default vazio — Player sobrescreve
   ```
5. Declarar `reduzirAtaqueBonus(int reducao)` — default vazio, Player sobrescreve

### `src/entities/Entity.cpp`
6. Implementar `adicionarEfeito()`: `efeitos_.push_back(std::move(efeito))`
7. Implementar `tickEfeitos()`:
   - Iterar `efeitos_`, chamar `tick(*this)` em cada um
   - Após o loop, remover os expirados com `std::remove_if` + `erase`
   - Após remover, se nenhum `ParalisiaEffect` estiver ativo, setar `paralisado_ = false`
8. Implementar `isParalisado()`, `setParalisado()`, `getEfeitos()`

> **Atenção:** `std::remove_if` com `unique_ptr` requer lambda com `const auto&` e `->isExpired()`.

**Teste:** buildar — sem erros. Comportamento do jogo inalterado.

---

## Chunk 3 — Inimigos aplicam efeitos ao atacar

**Conceito:** Goblin e Troll passam a chamar `player.adicionarEfeito(...)` no momento do ataque, após o `takeDamage`. O tipo de efeito e parâmetros seguem a spec.

### `src/entities/Goblin.cpp`
1. Adicionar include: `#include "effects/VenenoEffect.hpp"`
2. Em `update()`, após `player.takeDamage(getAttack())`, adicionar:
   ```cpp
   player.adicionarEfeito(std::make_unique<VenenoEffect>(2, 3));
   ```

### `src/entities/Troll.cpp`
3. Adicionar include: `#include "effects/ParalisiaEffect.hpp"`
4. Em `update()`, após `player.takeDamage(getAttack())`, adicionar:
   ```cpp
   player.adicionarEfeito(std::make_unique<ParalisiaEffect>(2));
   ```

### `src/core/Game.cpp` — `update()`
5. Adicionar chamadas a `tickEfeitos()` antes das ações de cada entidade:
   - Antes de `player_.update()`: `player_.tickEfeitos()`
   - Dentro do loop de inimigos, antes de `enemy->update(map_, player_)`: `enemy->tickEfeitos()`

### `src/entities/Player.cpp` — `move()`
6. No início de `move()`, checar paralisia:
   ```cpp
   if (isParalisado()) {
       // registrar no log via Game — por enquanto apenas retornar sem mover
       return;
   }
   ```
   > A mensagem de log será integrada no Chunk 5 junto com o HUD.

### `src/entities/Goblin.cpp` e `Troll.cpp` — `update()`
7. Checar `isParalisado()` no início de `update()`:
   ```cpp
   if (isParalisado()) return;
   ```

**Teste:** rodar o jogo. Ser atacado por Goblin deve acumular efeito de Veneno. Ser atacado por Troll deve paralisar por 2 turnos (player não move). Nenhum crash.

---

## Chunk 4 — Item com `AplicarEfeitoStrategy` e `PocaoDeRegeneracao`

**Conceito:** Nova Strategy de item que aplica um `StatusEffect` ao player em vez de modificar stats diretamente.

### `src/items/AplicarEfeitoStrategy.hpp`
1. Criar classe que herda de `ItemStrategy`:
   ```cpp
   class AplicarEfeitoStrategy : public ItemStrategy {
   public:
       // Recebe uma factory function para criar o efeito a cada uso
       using EfeitoFactory = std::function<std::unique_ptr<StatusEffect>()>;
       explicit AplicarEfeitoStrategy(EfeitoFactory factory);
       void usar(Player& player) override;
   private:
       EfeitoFactory factory_;
   };
   ```
   > Usar `std::function` como factory evita o problema de `unique_ptr` não ser copiável.

### `src/items/AplicarEfeitoStrategy.cpp`
2. `usar()`: `player.adicionarEfeito(factory_())`

### `src/items/ItemFactory.cpp`
3. Adicionar includes dos efeitos necessários
4. Adicionar `PocaoDeRegeneracao` ao pool de itens disponíveis (em andares 1+):
   ```cpp
   // ItemType::PocaoDeRegeneracao
   return std::make_unique<Item>("Pocao de Regeneracao", ItemType::PocaoDeRegeneracao,
       ItemSlot::Consumivel, x, y,
       std::make_unique<AplicarEfeitoStrategy>([](){
           return std::make_unique<RegeneracaoEffect>(5, 3);
       })
   );
   ```

### `src/items/ItemType` (enum)
5. Adicionar `PocaoDeRegeneracao` ao enum `ItemType` em `Item.hpp`

**Teste:** coletar e usar `PocaoDeRegeneracao` deve ativar `RegeneracaoEffect` — HP deve subir 5 por turno por 3 turnos.

---

## Chunk 5 — HUD exibe efeitos ativos + mensagens de log

**Conceito:** O HUD mostra os efeitos ativos do player entre colchetes. O log registra quando um efeito começa e quando expira.

### `src/ui/Renderer.cpp` — `renderHUD()`
1. Após a linha de HP/ATK/DEF, adicionar linha de efeitos:
   ```cpp
   const auto& efeitos = player.getEfeitos();
   if (!efeitos.empty()) {
       std::cout << " Efeitos: ";
       for (const auto& e : efeitos) std::cout << "[" << e->getNome() << "] ";
       std::cout << '\n';
   }
   ```

### `src/core/Game.cpp` — `update()`
2. Antes de chamar `player_.tickEfeitos()`, guardar snapshot dos nomes ativos
3. Após `tickEfeitos()`, comparar com o novo estado — efeitos que sumiram geram log `"[NomeEfeito] acabou"`
4. Em `Goblin::update()` e `Troll::update()`, após `adicionarEfeito`, o log pode ser emitido por `Game` verificando se o player tem o efeito novo
   > Alternativa mais simples: `adicionarEfeito()` em `Entity` aceita callback opcional de log — ou simplesmente logar direto em `Game::update()` após o tick dos inimigos

### `src/entities/Player.cpp` — `move()` (paralisia)
5. Atualizar o `return` de paralisia para também logar:
   - Como `Player` não tem acesso ao log diretamente, o log pode ser emitido em `Game::update()` verificando `player_.isParalisado()` antes de processar input
   - Ou via `Observer` — `notificarObservers()` já existe

**Teste final:** buildar completo. HUD exibe `[Veneno]` ao ser envenenado, desaparece após 3 turnos. Mensagem de log ao expirar. Pocao de Regeneracao visível no inventário e funcional.

---

## Checklist de conclusão

- [ ] Chunk 1: 4 efeitos compilam sem erros
- [ ] Chunk 2: `Entity` com lista de efeitos, jogo comportamento inalterado
- [ ] Chunk 3: Goblin/Troll aplicam efeitos; paralisia impede movimento
- [ ] Chunk 4: `PocaoDeRegeneracao` coletável e funcional
- [ ] Chunk 5: HUD mostra efeitos ativos; log registra início/fim
- [ ] Nenhum crash ao acumular múltiplos efeitos simultâneos
