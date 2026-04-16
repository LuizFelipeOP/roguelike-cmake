# Roguelike C++ — Roadmap do Projeto

Projeto roguelike clássico (inspirado no Rogue de 1980) desenvolvido em C++17 como exercício progressivo de aprendizado de arquitetura de software, design patterns, SOLID e Clean Code.

**Stack:** C++17, CMake, Visual Studio 2022 Community, console Windows/Linux  
**Convenções:** português brasileiro para nomes de métodos e variáveis de domínio, inglês para nomes de arquivo e estrutura de pastas

---

## Phases concluídas

### Phase 1 — Fundação
**Status:** Concluída  
**Objetivo:** Estrutura base do projeto, loop de jogo, entidade genérica e renderização no console.  
**O que foi feito:**
- Configuração do CMake com GLOB_RECURSE
- Classe abstrata `Entity` com posição e símbolo
- Classe `Player` herdando de `Entity`
- Classe `Map` com grid de tiles (parede/chão)
- Classe `Renderer` desenhando mapa + player no console
- Loop principal em `Game`: render → input → update

---

### Phase 2 — Geração Procedural de Mapa
**Status:** Concluída  
**Objetivo:** Geração procedural de salas e corredores, posicionamento do player na primeira sala.  
**O que foi feito:**
- Algoritmo de geração de salas com seed aleatória (`std::mt19937`)
- Corredores conectando salas (horizontal + vertical)
- Player posicionado no centro da primeira sala gerada
- Estrutura `Room` e `Point` no `Map`

---

### Phase 3 — Inimigos, Factory Pattern e Combate
**Status:** Concluída  
**Objetivo:** Inimigos com hierarquia de classes, Factory pattern, IA de perseguição e combate por colisão (bump attack).  
**Patterns praticados:** Factory Method, herança/polimorfismo  
**O que foi feito:**
- Classe abstrata `Enemy` herdando de `Entity`
- Subclasses concretas `Goblin` e `Troll` com atributos distintos
- `EnemyFactory::create()` como único ponto de criação de inimigos
- IA simples: inimigo persegue player quando explorado
- Combate por colisão: mover para tile do inimigo = atacar
- `takeDamage()` e `isAlive()` no `Player`
- Morte do jogador encerra o jogo

---

### Phase 4 — XP, Níveis e Fog of War
**Status:** Concluída  
**Objetivo:** Progressão do jogador e visibilidade limitada do mapa.  
**O que foi feito:**
- Sistema de XP: `addXP()` no `Player`, `getXPReward()` virtual puro em `Enemy`
- Level up com melhoria aleatória de atributo (HP, ATK, DEF, Sorte)
- Fog of War: grid `explored_[][]` no `Map`, `updateVisibility()` por raio
- Tiles não explorados não são renderizados
- Log de mensagens (`std::deque<std::string>`) exibido no HUD
- HUD atualizado com HP, nível, XP e mensagens

---

### Phase 5 — Itens, Inventário e Design Patterns
**Status:** Concluída
**Objetivo:** Sistema completo de itens com inventário, equipamentos, consumíveis e passivos.  
**Patterns praticados:** Strategy, Observer, Factory  
**O que foi feito / está sendo feito:**
- `ItemStrategy` (interface) + strategies concretas: `CuraStrategy`, `AumentarATKStrategy`, `BonusDefStrategy`
- Classe `Item` com delegação de comportamento para a strategy
- Enums `ItemType` e `ItemSlot`
- `Observer` (interface) + `StatsObserver` (recalcula bônus de equip) + `LogObserver` (registra eventos)
- `Inventario` com slots limitados por tipo (arma, armadura, acessório, consumíveis x5)
- `ItemFactory::create()` como único ponto de criação de itens
- `Player` como Subject: `adicionarObserver`, `notificarObservers`, `getInventario`
- Integração em `Game`: spawn de itens nas salas, teclas para coletar/usar/equipar
- Renderer atualizado: itens no mapa + tela de inventário

---

## Phases planejadas (sugestões)

### Phase 6 — Múltiplos Andares e Progressão de Dungeon
**Status:** Em andamento  
**Objetivo:** Adicionar escadas para descer/subir andares, com dificuldade crescente.  
**O que pode ser feito:**
- Tile especial `>` (escada para baixo) gerado em sala aleatória
- Ao pisar na escada, gerar novo mapa com seed diferente
- Inimigos mais fortes a cada andar (multiplicador de stats)
- HUD exibindo número do andar atual
- Patterns sugeridos: State (estado do andar atual)

---

### Phase 7 — Combate Avançado e Status Effects
**Objetivo:** Tornar o combate mais profundo com efeitos de status.  
**O que pode ser feito:**
- Efeitos: veneno, paralisia, regeneração
- Sistema de duração por turnos
- Inimigos com comportamentos distintos (ataque à distância, fuga, cura)
- Patterns sugeridos: Command (ações de turno), Decorator (efeitos empilháveis)

---

### Phase 8 — Persistência e Save/Load
**Objetivo:** Salvar e carregar o estado do jogo.  
**O que pode ser feito:**
- Serialização do estado: mapa, player, inimigos, itens, andar
- Salvar seed do mapa para recriar dungeon identicamente
- Arquivo de save em JSON ou formato binário simples
- Patterns sugeridos: Memento (snapshot do estado)

---

### Phase 9 — Interface e Qualidade de Vida
**Objetivo:** Melhorar a experiência visual e de uso.  
**O que pode ser feito:**
- Cores no console (ANSI escape codes)
- Tela de morte com estatísticas da run
- Histórico completo do log de mensagens
- Menu inicial (nova partida / carregar / sair)
- Patterns sugeridos: Facade (encapsular terminal colorido)

---

### Phase 10 — Roguelike Completo
**Objetivo:** Juntar tudo em uma run completa e jogável.  
**O que pode ser feito:**
- Boss no último andar
- Itens raros e lendários (`ItemRarity`)
- Identificação de itens (itens começam "desconhecidos")
- Placar de pontuação local
- Patterns sugeridos: Flyweight (itens com dados compartilhados), Chain of Responsibility (pipeline de dano)

---

## Estrutura de arquivos atual

```
roguelike/
├── CMakeLists.txt
├── patterns.md
├── docs/
│   ├── ROADMAP.md              ← este arquivo
│   └── superpowers/
│       ├── plans/              ← planos de implementação por phase
│       └── specs/              ← especificações de design por phase
└── src/
    ├── core/
    │   ├── Game.hpp / Game.cpp
    ├── entities/
    │   ├── Entity.hpp / Entity.cpp
    │   ├── Player.hpp / Player.cpp
    │   ├── Enemy.hpp / Enemy.cpp
    │   ├── Goblin.hpp / Goblin.cpp
    │   ├── Troll.hpp / Troll.cpp
    │   └── EnemyFactory.hpp / EnemyFactory.cpp
    ├── items/
    │   ├── ItemStrategy.hpp
    │   ├── CuraStrategy.hpp / .cpp
    │   ├── AumentarATKStrategy.hpp / .cpp
    │   ├── BonusDefStrategy.hpp / .cpp
    │   ├── Item.hpp / Item.cpp
    │   ├── Inventario.hpp / Inventario.cpp
    │   ├── ItemFactory.hpp / ItemFactory.cpp
    ├── map/
    │   ├── Map.hpp / Map.cpp
    ├── observers/
    │   ├── Observer.hpp
    │   ├── StatsObserver.hpp / StatsObserver.cpp
    │   └── LogObserver.hpp / LogObserver.cpp
    └── ui/
        ├── Renderer.hpp / Renderer.cpp
```
