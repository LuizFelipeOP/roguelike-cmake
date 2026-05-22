# Spec — Phase 9: Interface e Qualidade de Vida

**Data:** 2026-05-17  
**Status:** Aprovado

---

## Objetivo

Melhorar a experiência visual e de uso do roguelike com quatro funcionalidades:

1. **`ConsoleFacade`** — Fachada que centraliza todas as operações de cor e cursor do terminal
2. **Menu inicial** — Tela de entrada com opções Nova Partida / Carregar / Sair
3. **Tela de morte** — Resumo da run com estatísticas ao morrer
4. **Histórico de log** — Tela dedicada com todas as mensagens da sessão

---

## 1. ConsoleFacade — Pattern Facade

### Problema atual

`Renderer.cpp` mistura lógica de cor diretamente com lógica de renderização.  
Há `#ifdef _WIN32` / ANSI codes espalhados em três métodos distintos (`setCorEfeito`, `resetarCor`, `setCorPorNome`).  
Se mudarmos de terminal, temos que caçar cada `SetConsoleTextAttribute` manualmente.

### Solução

Criar `src/ui/ConsoleFacade.hpp` e `src/ui/ConsoleFacade.cpp` como **Fachada**:  
uma interface simples que esconde toda a complexidade de terminal multiplataforma.

```
Renderer  →  ConsoleFacade  →  WIN32 API / ANSI codes
```

### Interface pública

```cpp
// src/ui/ConsoleFacade.hpp
enum class Cor {
    Padrao,         // branco (reset)
    Amarelo,        // Paralisia
    Verde,          // Veneno
    Ciano,          // Regeneracao
    Vermelho,       // Enfraquecimento
    Magenta,        // destaque de menu
    AzulClaro,      // HUD info
};

class ConsoleFacade {
public:
    void setColor(Cor cor);
    void resetColor();
    void limpar();          // limpa a tela (substitui clearScreen)
    void ocultarCursor();   // oculta o cursor piscante
};
```

### Refactoring do Renderer

`Renderer` passa a conter uma instância `ConsoleFacade console_`.  
Os três métodos privados (`setCorEfeito`, `resetarCor`, `setCorPorNome`) são removidos  
e substituídos por chamadas à fachada.

---

## 2. EstadoJogo — Máquina de estados

### Problema atual

`Game` usa apenas `isRunning_` (bool). Não há distinção entre estar no menu, jogando, morto ou vendo o histórico.

### Solução

```cpp
// em Game.hpp
enum class EstadoJogo {
    Menu,       // tela inicial
    Jogando,    // loop principal
    Morto,      // tela de derrota
    Historico   // log completo
};
```

`processInput()` e `render()` despacham por estado.  
`isRunning_` é mantido para controlar o loop externo.

---

## 3. Menu Inicial

### Comportamento

Ao iniciar o jogo (antes do primeiro frame jogável), mostrar:

```
╔══════════════════════════════════════╗
║          ROGUELIKE  v0.9             ║
╠══════════════════════════════════════╣
║  [N] Nova Partida                    ║
║  [C] Carregar Jogo   (cinza se vazio)║
║  [ESC] Sair                          ║
╚══════════════════════════════════════╝
```

- `N` → inicia nova partida (`inicializarAndar()`), muda estado para `Jogando`
- `C` → só disponível se `SaveSystem::existeSave(SAVE_PATH)` for true; carrega e muda para `Jogando`
- `ESC` → `isRunning_ = false`

### Renderer

Novo método `Renderer::renderMenu(bool temSave)`.

---

## 4. Tela de Morte

### Comportamento

Quando `player_.isAlive() == false` em `update()`, mudar estado para `Morto`.  
O loop continua rodando (não sai imediatamente), mas `render()` mostra a tela de derrota.

```
╔══════════════════════════════════════╗
║           VOCÊ MORREU                ║ ← texto em vermelho
╠══════════════════════════════════════╣
║  Andar alcançado:    5               ║
║  Nível final:        3               ║
║  XP acumulado:       480             ║
║  Inimigos mortos:    12              ║
╠══════════════════════════════════════╣
║  [Enter / qualquer tecla] → Menu     ║
╚══════════════════════════════════════╝
```

Qualquer tecla → volta ao `EstadoJogo::Menu` (não reinicia automaticamente — player decide).

### Dados necessários

`Game` precisa de um contador `inimigosDestruidos_` (int) incrementado em `update()` cada vez que um inimigo morre.

### Renderer

Novo método `Renderer::renderTelaDerrota(int andar, int nivel, int xp, int inimigos)`.

---

## 5. Histórico Completo do Log

### Problema atual

`pushMessage()` mantém apenas as últimas 3 mensagens no log visível.  
O histórico completo da sessão é perdido.

### Solução

`Game` ganha um segundo container `historicoLog_` (`std::vector<std::string>`) sem limite de tamanho.  
`pushMessage()` insere em ambos: `messageLog_` (máx 3, para o HUD) e `historicoLog_` (ilimitado).

### Tecla de ativação

`H` (maiúsculo ou minúsculo) durante o jogo alterna `estado_` entre `Jogando` e `Historico`.

### Tela de histórico

```
╔══════════════════════════════════════╗
║         HISTÓRICO DE MENSAGENS       ║
╠══════════════════════════════════════╣
║  > Você atacou o Goblin por 8.       ║
║  > Goblin atacou você por 3.         ║
║  > Você subiu para o Nível 2!        ║
║  > Item coletado: Espada Curta.      ║
║  ...                                 ║
╠══════════════════════════════════════╣
║  [H / ESC] Fechar                    ║
╚══════════════════════════════════════╝
```

Mostra as últimas N mensagens que cabem na tela (sem scroll por ora).

### Renderer

Novo método `Renderer::renderHistorico(const std::vector<std::string>& historico)`.

---

## Padrões praticados nesta fase

| Pattern | Onde |
|---|---|
| **Facade** | `ConsoleFacade` encapsula WIN32 API + ANSI codes |
| **State** (informal) | `EstadoJogo` enum + dispatch por estado em `processInput` / `render` |

---

## O que NÃO está no escopo da Phase 9

- Scroll no histórico (Phase 10+)
- Placar de pontuação persistido (Phase 10)
- Animações ou efeitos visuais além de cor
- Múltiplos temas de cor
