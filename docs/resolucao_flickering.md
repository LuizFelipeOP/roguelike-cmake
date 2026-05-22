# Resolução do Flickering no Renderer

## O Problema

O método `limpar()` original usava `FillConsoleOutputCharacter` para preencher toda a janela do console com espaços antes de redesenhar cada frame. Isso causava um **flash branco visível** a cada atualização — a tela ficava em branco por alguns milissegundos antes do conteúdo reaparecer.

## Causa Raiz

O Windows Console API não tem double buffering nativo. Qualquer operação de limpeza total seguida de redesenho gera um frame intermediário vazio que é visível ao usuário.

## Solução Adotada

### Estratégia: Reposicionamento de cursor + preenchimento por linha

Em vez de apagar toda a tela, dividimos a responsabilidade em três operações:

| Método | Comportamento | Quando usar |
|---|---|---|
| `limpar()` | Apenas reposiciona o cursor para (0,0) | Todo frame — substitui o `FillConsoleOutputCharacter` global |
| `novaLinha()` | Preenche o resto da linha atual com espaços, depois `\n` | Toda quebra de linha no HUD, log e mapa |
| `limparTotal()` | `FillConsoleOutputCharacter` completo (limpeza real) | Apenas em transições de estado (ex: morte → menu) |

### Como funciona

1. `limpar()` move o cursor para (0,0) sem apagar nada.
2. O conteúdo novo é escrito por cima do conteúdo antigo, linha por linha.
3. `novaLinha()` preenche o restante de cada linha com espaços via `FillConsoleOutputCharacter` **localizado** — apaga apenas o excesso da linha atual, não a tela inteira.
4. Linhas mais longas do frame anterior são sobrescritas naturalmente pelo novo conteúdo.
5. `limparTotal()` é reservado para transições de estado, onde um flash momentâneo é aceitável.

### Trade-offs

**Vantagens:**
- Elimina o flickering completamente durante o jogo.
- Sem dependência de bibliotecas externas (ncurses, PDCurses etc.).
- Sem threads ou timers — solução puramente síncrona.

**Desvantagens:**
- Requer que **todas** as linhas do HUD/log/mapa usem `novaLinha()` em vez de `'\n'`.
- Se uma linha crescer e depois encolher entre frames, resíduos podem aparecer se `novaLinha()` não for chamado corretamente.
- `limparTotal()` em transições de estado ainda causa um flash, mas é aceitável nesse contexto.

## Arquivos Modificados

- `src/ui/ConsoleFacade.hpp` — declarações de `limparTotal()`, `novaLinha()`, `getLargura()`
- `src/ui/ConsoleFacade.cpp` — implementações; `limpar()` alterado para apenas `SetConsoleCursorPosition`
- `src/ui/Renderer.cpp` — todas as quebras de linha substituídas por `console_.novaLinha()`

## Alternativas Consideradas

- **PDCurses / ncurses**: eliminaria o problema na raiz com double buffering, mas adicionaria uma dependência externa e complexidade de setup no Windows.
- **WriteConsoleOutput em bloco**: escreve o frame inteiro de uma vez, eliminando o flash. Mais complexo de implementar (requer buffer de `CHAR_INFO`).
- **Virtual Terminal / ANSI**: usando `\033[H` para reposicionar cursor — equivalente ao que adotamos, mas via ANSI em vez de WIN32 API diretamente.
