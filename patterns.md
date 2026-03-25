# Design Patterns — Referência do Projeto

Explicações práticas dos patterns aplicados neste roguelike.
Atualizado conforme novas fases são implementadas.

---

## Strategy Pattern

**Fase aplicada:** Phase 5 — Itens

**Problema que resolve:**
Vários objetos fazem a mesma "coisa" (usar um item), mas de formas diferentes.
Sem o pattern, você acaba com um `switch` gigante que cresce a cada novo tipo:

```cpp
// Sem Strategy — ruim
if (tipo == POCAO)  { cura(); }
else if (tipo == BOMBA)   { explode(); }
else if (tipo == SCROLL)  { teleporta(); }
```

**Solução:**
Cada comportamento vira uma classe separada com a mesma interface.
O objeto que "usa" carrega um ponteiro para a estratégia e não precisa saber qual é.

```cpp
class ItemStrategy {
public:
    virtual void usar(Player& player) = 0;
    virtual ~ItemStrategy() = default;
};

class CuraStrategy : public ItemStrategy {
    void usar(Player& player) override { player.curar(20); }
};

class AumentarATKStrategy : public ItemStrategy {
    void usar(Player& player) override { player.raiseAttack(); }
};

// O Item delega — não sabe o que acontece
item.getStrategy()->usar(player);
```

**Princípio SOLID praticado:** Open/Closed
Adicionar um novo tipo de item não exige mexer no código existente — só criar uma nova Strategy.

---

## Observer Pattern

**Fase aplicada:** Phase 5 — Itens

**Problema que resolve:**
Quando o jogador equipa uma espada, o ATK precisa subir. Quando desequipa, volta.
Sem o pattern, o `Player` ficaria cheio de `if` e precisaria conhecer todos os seus dependentes:

```cpp
// Sem Observer — ruim
void equipar(Item* item) {
    if (item->tipo == ESPADA)   attack_  += item->bonus;
    if (item->tipo == ARMADURA) defense_ += item->bonus;
}
```

**Solução:**
O `Player` é o **Subject** (quem muda de estado).
Os interessados são **Observers** (quem reage à mudança).
O Subject mantém uma lista de observers e os notifica quando algo muda.

```cpp
class Observer {
public:
    virtual void onNotify(Player& player) = 0;
    virtual ~Observer() = default;
};

// Subject (Player) avisa todos os observers
player.notificarObservers();

// Observer reage
class StatsObserver : public Observer {
    void onNotify(Player& player) override {
        // recalcula ATK baseado nos equipamentos atuais
    }
};
```

**Duas aplicações neste projeto:**
1. Stats do Player reagem ao equipar/desequipar (ATK/DEF sobem e descem)
2. Log de mensagens é notificado quando eventos acontecem (item coletado, level up, etc.)

**Princípio SOLID praticado:** Single Responsibility
O `Player` não precisa saber como recalcular cada stat — só avisa que algo mudou.

---

## Factory Pattern

**Fase aplicada:** Phase 3 (EnemyFactory) e Phase 5 (ItemFactory)

**Problema que resolve:**
A criação de objetos complexos espalhada pelo código. Se a lógica de criação mudar,
você precisa caçar todos os lugares onde o objeto é criado.

**Solução:**
Centralizar a criação em uma classe Factory. O resto do código só chama `Factory::create()`.

```cpp
// EnemyFactory — já implementado na Phase 3
enemies_.push_back(EnemyFactory::create(EnemyType::Goblin, x, y));

// ItemFactory — Phase 5
items_.push_back(ItemFactory::create(ItemType::PocaoDeVida, x, y));
```

**Princípio SOLID praticado:** Single Responsibility + Open/Closed
A lógica de criação fica em um só lugar. Adicionar um novo tipo só exige mudar a Factory.

---

## Próximos patterns (fases futuras)

| Pattern | Fase prevista | Aplicação |
|---------|--------------|-----------|
| Command | Phase 6? | Encapsular ações do jogador para desfazer/refazer |
| State   | Phase 6?  | Estados de IA do inimigo (patrulha, perseguição, fuga) |
| Singleton | Phase 7? | Gerenciador de save/load |
