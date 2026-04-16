# Phase 5 — Itens, Inventário e Design Patterns — Plano de Implementação

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implementar um sistema completo de itens com inventário, equipamentos, consumíveis e passivos, praticando os patterns Strategy, Observer e Factory.

**Architecture:** `Item` carrega dados e delega comportamento para `ItemStrategy` (Strategy pattern). `Player` é Subject que notifica `StatsObserver` e `LogObserver` ao equipar/desequipar (Observer pattern). `ItemFactory` centraliza a criação de itens (Factory pattern). `Inventario` gerencia slots limitados por tipo.

**Tech Stack:** C++17, CMake, Visual Studio 2022, `std::unique_ptr`, herança/polimorfismo, `std::shuffle`

**Spec:** `docs/superpowers/specs/2026-03-26-phase5-itens-design.md`

---

## Chunk 1: Strategy — ItemStrategy, Item, Strategies concretas

### Task 1: ItemStrategy e enums

**Files:**
- Create: `src/items/ItemStrategy.hpp`
- Create: `src/items/Item.hpp` (enums apenas)

- [ ] **Criar `src/items/ItemStrategy.hpp`**

```cpp
#pragma once

class Player;  // forward declaration

class ItemStrategy {
public:
    virtual void usar(Player& player) = 0;
    virtual ~ItemStrategy() = default;
};
```

- [ ] **Criar `src/items/Item.hpp` — só os enums por enquanto**

```cpp
#pragma once

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

- [ ] **Verificar que o projeto ainda compila**

No VS Code: `CMake: Build` ou `Ctrl+Shift+B`
Esperado: build sem erros (arquivos novos ainda não fazem nada)

---

### Task 2: Strategies concretas

**Files:**
- Create: `src/items/CuraStrategy.hpp`
- Create: `src/items/CuraStrategy.cpp`
- Create: `src/items/AumentarATKStrategy.hpp`
- Create: `src/items/AumentarATKStrategy.cpp`
- Create: `src/items/BonusDefStrategy.hpp`
- Create: `src/items/BonusDefStrategy.cpp`

- [ ] **Criar `src/items/CuraStrategy.hpp`**

```cpp
#pragma once
#include "ItemStrategy.hpp"

class CuraStrategy : public ItemStrategy {
    int quantidade_;
public:
    CuraStrategy(int quantidade);
    void usar(Player& player) override;
};
```

- [ ] **Criar `src/items/CuraStrategy.cpp`**

```cpp
#include "CuraStrategy.hpp"
#include "entities/Player.hpp"

CuraStrategy::CuraStrategy(int quantidade) : quantidade_(quantidade) {}

void CuraStrategy::usar(Player& player) {
    player.curar(quantidade_);
}
```

- [ ] **Criar `src/items/AumentarATKStrategy.hpp`**

```cpp
#pragma once
#include "ItemStrategy.hpp"

class AumentarATKStrategy : public ItemStrategy {
public:
    void usar(Player& player) override;
};
```

- [ ] **Criar `src/items/AumentarATKStrategy.cpp`**

```cpp
#include "AumentarATKStrategy.hpp"
#include "entities/Player.hpp"

void AumentarATKStrategy::usar(Player& player) {
    player.raiseAttack();
}
```

- [ ] **Criar `src/items/BonusDefStrategy.hpp`**

```cpp
#pragma once
#include "ItemStrategy.hpp"

class BonusDefStrategy : public ItemStrategy {
public:
    void usar(Player& player) override;
};
```

- [ ] **Criar `src/items/BonusDefStrategy.cpp`**

```cpp
#include "BonusDefStrategy.hpp"
#include "entities/Player.hpp"

void BonusDefStrategy::usar(Player& player) {
    player.raiseDefense();
}
```

- [ ] **Verificar que o projeto compila**

Esperado: build sem erros

---

### Task 3: Classe Item completa

**Files:**
- Modify: `src/items/Item.hpp` (adicionar classe Item)
- Create: `src/items/Item.cpp`

- [ ] **Atualizar `src/items/Item.hpp` — adicionar classe Item**

```cpp
#pragma once
#include <string>
#include <memory>
#include "ItemStrategy.hpp"

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

class Player;  // forward declaration

class Item {
public:
    Item(std::string nome, ItemType tipo, ItemSlot slot, int x, int y,
         std::unique_ptr<ItemStrategy> strategy);

    void usar(Player& player);

    ItemSlot    getSlot()   const;
    ItemType    getTipo()   const;
    std::string getNome()   const;
    char        getSymbol() const;
    int         getX()      const;
    int         getY()      const;
    void        setPosicao(int x, int y);

private:
    std::string nome_;
    ItemType    tipo_;
    ItemSlot    slot_;
    int x_, y_;
    std::unique_ptr<ItemStrategy> strategy_;
};
```

- [ ] **Criar `src/items/Item.cpp`**

```cpp
#include "Item.hpp"
#include "entities/Player.hpp"

Item::Item(std::string nome, ItemType tipo, ItemSlot slot, int x, int y,
           std::unique_ptr<ItemStrategy> strategy)
    : nome_(std::move(nome))
    , tipo_(tipo)
    , slot_(slot)
    , x_(x)
    , y_(y)
    , strategy_(std::move(strategy))
{}

void Item::usar(Player& player) {
    if (strategy_) strategy_->usar(player);
}

ItemSlot    Item::getSlot()   const { return slot_; }
ItemType    Item::getTipo()   const { return tipo_; }
std::string Item::getNome()   const { return nome_; }
int         Item::getX()      const { return x_; }
int         Item::getY()      const { return y_; }

void Item::setPosicao(int x, int y) { x_ = x; y_ = y; }

char Item::getSymbol() const {
    switch (slot_) {
        case ItemSlot::Arma:      return '/';
        case ItemSlot::Armadura:  return ']';
        case ItemSlot::Acessorio: return '"';
        default:                  return '6'; // Consumivel
    }
}
```

- [ ] **Verificar que o projeto compila**

Esperado: build sem erros

- [ ] **Commit do Chunk 1**

```
git add src/items/
git commit -m "feat: add Item, ItemStrategy and concrete strategies (Strategy pattern)"
```

---

## Chunk 2: Observer — Observer, StatsObserver, LogObserver

### Task 4: Interface Observer

**Files:**
- Create: `src/observers/Observer.hpp`

- [ ] **Criar `src/observers/Observer.hpp`**

```cpp
#pragma once

class Player;  // forward declaration

class Observer {
public:
    virtual void onNotify(Player& player) = 0;
    virtual ~Observer() = default;
};
```

- [ ] **Verificar que o projeto compila**

---

### Task 5: StatsObserver

**Files:**
- Create: `src/observers/StatsObserver.hpp`
- Create: `src/observers/StatsObserver.cpp`

> Nota: `StatsObserver` recebe uma referência ao `Inventario` e recalcula `attackBonus_` e `defenseBonus_` do Player. Esses campos serão adicionados ao `Player` na Task 8.

- [ ] **Criar `src/observers/StatsObserver.hpp`**

```cpp
#pragma once
#include "Observer.hpp"

class Inventario;  // forward declaration

class StatsObserver : public Observer {
    Inventario& inventario_;
public:
    StatsObserver(Inventario& inv);
    void onNotify(Player& player) override;
};
```

- [ ] **Criar `src/observers/StatsObserver.cpp`**

```cpp
#include "StatsObserver.hpp"
#include "items/Inventario.hpp"
#include "entities/Player.hpp"

StatsObserver::StatsObserver(Inventario& inv) : inventario_(inv) {}

void StatsObserver::onNotify(Player& player) {
    // Zera bônus e recalcula baseado nos itens equipados
    int atkBonus = 0;
    int defBonus = 0;

    // Arma equipada contribui com ATK
    if (inventario_.getEquipado(ItemSlot::Arma)) {
        atkBonus += 1;
    }
    // Armadura equipada contribui com DEF
    if (inventario_.getEquipado(ItemSlot::Armadura)) {
        defBonus += 1;
    }
    // Acessório equipado contribui com DEF
    if (inventario_.getEquipado(ItemSlot::Acessorio)) {
        defBonus += 1;
    }

    player.setAttackBonus(atkBonus);
    player.setDefenseBonus(defBonus);
}
```

- [ ] **Verificar que o projeto compila**

---

### Task 6: LogObserver

**Files:**
- Create: `src/observers/LogObserver.hpp`
- Create: `src/observers/LogObserver.cpp`

- [ ] **Criar `src/observers/LogObserver.hpp`**

```cpp
#pragma once
#include "Observer.hpp"
#include <deque>
#include <string>

class LogObserver : public Observer {
    std::deque<std::string>& log_;
public:
    LogObserver(std::deque<std::string>& log);
    void onNotify(Player& player) override {}  // no-op
    void onEvent(const std::string& mensagem);
};
```

- [ ] **Criar `src/observers/LogObserver.cpp`**

```cpp
#include "LogObserver.hpp"

LogObserver::LogObserver(std::deque<std::string>& log) : log_(log) {}

void LogObserver::onEvent(const std::string& mensagem) {
    if (log_.size() >= 3) log_.pop_front();
    log_.push_back(mensagem);
}
```

- [ ] **Verificar que o projeto compila**

- [ ] **Commit do Chunk 2**

```
git add src/observers/
git commit -m "feat: add Observer interface, StatsObserver and LogObserver (Observer pattern)"
```

---

## Chunk 3: Inventário

### Task 7: Classe Inventario

**Files:**
- Create: `src/items/Inventario.hpp`
- Create: `src/items/Inventario.cpp`

- [ ] **Criar `src/items/Inventario.hpp`**

```cpp
#pragma once
#include <vector>
#include <memory>
#include "Item.hpp"

class Inventario {
public:
    // Adiciona item ao slot correto. Retorna false se slot cheio.
    bool adicionarItem(std::unique_ptr<Item> item);

    // Move consumível do índice dado para o slot de equip correspondente.
    // Valida índice e compatibilidade de slot — ignora silenciosamente se inválido.
    void equipar(ItemSlot slot, int index);

    // Remove item do slot de equip.
    // Se consumíveis cheios, item é descartado (chama logObserver via callback).
    void desequipar(ItemSlot slot);

    // Remove e retorna consumível do índice dado. Retorna nullptr se inválido.
    std::unique_ptr<Item> removerConsumivel(int index);

    Item* getEquipado(ItemSlot slot) const;
    const std::vector<std::unique_ptr<Item>>& getConsumiveis() const;
    bool cheio(ItemSlot slot) const;

    // Callback chamado quando item é descartado por inventário cheio
    // Deve ser conectado ao LogObserver pelo Game após construção
    std::function<void(const std::string&)> onDescarte;

private:
    std::unique_ptr<Item> arma_;
    std::unique_ptr<Item> armadura_;
    std::unique_ptr<Item> acessorio_;
    std::vector<std::unique_ptr<Item>> consumiveis_;  // máx 5

    std::unique_ptr<Item>& slotRef(ItemSlot slot);
};
```

> Nota: `#include <functional>` necessário para `std::function`. Adicione ao `.cpp`.

- [ ] **Criar `src/items/Inventario.cpp`**

```cpp
#include "Inventario.hpp"
#include <functional>

bool Inventario::adicionarItem(std::unique_ptr<Item> item) {
    ItemSlot slot = item->getSlot();

    if (slot == ItemSlot::Consumivel) {
        if (consumiveis_.size() >= 5) return false;
        consumiveis_.push_back(std::move(item));
        return true;
    }

    // Equip — verifica se slot está livre
    if (cheio(slot)) return false;
    slotRef(slot) = std::move(item);
    return true;
}

void Inventario::equipar(ItemSlot slot, int index) {
    // Valida index
    if (index < 0 || index >= static_cast<int>(consumiveis_.size())) return;

    // Valida que o item no índice é compatível com o slot destino
    if (consumiveis_[index]->getSlot() != slot) return;

    // Verifica se slot de equip está livre
    if (cheio(slot)) return;

    slotRef(slot) = std::move(consumiveis_[index]);
    consumiveis_.erase(consumiveis_.begin() + index);
}

void Inventario::desequipar(ItemSlot slot) {
    auto& s = slotRef(slot);
    if (!s) return;

    if (consumiveis_.size() < 5) {
        consumiveis_.push_back(std::move(s));
    } else {
        // Inventário cheio — descarta e notifica
        if (onDescarte) onDescarte("Inventario cheio! Item descartado.");
        s.reset();
    }
}

std::unique_ptr<Item> Inventario::removerConsumivel(int index) {
    if (index < 0 || index >= static_cast<int>(consumiveis_.size())) return nullptr;
    auto item = std::move(consumiveis_[index]);
    consumiveis_.erase(consumiveis_.begin() + index);
    return item;
}

Item* Inventario::getEquipado(ItemSlot slot) const {
    switch (slot) {
        case ItemSlot::Arma:      return arma_.get();
        case ItemSlot::Armadura:  return armadura_.get();
        case ItemSlot::Acessorio: return acessorio_.get();
        default: return nullptr;
    }
}

const std::vector<std::unique_ptr<Item>>& Inventario::getConsumiveis() const {
    return consumiveis_;
}

bool Inventario::cheio(ItemSlot slot) const {
    switch (slot) {
        case ItemSlot::Arma:      return arma_     != nullptr;
        case ItemSlot::Armadura:  return armadura_ != nullptr;
        case ItemSlot::Acessorio: return acessorio_!= nullptr;
        case ItemSlot::Consumivel: return consumiveis_.size() >= 5;
        default: return false;
    }
}

std::unique_ptr<Item>& Inventario::slotRef(ItemSlot slot) {
    switch (slot) {
        case ItemSlot::Arma:      return arma_;
        case ItemSlot::Armadura:  return armadura_;
        case ItemSlot::Acessorio: return acessorio_;
        default:                  return arma_;  // fallback seguro
    }
}
```

- [ ] **Verificar que o projeto compila**

- [ ] **Commit do Chunk 3**

```
git add src/items/Inventario.hpp src/items/Inventario.cpp
git commit -m "feat: add Inventario with slot management"
```

---

## Chunk 4: ItemFactory

### Task 8: ItemFactory

**Files:**
- Create: `src/items/ItemFactory.hpp`
- Create: `src/items/ItemFactory.cpp`

- [ ] **Criar `src/items/ItemFactory.hpp`**

```cpp
#pragma once
#include <memory>
#include "Item.hpp"

class ItemFactory {
public:
    static std::unique_ptr<Item> create(ItemType tipo, int x, int y);
};
```

- [ ] **Criar `src/items/ItemFactory.cpp`**

```cpp
#include "ItemFactory.hpp"
#include "CuraStrategy.hpp"
#include "AumentarATKStrategy.hpp"
#include "BonusDefStrategy.hpp"

std::unique_ptr<Item> ItemFactory::create(ItemType tipo, int x, int y) {
    switch (tipo) {
        case ItemType::PocaoDeVida:
            return std::make_unique<Item>(
                "Pocao de Vida", tipo, ItemSlot::Consumivel, x, y,
                std::make_unique<CuraStrategy>(20));

        case ItemType::PocaoDeForça:
            return std::make_unique<Item>(
                "Pocao de Forca", tipo, ItemSlot::Consumivel, x, y,
                std::make_unique<AumentarATKStrategy>());

        case ItemType::Espada:
            return std::make_unique<Item>(
                "Espada", tipo, ItemSlot::Arma, x, y,
                std::make_unique<AumentarATKStrategy>());

        case ItemType::Armadura:
            return std::make_unique<Item>(
                "Armadura", tipo, ItemSlot::Armadura, x, y,
                std::make_unique<BonusDefStrategy>());

        case ItemType::Amuleto:
            return std::make_unique<Item>(
                "Amuleto", tipo, ItemSlot::Acessorio, x, y,
                std::make_unique<BonusDefStrategy>());

        default:
            return nullptr;
    }
}
```

- [ ] **Verificar que o projeto compila**

- [ ] **Commit do Chunk 4**

```
git add src/items/ItemFactory.hpp src/items/ItemFactory.cpp
git commit -m "feat: add ItemFactory (Factory pattern)"
```

---

## Chunk 5: Modificações em Player

### Task 9: Atualizar Player

**Files:**
- Modify: `src/entities/Player.hpp`
- Modify: `src/entities/Player.cpp`

- [ ] **Adicionar em `Player.hpp` — novos campos e métodos**

Adicionar nos campos privados:
```cpp
int attackBonus_;    // bônus de equipamento — atualizado pelo StatsObserver
int defenseBonus_;   // bônus de equipamento — atualizado pelo StatsObserver
std::vector<Observer*> observers_;
```

Adicionar nos includes do `.hpp`:
```cpp
#include "observers/Observer.hpp"
#include "items/Inventario.hpp"
```

Adicionar nos métodos públicos:
```cpp
void curar(int quantidade);           // cura sem ultrapassar maxHp_
void setAttackBonus(int bonus);
void setDefenseBonus(int bonus);
void adicionarObserver(Observer* obs);
void notificarObservers();
Inventario& getInventario();
```

- [ ] **Atualizar `getAttack()` e `getDefense()` em `Player.cpp`**

```cpp
int Player::getAttack()  const { return attack_  + attackBonus_; }
int Player::getDefense() const { return defense_ + defenseBonus_; }
```

- [ ] **Implementar novos métodos em `Player.cpp`**

```cpp
void Player::curar(int quantidade) {
    hp_ = std::min(hp_ + quantidade, maxHp_);
}

void Player::setAttackBonus(int bonus)  { attackBonus_  = bonus; }
void Player::setDefenseBonus(int bonus) { defenseBonus_ = bonus; }

void Player::adicionarObserver(Observer* obs) {
    observers_.push_back(obs);
}

void Player::notificarObservers() {
    for (auto* obs : observers_) obs->onNotify(*this);
}

Inventario& Player::getInventario() { return inventario_; }
```

- [ ] **Inicializar novos campos no construtor de `Player`**

```cpp
, attackBonus_(0)
, defenseBonus_(0)
```

- [ ] **Verificar que o projeto compila**

- [ ] **Commit do Chunk 5**

```
git add src/entities/Player.hpp src/entities/Player.cpp
git commit -m "feat: update Player with inventory, observers, and equipment bonuses"
```

---

## Chunk 6: Modificações em Game

### Task 10: Spawn de itens e controles

**Files:**
- Modify: `src/core/Game.hpp`
- Modify: `src/core/Game.cpp`

- [ ] **Adicionar em `Game.hpp`**

```cpp
#include "items/Item.hpp"
#include "items/ItemFactory.hpp"
#include "observers/StatsObserver.hpp"
#include "observers/LogObserver.hpp"

// campos privados:
std::vector<std::unique_ptr<Item>> items_;
bool inventarioAberto_;
StatsObserver statsObserver_;
LogObserver   logObserver_;
```

- [ ] **Atualizar construtor de `Game` — inicializar observers e spawn**

No construtor, após criar o player:
```cpp
// Conectar observers ao player
player_.adicionarObserver(&statsObserver_);

// Conectar logObserver ao inventario (callback de descarte)
player_.getInventario().onDescarte = [this](const std::string& msg) {
    logObserver_.onEvent(msg);
};

// Spawn de itens — shuffle para garantir variedade de tipos
std::vector<ItemType> tipos = {
    ItemType::PocaoDeVida, ItemType::PocaoDeForça,
    ItemType::Espada, ItemType::Armadura, ItemType::Amuleto
};
std::shuffle(tipos.begin(), tipos.end(), rng);

std::uniform_real_distribution<float> chance(0.0f, 1.0f);
int tipoIdx = 0;
for (int i = 1; i < rooms.size(); i++) {
    if (chance(rng) < 0.7f) {
        // Posição aleatória dentro da sala
        const Room& sala = rooms[i];
        std::uniform_int_distribution<int> rx(sala.x + 1, sala.x + sala.w - 2);
        std::uniform_int_distribution<int> ry(sala.y + 1, sala.y + sala.h - 2);
        int ix = rx(rng), iy = ry(rng);

        ItemType tipo = tipos[tipoIdx % tipos.size()];
        tipoIdx++;
        items_.push_back(ItemFactory::create(tipo, ix, iy));
    }
}
```

- [ ] **Atualizar `processInput()` — novas teclas**

```cpp
case 'g': coletarItem();   break;
case 'u': usarConsumivel(); break;
case 'i': inventarioAberto_ = !inventarioAberto_; break;
// dentro do inventário:
case '1': case '2': case '3': case '4': case '5':
    if (inventarioAberto_) usarConsumivelInventario(key - '1'); break;
case 'e': if (inventarioAberto_) equiparSelecionado(); break;
case 'x': if (inventarioAberto_) desequiparSelecionado(); break;
```

- [ ] **Implementar `coletarItem()` em `Game.cpp`**

```cpp
void Game::coletarItem() {
    for (int i = 0; i < items_.size(); i++) {
        if (items_[i]->getX() == player_.getX() &&
            items_[i]->getY() == player_.getY()) {
            std::string nome = items_[i]->getNome();
            bool coletou = player_.getInventario().adicionarItem(std::move(items_[i]));
            items_.erase(items_.begin() + i);
            if (coletou)
                logObserver_.onEvent("Coletou: " + nome);
            else
                logObserver_.onEvent("Inventario cheio!");
            return;
        }
    }
}
```

- [ ] **Implementar `usarConsumivel()` — usa item direto no chão**

```cpp
void Game::usarConsumivel() {
    for (int i = 0; i < items_.size(); i++) {
        if (items_[i]->getX() == player_.getX() &&
            items_[i]->getY() == player_.getY() &&
            items_[i]->getSlot() == ItemSlot::Consumivel) {
            std::string nome = items_[i]->getNome();
            items_[i]->usar(player_);
            items_.erase(items_.begin() + i);
            logObserver_.onEvent("Usou: " + nome);
            return;
        }
    }
}
```

- [ ] **Implementar `usarConsumivelInventario(int index)`**

```cpp
void Game::usarConsumivelInventario(int index) {
    auto item = player_.getInventario().removerConsumivel(index);
    if (!item) return;
    std::string nome = item->getNome();
    item->usar(player_);
    logObserver_.onEvent("Usou: " + nome);
}
```

- [ ] **Implementar `equiparSelecionado()` e `desequiparSelecionado()`**

```cpp
// Equipar: por simplicidade, tenta equipar o primeiro consumível selecionável
// O índice selecionado deve vir de uma variável de estado — simplificação:
// pressionar E tenta equipar consumivel[0] no slot correspondente
void Game::equiparSelecionado() {
    auto& inv = player_.getInventario();
    const auto& cons = inv.getConsumiveis();
    if (cons.empty()) return;
    ItemSlot slot = cons[0]->getSlot();
    inv.equipar(slot, 0);
    player_.notificarObservers();
    logObserver_.onEvent("Item equipado!");
}

void Game::desequiparSelecionado() {
    // Tenta desequipar arma primeiro, depois armadura, depois acessório
    auto& inv = player_.getInventario();
    for (ItemSlot s : {ItemSlot::Arma, ItemSlot::Armadura, ItemSlot::Acessorio}) {
        if (inv.getEquipado(s)) {
            inv.desequipar(s);
            player_.notificarObservers();
            logObserver_.onEvent("Item desequipado.");
            return;
        }
    }
}
```

- [ ] **Verificar que o projeto compila**

- [ ] **Commit do Chunk 6**

```
git add src/core/Game.hpp src/core/Game.cpp
git commit -m "feat: add item spawn, collect/use/equip controls in Game"
```

---

## Chunk 7: Renderer — itens no mapa e tela de inventário

### Task 11: Atualizar Renderer

**Files:**
- Modify: `src/ui/Renderer.hpp`
- Modify: `src/ui/Renderer.cpp`

- [ ] **Atualizar assinatura de `render()` em `Renderer.hpp`**

```cpp
#include "items/Item.hpp"
#include "items/Inventario.hpp"

void render(const Map& map, const Player& player,
            const std::vector<std::unique_ptr<Enemy>>& enemies,
            const std::vector<std::unique_ptr<Item>>& items,
            const std::deque<std::string>& messageLog,
            bool inventarioAberto);

void renderInventario(const Inventario& inv);
```

- [ ] **Atualizar `render()` em `Renderer.cpp` — desenhar itens no mapa**

No loop de tiles, após checar inimigos, antes de desenhar o tile:
```cpp
// Desenha item no chão se tile está explorado
bool hadItem = false;
for (const auto& item : items) {
    if (item->getX() == x && item->getY() == y) {
        std::cout << item->getSymbol();
        hadItem = true;
        break;
    }
}
if (!hadItem && !hadEnemy) std::cout << map.getTile(x, y);
```

- [ ] **Atualizar `renderHUD()` — abrir inventário se flag ativa**

```cpp
void Renderer::renderHUD(const Player& player,
                         const std::deque<std::string>& messageLog,
                         bool inventarioAberto) {
    if (inventarioAberto) {
        renderInventario(player.getInventario());
        return;
    }
    // ... HUD normal (sem mudanças)
}
```

- [ ] **Implementar `renderInventario()` em `Renderer.cpp`**

```cpp
void Renderer::renderInventario(const Inventario& inv) {
    std::cout << std::string(40, '-') << '\n';
    std::cout << " INVENTARIO\n";
    std::cout << std::string(40, '-') << '\n';
    std::cout << " Equipados:\n";

    auto printSlot = [&](const char* label, ItemSlot slot) {
        Item* item = inv.getEquipado(slot);
        std::cout << "   " << label << (item ? item->getNome() : "--") << '\n';
    };

    printSlot("Arma:      ", ItemSlot::Arma);
    printSlot("Armadura:  ", ItemSlot::Armadura);
    printSlot("Acessorio: ", ItemSlot::Acessorio);

    std::cout << "\n Consumiveis:\n";
    const auto& cons = inv.getConsumiveis();
    for (int i = 0; i < 5; i++) {
        std::cout << "   [" << (i + 1) << "] ";
        if (i < static_cast<int>(cons.size()))
            std::cout << cons[i]->getNome();
        else
            std::cout << "--";
        std::cout << '\n';
    }

    std::cout << std::string(40, '-') << '\n';
    std::cout << " [1-5] Usar  [E] Equipar  [X] Desequipar  [I] Fechar\n";
    std::cout << std::string(40, '-') << '\n';
}
```

- [ ] **Atualizar chamada de `render()` em `Game::render()`**

```cpp
void Game::render() {
    renderer_.render(map_, player_, enemies_, items_, messageLog_, inventarioAberto_);
}
```

- [ ] **Verificar que o projeto compila e roda**

Testar:
- Itens aparecem no mapa com símbolos corretos
- `G` coleta item
- `U` usa consumível no chão
- `I` abre/fecha tela de inventário
- `1`-`5` usa consumível do inventário
- `E` equipa, `X` desequipa
- Log mostra mensagens corretas

- [ ] **Commit final do Chunk 7**

```
git add src/ui/Renderer.hpp src/ui/Renderer.cpp
git commit -m "feat: render items on map and inventory screen"
```

---

## Resumo de arquivos

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

## Lembrete CMake

Sempre que adicionar arquivos `.cpp` novos, rodar no VS Code:
**`CMake: Delete Cache and Reconfigure`** seguido de **`CMake: Build`**
