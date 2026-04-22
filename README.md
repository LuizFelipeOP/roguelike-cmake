# Roguelike

Dungeon roguelike em C++ com geração procedural de mapas.

## Pré-requisitos

- Visual Studio 2022 (Community ou Build Tools)
- CMake (baixe em cmake.org/download — marque "Add to PATH" na instalação)
- VS Code com as extensões:
  - C/C++ (Microsoft)
  - CMake Tools (Microsoft)

## Como compilar

### Pelo VS Code

1. Abra o VS Code
2. `File > Open Folder` — selecione a pasta `roguelike/`
3. `Ctrl+Shift+P` — digite `CMake: Configure` — Enter
4. Selecione o kit: `Visual Studio Community 2022 Release - amd64`
5. Aperte `F7` para compilar

### Pelo terminal

Abra o terminal no VS Code com `Ctrl+` (acento grave) e rode:

```
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

## Como rodar

No terminal do VS Code:

```
.\build\Debug\roguelike.exe
```

## Controles

| Tecla | Acao     |
|-------|----------|
| W     | Cima     |
| A     | Esquerda |
| S     | Baixo    |
| D     | Direita  |
| Q     | Sair     |

## Testes automatizados

O projeto usa [doctest](https://github.com/doctest/doctest) como framework de testes. As dependências são baixadas automaticamente pelo CMake na primeira vez que você configura o projeto.

### Rodando os testes pelo terminal

```
cmake -S . -B build
cmake --build build --target roguelike_tests
cd build
ctest --output-on-failure
```

- `cmake -S . -B build` — configura o projeto e baixa o doctest (necessário apenas na primeira vez, ou ao adicionar novos arquivos de teste)
- `cmake --build build --target roguelike_tests` — compila apenas o executável de testes, sem compilar o jogo
- `ctest --output-on-failure` — roda todos os testes e exibe detalhes apenas dos que falharam

Se preferir ver a saída completa de todos os testes:

```
ctest -V
```

### Rodando o executável de testes diretamente

```
.\build\Debug\roguelike_tests.exe
```

Isso mostra o resultado de cada `TEST_CASE` individualmente no console.

### Estrutura dos testes

```
tests/
├── main_tests.cpp        — ponto de entrada (define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
├── test_entities.cpp     — Player, Goblin, Troll, Enemy
├── test_factories.cpp    — EnemyFactory, ItemFactory
├── test_map.cpp          — Room, Map, Fog of War
├── test_items.cpp        — Strategies, Item, Inventario
├── test_observers.cpp    — StatsObserver, anti-duplicacao de observer
└── test_game.cpp         — Game (andares, descerAndar)
```

> **Nota sobre erros de LSP no VS Code:** os arquivos em `tests/` podem mostrar sublinhados vermelhos no editor porque o `c_cpp_properties.json` nao aponta para os headers do doctest e do `src/`. Isso e um falso positivo — o CMake compila e os testes passam normalmente. Para eliminar os erros visuais, use a extensao CMake Tools e deixe ela gerenciar o IntelliSense.

## Estrutura do projeto

```
roguelike/
├── CMakeLists.txt
└── src/
    ├── main.cpp
    ├── core/
    │   ├── Game.hpp / Game.cpp       — loop principal do jogo
    ├── entities/
    │   ├── Entity.hpp / Entity.cpp   — classe base abstrata
    │   ├── Player.hpp / Player.cpp   — jogador (@)
    ├── map/
    │   ├── Map.hpp / Map.cpp         — mapa e geracao procedural
    │   ├── Room.hpp / Room.cpp       — salas do dungeon
    └── ui/
        ├── Renderer.hpp / Renderer.cpp — renderizacao no console
```
