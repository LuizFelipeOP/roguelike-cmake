# Plano de Implementação — Phase 6: Múltiplos Andares

**Data:** 2026-04-17  
**Spec:** `docs/superpowers/specs/2026-04-17-phase6-andares.md`  
**Status:** Pendente  

---

## Visão geral

Implementar em 5 chunks independentes e sequenciais. Cada chunk compila e testa antes de avançar para o próximo.

---

## Chunk 1 — Escada no `Map`

**Conceito:** `Map` precisa gerar e armazenar a posição do tile `'>'`, expondo-a para o `Game` checar a colisão do player.

### `Map.hpp`
1. Adicionar campo privado:
   ```cpp
   Point escada_;
   ```
2. Declarar método público:
   ```cpp
   Point getPosicaoEscada() const;
   ```

### `Map.cpp`

3. Implementar `getPosicaoEscada()`:
   ```cpp
   Point Map::getPosicaoEscada() const { return escada_; }
   ```

4. Em `generate()`, após o loop de criação de salas (mas antes do `if(rooms_.empty())`), adicionar:
   - Se houver mais de uma sala, escolher uma sala aleatória com índice entre `1` e `rooms_.size() - 1`
   - Montar vetor com os 4 cantos internos dessa sala:
     ```
     (x+1, y+1), (x+width-2, y+1), (x+1, y+height-2), (x+width-2, y+height-2)
     ```
   - Sortear um dos quatro com `std::uniform_int_distribution<int>(0, 3)` usando o `rng` já existente
   - Escrever `'>'` nessa posição em `tiles_`
   - Salvar em `escada_`

> **Atenção:** o `rng` já existe em `generate()` — use o mesmo objeto para manter a reprodutibilidade da seed.

**Teste:** buildar e rodar. Ao explorar salas, o tile `>` deve aparecer em um canto. Não deve aparecer na sala inicial do player.

---

## Chunk 2 — `EnemyFactory` e subclasses recebem andar

**Conceito:** Goblin e Troll precisam de um construtor que aceite stats customizados. A Factory calcula os valores escalados e os passa.

### `Goblin.hpp` e `Troll.hpp`
1. Adicionar segundo construtor com stats explícitos:
   ```cpp
   Goblin(int x, int y, int hp, int atk, int def);
   Troll(int x, int y, int hp, int atk, int def);
   ```

### `Goblin.cpp` e `Troll.cpp`
2. Implementar o novo construtor delegando para `Enemy(x, y, hp, atk, def, xp)`.  
   O XP pode escalar também: `xpBase * andar` (calcular na Factory e passar).

### `EnemyFactory.hpp`
3. Atualizar assinatura:
   ```cpp
   static std::unique_ptr<Enemy> create(EnemyType type, int x, int y, int andar = 1);
   ```

### `EnemyFactory.cpp`
4. Calcular multiplicador: `float mult = 1.0f + (andar - 1) * 0.2f`
5. Aplicar nos stats base de cada tipo antes de chamar o novo construtor:
   ```
   Goblin base: hp=10, atk=3, def=1, xp=10
   Troll base:  hp=20, atk=5, def=2, xp=20
   ```
   Exemplo: `int hp = static_cast<int>(10 * mult)`

### `Game.cpp`
6. Atualizar as chamadas de `EnemyFactory::create` no construtor para passar `1` (ou omitir, pois default = 1)

**Teste:** buildar. Comportamento igual ao anterior no andar 1.

---

## Chunk 3 — `ItemFactory` recebe andar

**Conceito:** Itens melhores em andares mais profundos — valores das strategies escalam com o andar.

### `ItemFactory.hpp`
1. Atualizar assinatura:
   ```cpp
   static std::unique_ptr<Item> create(ItemType tipo, int x, int y, int andar = 1);
   ```

### `ItemFactory.cpp`
2. Usar `andar` para escalar os valores passados às strategies:
   - `CuraStrategy`: `valorBase * andar`
     - PocaoDeVidaPequena: base 10 → `10 * andar`
     - PocaoDeVida: base 20 → `20 * andar`
   - `AumentarATKStrategy`:
     - PocaoDeForça: base 1 → `1 + (andar - 1)`
     - Espada: base 2 → `2 + (andar - 1)`
     - EspadaGrande: base 4 → `4 + (andar - 1)`
   - `BonusDefStrategy` não recebe valor — sem mudança por enquanto

### `Game.cpp`
3. Atualizar chamadas de `ItemFactory::create` no construtor para passar `1` (ou omitir)

**Teste:** buildar. Comportamento igual ao anterior no andar 1.

---

## Chunk 4 — `descerAndar()` em `Game`

**Conceito:** Método que encapsula toda a transição entre andares — gera novo mapa, limpa estado antigo, reposiciona player e respawna entidades com o novo andar.

### `Game.hpp`
1. Adicionar campo privado:
   ```cpp
   int andarAtual_;
   ```
2. Declarar método privado:
   ```cpp
   void descerAndar();
   ```

### `Game.cpp` — construtor
3. Inicializar no construtor: `, andarAtual_(1)`
   > Atenção: a ordem de inicialização na lista deve seguir a ordem de declaração em `Game.hpp`.

### `Game.cpp` — `descerAndar()`
4. Implementar seguindo este fluxo:
   ```
   andarAtual_++
   map_.generate(seed_base + andarAtual_)   // use time(nullptr) + andarAtual_ como seed
   enemies_.clear()
   items_.clear()
   
   // reposicionar player
   Point start = map_.getRooms().front().center()
   player_.setX(start.x) / player_.setY(start.y)   // verifique se Player tem setX/setY
   
   // reconectar callback do inventário
   player_.getInventario().onDescarte = [this](const std::string& msg){
       logObserver_.onEvent(msg);
   };
   
   // respawnar inimigos e itens (mesma lógica do construtor, mas passando andarAtual_)
   
   map_.updateVisibility(player_.getX(), player_.getY())
   logObserver_.onEvent("Voce desceu para o andar " + std::to_string(andarAtual_))
   ```

> **Atenção:** verifique se `Player` tem `setX()` e `setY()`. Se não tiver, será necessário adicioná-los em `Player.hpp/.cpp` antes de continuar.

### `Game.cpp` — `update()`
5. Adicionar checagem ao final de `update()`, antes do `if (!player_.isAlive())`:
   ```cpp
   Point escada = map_.getPosicaoEscada();
   if (player_.getX() == escada.x && player_.getY() == escada.y) {
       descerAndar();
   }
   ```

**Teste:** buildar, explorar, encontrar `>`, pisar — novo mapa deve ser gerado e log deve exibir "Voce desceu para o andar 2".

---

## Chunk 5 — Andar no HUD (`Renderer`)

**Conceito:** Exibir o número do andar atual na barra de status.

### `Renderer.hpp`
1. Atualizar assinaturas:
   ```cpp
   void render(..., bool inventarioAberto, int andar);
   void renderHUD(const Player&, const std::deque<std::string>&, bool inventarioAberto, int andar);
   ```

### `Renderer.cpp`
2. Em `renderHUD`, adicionar exibição de `"Andar: " + std::to_string(andar)` junto ao HP/nível

### `Game.cpp` — `render()`
3. Atualizar chamada:
   ```cpp
   renderer_.render(map_, player_, enemies_, items_, messageLog_, inventarioAberto_, andarAtual_);
   ```

**Teste final:** buildar completo. HUD exibe "Andar: 1" no início e atualiza ao descer.

---

## Checklist de conclusão

- [ ] Chunk 1: tile `>` aparece em canto de sala não-inicial ao explorar
- [ ] Chunk 2: build sem erros; inimigos escalam visivelmente nos andares seguintes
- [ ] Chunk 3: build sem erros; itens com valores maiores nos andares seguintes
- [ ] Chunk 4: pisar na escada gera novo mapa, log confirma andar
- [ ] Chunk 5: HUD exibe número do andar corretamente
- [ ] Nenhum crash ao descer múltiplos andares seguidos
