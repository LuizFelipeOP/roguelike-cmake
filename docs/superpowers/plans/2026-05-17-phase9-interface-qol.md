# Phase 9 — Interface e Qualidade de Vida — Plano de Implementação

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Adicionar ConsoleFacade (Facade pattern), máquina de estados, menu inicial, tela de morte e histórico de log ao roguelike.

**Architecture:** `ConsoleFacade` encapsula toda a lógica de cor/cursor multiplataforma; `EstadoJogo` enum transforma `Game` em uma máquina de estados simples (Menu → Jogando → Morto / Historico); `Renderer` ganha três novos métodos de tela.

**Tech Stack:** C++17, MSVC, Win32 API (ConsoleFacade), doctest v2.4.11

**Spec:** `docs/superpowers/specs/2026-05-17-phase9-interface-qol.md`

---

## Chunk 1: ConsoleFacade + Refactor do Renderer

### Task 1: Criar ConsoleFacade

**Files:**
- Criar: `src/ui/ConsoleFacade.hpp`
- Criar: `src/ui/ConsoleFacade.cpp`

**Contexto:**  
`Renderer.cpp` tem `#ifdef _WIN32` e ANSI codes em três métodos (`setCorEfeito`, `resetarCor`, `setCorPorNome`).  
A fachada centraliza tudo isso em um único lugar. O `Renderer` só vai falar com a fachada — nunca mais com a API do terminal diretamente.

- [ ] **Criar `src/ui/ConsoleFacade.hpp`:**

```cpp
// ConsoleFacade.hpp — Fachada para operações de terminal
//
// Pattern: Facade
// Problema resolvido: Renderer não deve saber se estamos no Windows ou Linux.
// Toda a lógica de #ifdef fica aqui — o Renderer só chama setColor/resetColor/limpar.
#pragma once
#include <string>

enum class Cor {
    Padrao,         // branco — reset
    Amarelo,        // Paralisia
    Verde,          // Veneno
    Ciano,          // Regeneracao
    Vermelho,       // Enfraquecimento
    Magenta,        // destaque de menu
    AzulClaro,      // HUD / info
};

class ConsoleFacade {
public:
    void setColor(Cor cor);   // aplica cor no terminal
    void resetColor();        // volta para Cor::Padrao
    void limpar();            // limpa a tela
    void ocultarCursor();     // remove o cursor piscante
};
```

- [ ] **Criar `src/ui/ConsoleFacade.cpp`:**

```cpp
// ConsoleFacade.cpp — Implementação multiplataforma da fachada de terminal

#include "ui/ConsoleFacade.hpp"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

void ConsoleFacade::setColor(Cor cor) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (cor) {
        case Cor::Amarelo:   SetConsoleTextAttribute(h, 14); break;
        case Cor::Verde:     SetConsoleTextAttribute(h, 10); break;
        case Cor::Ciano:     SetConsoleTextAttribute(h, 11); break;
        case Cor::Vermelho:  SetConsoleTextAttribute(h, 12); break;
        case Cor::Magenta:   SetConsoleTextAttribute(h, 13); break;
        case Cor::AzulClaro: SetConsoleTextAttribute(h, 9);  break;
        default:             SetConsoleTextAttribute(h, 7);  break; // Padrao
    }
#else
    switch (cor) {
        case Cor::Amarelo:   std::cout << "\033[33m"; break;
        case Cor::Verde:     std::cout << "\033[32m"; break;
        case Cor::Ciano:     std::cout << "\033[36m"; break;
        case Cor::Vermelho:  std::cout << "\033[31m"; break;
        case Cor::Magenta:   std::cout << "\033[35m"; break;
        case Cor::AzulClaro: std::cout << "\033[34m"; break;
        default:             std::cout << "\033[0m";  break;
    }
#endif
}

void ConsoleFacade::resetColor() {
    setColor(Cor::Padrao);
}

void ConsoleFacade::limpar() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD tamanho = csbi.dwSize.X * csbi.dwSize.Y;
    COORD origem = {0, 0};
    DWORD escrito;
    FillConsoleOutputCharacter(hConsole, ' ', tamanho, origem, &escrito);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, tamanho, origem, &escrito);
    SetConsoleCursorPosition(hConsole, origem);
#else
    std::cout << "\033[2J\033[H";
#endif
}

void ConsoleFacade::ocultarCursor() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = {1, FALSE};
    SetConsoleCursorInfo(h, &info);
#else
    std::cout << "\033[?25l";
#endif
}
```

- [ ] **Buildar (F7) — deve compilar sem erros.**

---

### Task 2: Refatorar Renderer para usar ConsoleFacade

**Files:**
- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`

**Contexto:**  
`Renderer` tem três métodos privados de cor (`setCorEfeito`, `resetarCor`, `setCorPorNome`) e `clearScreen()`.  
Todos são substituídos por chamadas à `ConsoleFacade console_`.

- [ ] **Adicionar `ConsoleFacade console_` como membro privado em `Renderer.hpp`:**

```cpp
#include "ui/ConsoleFacade.hpp"
// ...
private:
    ConsoleFacade console_;
    // remover: void clearScreen(); void setCorEfeito(...); void resetarCor(); void setCorPorNome(...);
```

- [ ] **Em `Renderer.cpp`, substituir os métodos antigos:**

Remover as implementações de `clearScreen`, `setCorEfeito`, `resetarCor`, `setCorPorNome`.

Substituir cada chamada:

| Antes | Depois |
|---|---|
| `clearScreen()` | `console_.limpar()` |
| `SetConsoleTextAttribute(h, 14)` | `console_.setColor(Cor::Amarelo)` |
| `SetConsoleTextAttribute(h, 10)` | `console_.setColor(Cor::Verde)` |
| `SetConsoleTextAttribute(h, 11)` | `console_.setColor(Cor::Ciano)` |
| `SetConsoleTextAttribute(h, 12)` | `console_.setColor(Cor::Vermelho)` |
| `SetConsoleTextAttribute(h, 7)` | `console_.resetColor()` |
| `setCorEfeito(nomes)` | Lógica inline com `console_.setColor(...)` |
| `resetarCor()` | `console_.resetColor()` |
| `setCorPorNome(nome)` | `console_.setColor(corParaNome(nome))` — helper inline |

Para simplificar, manter um helper privado estático:

```cpp
// em Renderer.cpp (helper local, não precisa de header)
static Cor corDoEfeito(const std::string& nome) {
    if (nome == "Paralisia")       return Cor::Amarelo;
    if (nome == "Veneno")          return Cor::Verde;
    if (nome == "Regeneracao")     return Cor::Ciano;
    if (nome == "Enfraquecimento") return Cor::Vermelho;
    return Cor::Padrao;
}
```

- [ ] **No início de `render()`, chamar `console_.ocultarCursor()` em vez do bloco CONSOLE_CURSOR_INFO.**
- [ ] **Buildar (F7) e confirmar que o jogo funciona visualmente igual ao antes.**

---

## Chunk 2: EstadoJogo + Contadores

### Task 3: Adicionar EstadoJogo e contadores ao Game

**Files:**
- Modificar: `src/core/Game.hpp`
- Modificar: `src/core/Game.cpp`
- Modificar: `tests/test_game.cpp`

**Contexto:**  
`Game` só tem `isRunning_` para controle de fluxo. Precisamos de um estado explícito para cada "tela".  
O `inimigosDestruidos_` é necessário para a tela de morte.  
O `historicoLog_` guarda todas as mensagens da sessão.

- [ ] **Adicionar ao `Game.hpp`:**

```cpp
// Logo após os includes, antes da classe
enum class EstadoJogo {
    Menu,       // tela inicial
    Jogando,    // loop de jogo
    Morto,      // tela de derrota
    Historico   // log completo
};
```

```cpp
// Novos membros privados
EstadoJogo estado_;
int inimigosDestruidos_;
std::vector<std::string> historicoLog_;
```

- [ ] **No construtor `Game::Game()` em `Game.cpp`:**

```cpp
// Substituir:
: isRunning_(true)
// Por:
: isRunning_(true)
, estado_(EstadoJogo::Menu)
, inimigosDestruidos_(0)
, historicoLog_()
```

**Remover a chamada a `inicializarAndar()` do construtor** — o andar só deve ser inicializado quando o player escolher "Nova Partida" no menu, não ao construir o `Game`.

- [ ] **Em `pushMessage()`**, adicionar inserção no histórico:

```cpp
void Game::pushMessage(const std::string& msg) {
    messageLog_.push_front(msg);                // log do HUD (limitado)
    if (messageLog_.size() > 3)
        messageLog_.pop_back();
    historicoLog_.push_back(msg);               // histórico completo (ilimitado)
}
```

- [ ] **Em `update()`**, no bloco que detecta morte de inimigo (após `enemies_.erase`), incrementar o contador:

```cpp
inimigosDestruidos_++;
```

- [ ] **Em `update()`**, substituir a transição de morte do player:

```cpp
// Antes:
if (!player_.isAlive()) isRunning_ = false;

// Depois:
if (!player_.isAlive()) {
    estado_ = EstadoJogo::Morto;
}
```

- [ ] **Adicionar getters `protected` em `Game.hpp` para os testes:**

```cpp
EstadoJogo getEstado()            const { return estado_; }
int        getInimigosDestruidos() const { return inimigosDestruidos_; }
```

- [ ] **Adicionar testes em `tests/test_game.cpp`:**

```cpp
TEST_CASE("Game inicia no estado Menu") {
    GameTeste g;
    CHECK(g.getEstado() == EstadoJogo::Menu);
}

TEST_CASE("inimigosDestruidos começa em zero") {
    GameTeste g;
    CHECK(g.getInimigosDestruidos() == 0);
}
```

> Nota: Como o Game agora começa no Menu (sem `inicializarAndar()` no construtor), a subclasse `GameTeste` precisa chamar `inicializarAndar()` explicitamente se os testes existentes dependem de inimigos/itens. Ajustar conforme necessário.

- [ ] **Buildar (F7) e rodar testes (`roguelike_tests.exe`) — todos devem passar.**

---

## Chunk 3: Menu Inicial + Tela de Morte + Histórico

### Task 4: Renderer — três novas telas

**Files:**
- Modificar: `src/ui/Renderer.hpp`
- Modificar: `src/ui/Renderer.cpp`

**Contexto:**  
Três novos métodos de renderização — um para cada estado novo.  
Nenhum deles tem lógica de game, só visual.

- [ ] **Declarar em `Renderer.hpp`:**

```cpp
#include <vector>
// ...
void renderMenu(bool temSave);
void renderTelaDerrota(int andar, int nivel, int xp, int inimigos);
void renderHistorico(const std::vector<std::string>& historico);
```

- [ ] **Implementar `renderMenu` em `Renderer.cpp`:**

```cpp
void Renderer::renderMenu(bool temSave) {
    console_.limpar();
    console_.setColor(Cor::Magenta);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║          ROGUELIKE  v0.9             ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  [N] Nova Partida                    ║\n";
    if (temSave) {
        std::cout << "║  [C] Carregar Jogo                   ║\n";
    } else {
        console_.setColor(Cor::Padrao); // cinza — sem save disponível
        std::cout << "║  [C] Carregar Jogo  (sem save)       ║\n";
        console_.resetColor();
    }
    std::cout << "║  [ESC] Sair                          ║\n";
    console_.setColor(Cor::Magenta);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}
```

- [ ] **Implementar `renderTelaDerrota` em `Renderer.cpp`:**

```cpp
void Renderer::renderTelaDerrota(int andar, int nivel, int xp, int inimigos) {
    console_.limpar();
    console_.setColor(Cor::Vermelho);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║           VOCÊ MORREU                ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  Andar alcancado:   " << andar   << "\n";
    std::cout << "║  Nivel final:       " << nivel   << "\n";
    std::cout << "║  XP acumulado:      " << xp      << "\n";
    std::cout << "║  Inimigos mortos:   " << inimigos << "\n";
    console_.setColor(Cor::Vermelho);
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  [qualquer tecla] -> Menu            ║\n";
    console_.setColor(Cor::Vermelho);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}
```

- [ ] **Implementar `renderHistorico` em `Renderer.cpp`:**

```cpp
void Renderer::renderHistorico(const std::vector<std::string>& historico) {
    console_.limpar();
    console_.setColor(Cor::AzulClaro);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║       HISTÓRICO DE MENSAGENS         ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();

    // Exibe as últimas 20 mensagens (ou todas se < 20)
    const int MAX_LINHAS = 20;
    int inicio = static_cast<int>(historico.size()) - MAX_LINHAS;
    if (inicio < 0) inicio = 0;
    for (int i = inicio; i < static_cast<int>(historico.size()); ++i) {
        std::cout << "  > " << historico[i] << '\n';
    }

    console_.setColor(Cor::AzulClaro);
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "  [H / ESC] Fechar\n";
    console_.setColor(Cor::AzulClaro);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}
```

- [ ] **Buildar (F7) — sem erros.**

---

### Task 5: Game — despacho por estado

**Files:**
- Modificar: `src/core/Game.hpp`
- Modificar: `src/core/Game.cpp`

**Contexto:**  
`run()`, `render()` e `processInput()` precisam se comportar diferente em cada `EstadoJogo`.

- [ ] **Refatorar `Game::render()` para despachar por estado:**

```cpp
void Game::render() {
    switch (estado_) {
        case EstadoJogo::Menu:
            renderer_.renderMenu(SaveSystem::existeSave(SAVE_PATH));
            break;
        case EstadoJogo::Jogando:
            renderer_.render(map_, player_, enemies_, items_, messageLog_, inventarioAberto_, andarAtual_);
            break;
        case EstadoJogo::Morto:
            renderer_.renderTelaDerrota(andarAtual_, player_.getLevel(), player_.getXP(), inimigosDestruidos_);
            break;
        case EstadoJogo::Historico:
            renderer_.renderHistorico(historicoLog_);
            break;
    }
}
```

- [ ] **Refatorar `Game::processInput()` para despachar por estado:**

```cpp
void Game::processInput() {
    char key = readKey();

    switch (estado_) {

        case EstadoJogo::Menu:
            if (key == 27) { isRunning_ = false; return; }          // ESC
            if (key == 'n' || key == 'N') {
                inimigosDestruidos_ = 0;
                historicoLog_.clear();
                messageLog_.clear();
                // Reiniciar player e mapa para nova partida
                player_ = Player(2, 2);
                player_.adicionarObserver(&statsObserver_);
                player_.getInventario().onDescarte = [this](const std::string& msg){
                    logObserver_.onEvent(msg);
                };
                player_.onEfeitoEvento = [this](const std::string& msg){
                    pushMessage(msg);
                };
                andarAtual_ = 1;
                enemies_.clear();
                items_.clear();
                inicializarAndar();
                estado_ = EstadoJogo::Jogando;
            }
            if (key == 'c' || key == 'C') {
                if (SaveSystem::existeSave(SAVE_PATH)) {
                    carregar();
                    estado_ = EstadoJogo::Jogando;
                }
            }
            return;

        case EstadoJogo::Morto:
            estado_ = EstadoJogo::Menu;   // qualquer tecla volta ao menu
            return;

        case EstadoJogo::Historico:
            if (key == 27 || key == 'h' || key == 'H') {
                estado_ = EstadoJogo::Jogando;
            }
            return;

        case EstadoJogo::Jogando:
            if (key == 27) { isRunning_ = false; return; }
            if (key == 'S') { salvar();   return; }
            if (key == 'L') { carregar(); return; }
            if (key == 'h' || key == 'H') {
                estado_ = EstadoJogo::Historico;
                return;
            }
            key = static_cast<char>(tolower(key));
            // ... restante do switch existente (WASD, G, U, I, etc.)
            switch (key) {
                case 'w': player_.move( 0, -1, map_, enemies_); break;
                case 's': player_.move( 0,  1, map_, enemies_); break;
                case 'a': player_.move(-1,  0, map_, enemies_); break;
                case 'd': player_.move( 1,  0, map_, enemies_); break;
                case 'g': coletarItem(); break;
                case 'u': usarConsumivel(); break;
                case 'i': inventarioAberto_ = !inventarioAberto_; break;
                case '1': case '2': case '3': case '4': case '5':
                    if (inventarioAberto_) usarConsumivelInventario(key - '1'); break;
                case 'e': if (inventarioAberto_) equiparSelecionado(); break;
                case 'x': if (inventarioAberto_) desequiparSelecionado(); break;
                case 'r':
                    if (inventarioAberto_) {
                        char next = readKey();
                        if (next >= '1' && next <= '5')
                            descartarItem(next - '1');
                    }
                    break;
                default: break;
            }
            return;
    }
}
```

- [ ] **Ajustar `Game::run()` — remover o `if (!player_.isAlive())` do final** (já tratado pelo estado `Morto`):

```cpp
void Game::run() {
    while (isRunning_) {
        render();
        processInput();
        if (estado_ == EstadoJogo::Jogando)
            update();
    }
    std::cout << "\nAte a proxima aventura!\n";
}
```

- [ ] **Adicionar `SAVE_PATH` como constante no topo de `Game.cpp`** (se não existir já):

```cpp
static const std::string SAVE_PATH = "savegame.json";
```

- [ ] **Buildar (F7).**
- [ ] **Testar manualmente:**
  1. Executar → deve aparecer o menu
  2. `N` → inicia partida normal
  3. `H` → abre histórico de mensagens
  4. `H` novamente → fecha histórico
  5. `S` → salva; `ESC` → volta ao menu; `C` → carrega a partida salva
  6. Morrer → deve aparecer a tela de derrota com estatísticas
  7. Qualquer tecla na tela de morte → volta ao menu

- [ ] **Rodar testes (`roguelike_tests.exe`) — todos devem passar.**

---

## Chunk 4: Ajustes nos testes existentes

### Task 6: Adaptar test_game.cpp

**Files:**
- Modificar: `tests/test_game.cpp`

**Contexto:**  
`GameTeste` (subclasse de `Game` nos testes) pode precisar de ajustes porque o construtor de `Game` não chama mais `inicializarAndar()` diretamente — essa chamada acontece ao escolher "Nova Partida".

- [ ] **Verificar se `GameTeste` chama `inicializarAndar()` explicitamente no seu construtor.** Se os testes existentes de `descerAndar` / contagem de inimigos dependem do andar estar inicializado, adicionar a chamada:

```cpp
class GameTeste : public Game {
public:
    GameTeste() { inicializarAndar(); }  // ← inicializa manualmente para testes
    void render() override {}
    // getters de teste...
};
```

- [ ] **Rodar testes — todos devem passar (71+ testes).**

---

## Resumo de arquivos criados / modificados

| Arquivo | Ação |
|---|---|
| `src/ui/ConsoleFacade.hpp` | **Criar** — Facade de terminal |
| `src/ui/ConsoleFacade.cpp` | **Criar** — Implementação WIN32 + ANSI |
| `src/ui/Renderer.hpp` | **Modificar** — adicionar `console_`, `renderMenu`, `renderTelaDerrota`, `renderHistorico` |
| `src/ui/Renderer.cpp` | **Modificar** — usar ConsoleFacade; remover métodos antigos de cor; adicionar 3 novas telas |
| `src/core/Game.hpp` | **Modificar** — `EstadoJogo`, `estado_`, `inimigosDestruidos_`, `historicoLog_`, getters |
| `src/core/Game.cpp` | **Modificar** — construtor, `run()`, `processInput()`, `render()`, `update()`, `pushMessage()` |
| `tests/test_game.cpp` | **Modificar** — adaptar `GameTeste`, adicionar 2 testes de estado |

---

## Ordem de execução recomendada

1. Task 1 → Task 2 (ConsoleFacade — fundação visual)
2. Task 3 (EstadoJogo — fundação de fluxo)
3. Task 4 (Renderer — novas telas)
4. Task 5 (Game — despacho por estado)
5. Task 6 (Ajustes nos testes)

Cada task termina com build + testes passando antes de avançar.
