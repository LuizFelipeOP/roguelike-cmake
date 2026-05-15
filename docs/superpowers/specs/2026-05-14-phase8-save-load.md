# Spec — Phase 8: Persistência e Save/Load

**Data:** 2026-05-14  
**Status:** Aprovada

---

## Objetivo

Implementar save e load do estado completo do jogo em arquivo JSON, usando o padrão **Memento**. O jogador pode salvar com `S` e carregar com `L` a qualquer momento durante a partida.

---

## Motivação

O roguelike é um jogo de sessões longas. Sem persistência, qualquer interrupção perde o progresso. A Phase 8 resolve isso de forma arquiteturalmente limpa, separando os dados serializáveis (Memento) da lógica de I/O (SaveSystem), sem poluir as classes de domínio.

---

## Decisões de design

### Pattern: Memento

- `GameMemento` (`src/persistence/GameMemento.hpp`) — struct puro com todos os dados serializáveis. Sem lógica, sem dependências de jogo.
- `Game` é o **Originator** — cria o memento em `salvar()` e restaura o estado em `carregar()`.
- `SaveSystem` é o **Caretaker** — responsável exclusivamente por escrever e ler o arquivo JSON.

```
Game::salvar()
    → coleta estado → GameMemento
    → SaveSystem::escrever(memento, "save.json")

Game::carregar()
    → SaveSystem::ler("save.json") → GameMemento
    → restaura todos os subsistemas a partir do memento
```

### Biblioteca JSON: nlohmann/json

Adicionada via `CMakeLists.txt` com `FetchContent`. Header-only, sem DLL.

```cmake
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
)
```

> Alternativa mais robusta: usar o repositório completo com `FetchContent_MakeAvailable`.

---

## Structs do GameMemento

Definidos em `src/persistence/GameMemento.hpp`. Todos são structs POD ou near-POD.

```cpp
struct EfeitoMemento {
    std::string tipo;    // "Veneno", "Paralisia", "Regeneracao", "Enfraquecimento"
    int duracaoRestante;
    int potencia;        // dano (Veneno), cura (Regeneracao), reducao (Enfraquecimento)
                         // Para Paralisia: potencia não é usado (usar 0)
};

struct PlayerMemento {
    int x, y;
    int hp, maxHp;
    int attack, defense, luck;
    int xp, level, xpProxLevel;
    int attackBonus, defenseBonus;
    std::vector<EfeitoMemento> efeitos;
};

struct ItemMemento {
    std::string tipo;    // nome do enum ItemType serializado como string
    int x, y;            // -1,-1 se estiver no inventário
    bool equipado;
    std::string slot;    // "Arma", "Armadura", "Acessorio", "Consumivel"
};

struct EnemyMemento {
    std::string tipo;    // "Goblin", "Troll", "Orc"
    int x, y;
    int hp;
    std::string iaEstado; // "Perseguindo", "Fugindo"
    std::vector<EfeitoMemento> efeitos;
};

struct MapMemento {
    unsigned int seed;
    int andar;
    std::vector<std::vector<bool>> explored; // grade completa do fog of war
    int escadaX, escadaY;                    // posição da escada atual
};

struct GameMemento {
    PlayerMemento player;
    std::vector<ItemMemento> itensInventario;  // itens dentro do inventário
    std::vector<ItemMemento> itensMapa;        // itens ainda no chão
    std::vector<EnemyMemento> inimigos;
    MapMemento mapa;
};
```

---

## Preparação das classes existentes

### Map — adicionar `seed_` e `getSeed()`

`generate()` recebe a seed mas não a armazena. Precisamos guardar para serialização.

```cpp
// Map.hpp — adicionar:
unsigned int getSeed() const;

// Map.hpp — private:
unsigned int seed_;

// Map.cpp — em generate():
seed_ = (seed == 0) ? /* seed aleatória gerada */ : seed;
// A seed aleatória já é gerada internamente — apenas armazená-la em seed_
```

### Enemy — adicionar `IAEstado`

```cpp
// Enemy.hpp — adicionar enum e getter:
enum class IAEstado { Perseguindo, Fugindo };
IAEstado getIAEstado() const;

// Enemy.cpp — implementação:
// Usa dynamic_cast internamente para não expor os tipos de IA ao exterior
IAEstado Enemy::getIAEstado() const {
    if (dynamic_cast<FugirStrategy*>(ia_.get())) return IAEstado::Fugindo;
    return IAEstado::Perseguindo;
}
```

> Nota: o `dynamic_cast` fica **dentro de `Enemy.cpp`**, não no `SaveSystem`. O domínio conhece suas próprias strategies; o sistema de persistência só enxerga o enum.

### StatusEffect — adicionar getters para serialização

`StatusEffect` atualmente expõe apenas `tick()`, `isExpired()` e `getNome()`. Para serializar precisamos de duração restante e potência.

```cpp
// StatusEffect.hpp — adicionar:
virtual int getDuracao() const = 0;
virtual int getPotencia() const = 0;
```

Cada subclasse (`VenenoEffect`, `ParalisiaEffect`, `RegeneracaoEffect`, `EnfraquecimentoEffect`) implementa retornando seus campos privados correspondentes.  
Para `ParalisiaEffect`, `getPotencia()` retorna `0`.

---

## SaveSystem

```cpp
// src/persistence/SaveSystem.hpp
#pragma once
#include "GameMemento.hpp"
#include <string>

class SaveSystem {
public:
    static void escrever(const GameMemento& memento, const std::string& caminho);
    static GameMemento ler(const std::string& caminho);
    static bool existeSave(const std::string& caminho);
};
```

- `escrever()` serializa `GameMemento` para JSON e grava em disco.
- `ler()` lê o JSON e preenche um `GameMemento`.
- `existeSave()` verifica se o arquivo existe antes de tentar carregar.

### Estratégia de serialização de enums

Enums são salvos como strings para legibilidade:

```json
{ "tipo": "Goblin", "iaEstado": "Perseguindo" }
```

Funções auxiliares internas no `SaveSystem.cpp`:
```cpp
static std::string itemTipoParaString(ItemType t);
static ItemType stringParaItemTipo(const std::string& s);
// idem para EnemyType, IAEstado, ItemSlot
```

---

## Integração no Game

### Novos métodos em `Game`

```cpp
// Game.hpp — adicionar em private:
void salvar();
void carregar();
```

### `Game::salvar()`

1. Monta `PlayerMemento` a partir de `player_`
2. Serializa inventário (arma, armadura, acessório, consumíveis) → `itensInventario`
3. Serializa `items_` (itens no mapa) → `itensMapa`
4. Serializa `enemies_` → `inimigos`
5. Monta `MapMemento` com `map_.getSeed()`, `andarAtual_`, `explored_[][]`, posição da escada
6. Chama `SaveSystem::escrever(memento, "save.json")`
7. `pushMessage("Jogo salvo.")`

### `Game::carregar()`

1. Verifica `SaveSystem::existeSave("save.json")` — se não existir: `pushMessage("Nenhum save encontrado.")` e retorna
2. Lê `GameMemento` via `SaveSystem::ler("save.json")`
3. Restaura `map_`: chama `map_.generate(memento.mapa.seed)` e restaura `explored_[][]` + escada
4. Restaura `player_`: define posição, stats, efeitos
5. Restaura `items_` via `ItemFactory::create(tipo, x, y)` para cada `ItemMemento`
6. Restaura inventário: recria itens e os adiciona via `inventario_.adicionarItem()` + `equipar()` se aplicável
7. Restaura `enemies_` via `EnemyFactory::create(tipo, x, y, andar)`, aplica HP salvo e IA state
8. `pushMessage("Jogo carregado.")`

### `Game::processInput()`

```cpp
case 'S': salvar(); break;
case 'L': carregar(); break;
```

---

## Fluxo do arquivo save.json

```json
{
  "player": {
    "x": 10, "y": 5,
    "hp": 18, "maxHp": 25,
    "attack": 7, "defense": 3, "luck": 1,
    "xp": 40, "level": 2, "xpProxLevel": 60,
    "attackBonus": 2, "defenseBonus": 1,
    "efeitos": [
      { "tipo": "Veneno", "duracaoRestante": 2, "potencia": 1 }
    ]
  },
  "itensInventario": [
    { "tipo": "Espada", "x": -1, "y": -1, "equipado": true, "slot": "Arma" },
    { "tipo": "PocaoDeVida", "x": -1, "y": -1, "equipado": false, "slot": "Consumivel" }
  ],
  "itensMapa": [
    { "tipo": "Armadura", "x": 20, "y": 8, "equipado": false, "slot": "Armadura" }
  ],
  "inimigos": [
    { "tipo": "Goblin", "x": 12, "y": 6, "hp": 2, "iaEstado": "Fugindo", "efeitos": [] },
    { "tipo": "Troll",  "x": 30, "y": 14, "hp": 20, "iaEstado": "Perseguindo", "efeitos": [] }
  ],
  "mapa": {
    "seed": 1234567890,
    "andar": 2,
    "escadaX": 45, "escadaY": 18,
    "explored": [[false, true, ...], ...]
  }
}
```

---

## Arquivos afetados

| Arquivo | Ação |
|---|---|
| `CMakeLists.txt` | EDITAR — adicionar nlohmann/json via FetchContent |
| `src/persistence/GameMemento.hpp` | CRIAR — structs de snapshot |
| `src/persistence/SaveSystem.hpp` | CRIAR — interface do caretaker |
| `src/persistence/SaveSystem.cpp` | CRIAR — implementação JSON |
| `src/map/Map.hpp` | EDITAR — adicionar `seed_`, `getSeed()` |
| `src/map/Map.cpp` | EDITAR — armazenar seed em `generate()` |
| `src/entities/Enemy.hpp` | EDITAR — adicionar `IAEstado` enum e `getIAEstado()` |
| `src/entities/Enemy.cpp` | EDITAR — implementar `getIAEstado()` com dynamic_cast interno |
| `src/ia/FugirStrategy.hpp` | NENHUMA — apenas include em Enemy.cpp |
| `src/effects/StatusEffect.hpp` | EDITAR — adicionar `getDuracao()` e `getPotencia()` puros |
| `src/effects/VenenoEffect.hpp/.cpp` | EDITAR — implementar getters |
| `src/effects/ParalisiaEffect.hpp/.cpp` | EDITAR — implementar getters |
| `src/effects/RegeneracaoEffect.hpp/.cpp` | EDITAR — implementar getters |
| `src/effects/EnfraquecimentoEffect.hpp/.cpp` | EDITAR — implementar getters |
| `src/core/Game.hpp` | EDITAR — declarar `salvar()`, `carregar()` |
| `src/core/Game.cpp` | EDITAR — implementar `salvar()`, `carregar()`, teclas S/L |

---

## Chunks de implementação

| Chunk | Conteúdo |
|---|---|
| **1** | CMake + nlohmann/json + `GameMemento.hpp` + `SaveSystem` (stubs) |
| **2** | Preparar classes: `Map::seed_`, `Enemy::IAEstado`, `StatusEffect` getters |
| **3** | Implementar `SaveSystem::escrever()` — serialização completa para JSON |
| **4** | Implementar `SaveSystem::ler()` + `Game::salvar()` + `Game::carregar()` + teclas S/L |

---

## Patterns praticados

- **Memento** — `GameMemento` como snapshot puro; `Game` como Originator; `SaveSystem` como Caretaker
- **Single Responsibility** — `SaveSystem` cuida apenas de I/O; `GameMemento` só guarda dados; classes de domínio não sabem que estão sendo serializadas
- **Open/Closed** — adicionar novo campo ao save = adicionar ao `GameMemento` + `SaveSystem` sem tocar no resto
