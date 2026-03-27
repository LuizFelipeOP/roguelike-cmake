# Phase 5 Design Spec — Itens, Inventário e Design Patterns

**Data:** 2026-03-26
**Status:** Aprovado

---

## Contexto

O projeto é um roguelike clássico em C++ (inspirado em Rogue 1980), desenvolvido como exercício de aprendizado de SOLID, Design Patterns e Clean Code. A Phase 4 entregou XP/Níveis e Fog of War. A Phase 5 adiciona um sistema completo de itens com inventário, equipamentos, consumíveis e passivos, praticando os patterns Strategy, Observer e Factory.

---

## Escopo

Três sistemas integrados:

1. **Itens e Strategy** — cada item tem um comportamento de uso encapsulado numa Strategy
2. **Inventário** — slots limitados por tipo, gerenciado pelo jogador
3. **Observer** — stats recalculados ao equipar/desequipar; log notificado por eventos

---

## Sistema 1 — Item e ItemStrategy

### ItemStrategy — interface virtual pura

```cpp
class ItemStrategy {
public:
    virtual void usar(Player& player) = 0;
    virtual ~ItemStrategy() = default;
};
```

### Estratégias concretas

| Classe | Efeito |
|--------|--------|
| `CuraStrategy(int quantidade)` | Cura `quantidade` de HP do jogador |
| `AumentarATKStrategy` | Aumenta ATK permanentemente (+1) |
| `BonusDefStrategy` | Aumenta DEF permanentemente (+1) |

### Enums

```cpp
enum class ItemType {
    PocaoDeVida,
    PocaoDeForça,
    Espada,
    Armadura,
    Amuleto
};

enum class ItemSlot {
    Arma,
    Armadura,
    Acessorio,
    Consumivel
};
```

### Classe Item

```cpp
class Item {
    std::string nome_;
    ItemType tipo_;
    ItemSlot slot_;
    int x_, y_;   // posição no mapa (ou -1/-1 se no inventário)
    std::unique_ptr<ItemStrategy> strategy_;
public:
    Item(std::string nome, ItemType tipo, ItemSlot slot, int x, int y,
         std::unique_ptr<ItemStrategy> strategy);

    void usar(Player& player);          // delega para strategy_
    ItemSlot getSlot()        const;
    ItemType getTipo()        const;
    std::string getNome()     const;
    char getSymbol()          const;    // símbolo para renderização
    int getX()                const;
    int getY()                const;
    void setPosicao(int x, int y);      // -1,-1 quando no inventário
};
```

### Símbolos de renderização

| ItemSlot | Símbolo |
|----------|---------|
| Consumivel (poção) | `6` |
| Arma | `/` |
| Armadura | `]` |
| Acessorio | `"` |

---

## Sistema 2 — Inventário

### Slots

- **Arma:** 1 slot único (`unique_ptr<Item>` ou `nullptr`)
- **Armadura:** 1 slot único (`unique_ptr<Item>` ou `nullptr`)
- **Acessorio:** 1 slot único (`unique_ptr<Item>` ou `nullptr`)
- **Consumíveis:** até 5 slots (`vector<unique_ptr<Item>>`, máximo 5)

### Classe Inventario

```cpp
class Inventario {
public:
    bool adicionarItem(std::unique_ptr<Item> item);  // false se slot cheio
    void equipar(ItemSlot slot, int index);           // move consumível → equip
    void desequipar(ItemSlot slot);                   // remove equip, libera slot
    std::unique_ptr<Item> removerConsumivel(int index); // remove e retorna
    Item* getEquipado(ItemSlot slot)                  const;
    const std::vector<std::unique_ptr<Item>>& getConsumiveis() const;
    bool cheio(ItemSlot slot)                         const;

private:
    std::unique_ptr<Item> arma_;
    std::unique_ptr<Item> armadura_;
    std::unique_ptr<Item> acessorio_;
    std::vector<std::unique_ptr<Item>> consumiveis_;  // máx 5
};
```

### Regras de slot

- Tentar adicionar item de equip com slot ocupado → retorna `false` (item fica no chão)
- Tentar adicionar consumível com 5 slots cheios → retorna `false`
- Desequipar libera o slot; o item retorna para o vetor de consumíveis. Se os consumíveis estiverem cheios, o item é descartado e o `LogObserver` emite: `"Inventario cheio! Item descartado."`
- `equipar(ItemSlot slot, int index)` valida o index antes de agir: index fora do range ou slot do item incompatível com o slot destino → operação ignorada silenciosamente, sem quebrar o código

---

## Sistema 3 — Observer

### Interface base

```cpp
class Observer {
public:
    virtual void onNotify(Player& player) = 0;
    virtual ~Observer() = default;
};
```

### Observer 1 — StatsObserver

Recalcula ATK e DEF do jogador baseado nos equipamentos atuais do inventário.
Chamado sempre que o jogador equipa ou desequipa um item.

```cpp
class StatsObserver : public Observer {
    Inventario& inventario_;
public:
    StatsObserver(Inventario& inv);
    void onNotify(Player& player) override;
    // zera bônus de equip, soma bônus dos itens atualmente equipados
};
```

### Observer 2 — LogObserver

Recebe eventos como strings e os empurra no `messageLog_` do Game.
Interface separada de `onNotify` para suportar mensagens arbitrárias.
`onNotify(Player&)` é implementado como no-op (corpo vazio) para satisfazer o contrato da interface.

```cpp
class LogObserver : public Observer {
    std::deque<std::string>& log_;
public:
    LogObserver(std::deque<std::string>& log);
    void onNotify(Player& player) override {}  // no-op, não usado diretamente
    void onEvent(const std::string& mensagem); // empurra msg no log
};
```

### Player como Subject

Novos membros em `Player`:

```cpp
std::vector<Observer*> observers_;

void adicionarObserver(Observer* obs);
void notificarObservers();   // chama onNotify em todos os observers
```

`notificarObservers()` é chamado em `equipar()` e `desequipar()` do inventário (via `Player`).

---

## Sistema 4 — ItemFactory

```cpp
class ItemFactory {
public:
    static std::unique_ptr<Item> create(ItemType tipo, int x, int y);
};
```

### Mapeamento tipo → configuração

| ItemType | Nome | Slot | Strategy | Símbolo |
|----------|------|------|----------|---------|
| `PocaoDeVida` | "Pocao de Vida" | Consumivel | `CuraStrategy(20)` | `6` |
| `PocaoDeForça` | "Pocao de Forca" | Consumivel | `AumentarATKStrategy` | `6` |
| `Espada` | "Espada" | Arma | `AumentarATKStrategy` | `/` |
| `Armadura` | "Armadura" | Armadura | `BonusDefStrategy` | `]` |
| `Amuleto` | "Amuleto" | Acessorio | `BonusDefStrategy` | `"` |

---

## Spawn de itens no mapa

- `Game` mantém `std::vector<std::unique_ptr<Item>> items_`
- Na inicialização, para cada sala (exceto a sala do jogador): **70% de chance** de spawnar um item
- Posição aleatória dentro da sala (não pode ser parede)
- **Distribuição de tipos:** os tipos são embaralhados com `std::shuffle` antes do spawn, garantindo variedade e evitando que todos os itens sejam do mesmo tipo. A distribuição não é uniforme garantida, mas o shuffle previne repetição pura.

## Tecla `U` — comportamento detalhado

- **No mapa (fora do inventário):** se o jogador está sobre um item consumível no chão, `U` usa o item diretamente (`item->usar(player)`), remove o item do mapa e emite mensagem no log. Se não houver item consumível no tile, nada acontece.
- **No inventário aberto:** `U` usa o primeiro consumível da lista (`consumiveis_[0]->usar(player)`), remove o item do inventário e emite mensagem no log.
- Após uso, o item é destruído (`unique_ptr` sai de escopo).

---

## Controles

| Tecla | Ação |
|-------|------|
| `G` | Pega item no tile atual do jogador |
| `U` | Usa o primeiro consumível do inventário diretamente |
| `I` | Abre/fecha tela de inventário |
| `1`-`5` | Dentro do inventário: seleciona consumível pelo índice |
| `E` | Dentro do inventário: equipa item selecionado |
| `X` | Dentro do inventário: desequipa slot selecionado |
| `Q` / `ESC` | Fecha inventário (sem ESC no Windows via _getch, usar `i` como toggle) |

---

## Tela de Inventário

Renderizada no lugar do HUD quando `inventarioAberto_` é `true` em `Game`:

```
----------------------------------------
 INVENTARIO
----------------------------------------
 Equipados:
   Arma:      Espada (+1 ATK)
   Armadura:  Armadura (+1 DEF)
   Acessorio: --

 Consumiveis:
   [1] Pocao de Vida
   [2] Pocao de Forca
   [3] --
   [4] --
   [5] --
----------------------------------------
 [1-5] Usar  [E] Equipar  [X] Desequipar  [I] Fechar
----------------------------------------
```

---

## Mudanças em `Player`

- Adicionar campo `Inventario inventario_`
- Adicionar campo `int attackBonus_` e `int defenseBonus_` — bônus de equipamento, separados dos stats base
- `getAttack()` retorna `attack_ + attackBonus_`
- `getDefense()` retorna `defense_ + defenseBonus_`
- Adicionar `adicionarObserver()`, `notificarObservers()`
- Adicionar `curar(int quantidade)` — cura HP sem ultrapassar `maxHp_`

---

## Mudanças em `Game`

- Adicionar `std::vector<std::unique_ptr<Item>> items_` — itens no chão
- Adicionar `bool inventarioAberto_` — controla renderização
- `processInput()`: tratar teclas `G`, `U`, `I`, `1`-`5`, `E`, `X`
- `update()`: verificar se jogador está no mesmo tile que um item (para `G`)
- `render()`: passa `items_` para o `Renderer`

---

## Mudanças em `Renderer`

- `render()` recebe `items_` e desenha no mapa os que têm `isExplored` no tile
- `renderHUD()` verifica `inventarioAberto_`: se true, chama `renderInventario()` em vez do HUD normal
- Novo método `renderInventario(const Inventario& inv)`

---

## Arquitetura — Resumo de arquivos novos e modificados

| Arquivo | Status |
|---------|--------|
| `src/items/ItemStrategy.hpp` | Novo |
| `src/items/CuraStrategy.hpp / .cpp` | Novo |
| `src/items/AumentarATKStrategy.hpp / .cpp` | Novo |
| `src/items/BonusDefStrategy.hpp / .cpp` | Novo |
| `src/items/Item.hpp / .cpp` | Novo |
| `src/items/Inventario.hpp / .cpp` | Novo |
| `src/items/ItemFactory.hpp / .cpp` | Novo |
| `src/observers/Observer.hpp` | Novo |
| `src/observers/StatsObserver.hpp / .cpp` | Novo |
| `src/observers/LogObserver.hpp / .cpp` | Novo |
| `src/entities/Player.hpp / .cpp` | Modificado |
| `src/core/Game.hpp / .cpp` | Modificado |
| `src/ui/Renderer.hpp / .cpp` | Modificado |

---

## Conceitos de C++ praticados

- `std::unique_ptr` para ownership de itens no inventário e no mapa
- Herança + polimorfismo (`ItemStrategy`, `Observer`)
- Método virtual puro como contrato de interface
- Separação de dados e comportamento (Item vs Strategy)
- Composição: `Player` tem `Inventario`, `Inventario` tem `Item`s

---

## O que NÃO está no escopo desta fase

- Itens com efeitos temporários (veneno, buffs por turnos)
- Sistema de raridade
- Loja ou troca de itens
- IA avançada com A* (Phase 6)
- Persistência/save (Phase 7)
