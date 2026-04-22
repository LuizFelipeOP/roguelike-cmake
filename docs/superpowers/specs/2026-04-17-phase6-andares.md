# Spec — Phase 6: Múltiplos Andares e Progressão de Dungeon

**Data:** 2026-04-17  
**Status:** Aprovada  

---

## Objetivo

Adicionar progressão vertical ao dungeon: o jogador pode descer andares pisando em uma escada (`>`). Cada andar gera um novo mapa, inimigos mais fortes e itens mais poderosos.

---

## Decisões de design

### Escada (`>`)
- Tile especial `'>'` colocado no canto interno de uma sala aleatória (não a primeira)
- Os quatro cantos internos possíveis de uma sala `(x, y, width, height)` são:
  - `(x+1, y+1)` — superior esquerdo
  - `(x+width-2, y+1)` — superior direito
  - `(x+1, y+height-2)` — inferior esquerdo
  - `(x+width-2, y+height-2)` — inferior direito
- Um dos quatro é escolhido aleatoriamente com o `rng` já presente em `generate()`
- A escada respeita o Fog of War: só aparece quando o tile for explorado (`isExplored`)
- `Map` armazena a posição em campo privado `Point escada_` e expõe `getPosicaoEscada()`

### Progressão por andar
- Cada andar incrementa `andarAtual_` em `Game`
- Multiplicador de stats dos inimigos: `1.0 + (andar - 1) * 0.2` (andar 1 = base, andar 2 = +20%, etc.)
- `EnemyFactory::create` recebe `int andar` com default `= 1`; calcula stats internamente
- Itens também escalam: `ItemFactory::create` recebe `int andar` com default `= 1`; multiplica valores das strategies
- Ao descer: mapa anterior é descartado, `enemies_` e `items_` são limpos

### Estado preservado ao descer
- Preservado: HP, inventário, XP, nível do player
- Descartado: mapa anterior, inimigos, itens no chão

### Reconexão de observers
- `descerAndar()` deve reconectar o `onDescarte` do inventário ao `logObserver_`
- Motivo: o callback não é destruído, mas é boa prática garantir a reconexão após reinicializar o mapa

### HUD
- Exibe `"Andar: N"` na barra de status
- `render()` e `renderHUD()` recebem `int andar` como parâmetro adicional

---

## Fluxo de descida

```
Player pisa em '>' (posição == escada_)
    → descerAndar()
        → andarAtual_++
        → map_.generate(seed + andarAtual_)
        → enemies_.clear(), items_.clear()
        → reposicionar player na primeira sala
        → reconectar onDescarte
        → respawnar inimigos com andar
        → respawnar itens com andar
        → map_.updateVisibility(player pos)
        → logObserver_.onEvent("Voce desceu para o andar N")
```

---

## Arquivos afetados

| Arquivo | Mudança |
|---|---|
| `Map.hpp` | + campo `escada_`, + `getPosicaoEscada()` |
| `Map.cpp` | `generate()` posiciona `'>'` em canto de sala aleatória |
| `Goblin.hpp/.cpp` | + construtor com stats customizáveis |
| `Troll.hpp/.cpp` | + construtor com stats customizáveis |
| `EnemyFactory.hpp/.cpp` | + parâmetro `int andar`, calcula multiplicador |
| `ItemFactory.hpp/.cpp` | + parâmetro `int andar`, escala valores das strategies |
| `Game.hpp` | + campo `andarAtual_`, + método `descerAndar()` |
| `Game.cpp` | implementa `descerAndar()`, checa escada em `update()`, passa `andar` para render/factories |
| `Renderer.hpp` | + parâmetro `int andar` em `render()` e `renderHUD()` |
| `Renderer.cpp` | exibe `"Andar: N"` no HUD |

---

## Patterns praticados nesta fase

- **Factory Method** reforçado: factories agora recebem contexto (andar) para variar a criação
- **Single Responsibility**: `descerAndar()` encapsula toda a lógica de transição de andar em `Game`
