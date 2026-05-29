#include "ui/SDLRenderer.hpp"
#include "ui/SalaThema.hpp"
#include "map/Map.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "items/Item.hpp"
#include "items/Inventario.hpp"
#include <string>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Construtor — carrega uma textura por SpriteSheet a partir do diretório base
// ─────────────────────────────────────────────────────────────────────────────

SDLRenderer::SDLRenderer(SDL_Renderer* renderer,
                         const std::string& spriteDir,
                         const std::string& fontPath)
    : renderer_(renderer)
{
    TTF_Init();
    font_      = TTF_OpenFont(fontPath.c_str(), 8);
    fontSmall_ = TTF_OpenFont(fontPath.c_str(), 6);

    if (!font_)      SDL_Log("ERRO font: %s", TTF_GetError());
    if (!fontSmall_) SDL_Log("ERRO fontSmall: %s", TTF_GetError());

    if (!renderer_) return;

    // Monta os caminhos e carrega cada sheet — falha individual não impede os outros
    const struct { SpriteSheet sheet; const char* arquivo; } entradas[] = {
        { SpriteSheet::Monsters,       "Monsters.png"       },
        { SpriteSheet::Items,          "Items.png"          },
        { SpriteSheet::Terrain,        "Terrain.png"        },
        { SpriteSheet::TerrainObjects, "Terrain_Objects.png"},
        { SpriteSheet::Interface,      "Interface.png"      },
    };
    for (const auto& e : entradas) {
        std::string caminho = spriteDir + e.arquivo;
        SDL_Texture* tex = IMG_LoadTexture(renderer_, caminho.c_str());
        if (!tex)
            SDL_Log("AVISO: sheet '%s' nao carregado: %s", e.arquivo, IMG_GetError());
        sheets_[e.sheet] = tex; // nullptr é válido — desenharTile usa fallback
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Destrutor — libera todas as texturas e fontes
// ─────────────────────────────────────────────────────────────────────────────

SDLRenderer::~SDLRenderer() {
    for (auto& [sheet, tex] : sheets_)
        if (tex) SDL_DestroyTexture(tex);
    if (font_)      TTF_CloseFont(font_);
    if (fontSmall_) TTF_CloseFont(fontSmall_);
    TTF_Quit();
}

// ─────────────────────────────────────────────────────────────────────────────
// desenharTileInterface — desenha tile do Interface.png por col/row (16×24 px)
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::desenharTileInterface(int col, int row, int pixelX, int pixelY, int escala) {
    auto it = sheets_.find(SpriteSheet::Interface);
    if (it == sheets_.end() || !it->second) return;
    const int TW = 16, TH = 24;
    SDL_Rect src = { col * TW, row * TH, TW, TH };
    SDL_Rect dst = { pixelX, pixelY, TW * escala, TH * escala };
    SDL_RenderCopy(renderer_, it->second, &src, &dst);
}

// ─────────────────────────────────────────────────────────────────────────────
// clear / present
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::clear() {
    if (!renderer_) return;
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
}

void SDLRenderer::present() {
    if (!renderer_) return;
    SDL_RenderPresent(renderer_);
}

// ─────────────────────────────────────────────────────────────────────────────
// corDoTile — retorna r,g,b base para o TileID
// ─────────────────────────────────────────────────────────────────────────────
static void corDoTile(TileID id, Uint8& r, Uint8& g, Uint8& b) {
    switch (id) {
        case TileID::PLAYER:        r= 49; g= 68; b= 74; break; // #31444a
        case TileID::GOBLIN:        r= 65; g=106; b= 93; break; // #416a5d
        case TileID::ORC:           r=191; g=141; b=119; break; // #bf8d77
        case TileID::TROLL:         r= 48; g= 43; b= 37; break; // #302b25
        case TileID::PAREDE:        r=139; g= 72; b= 74; break; // #8b484a
        case TileID::PAREDE_LATERAL:r=100; g= 85; b= 80; break; // tom neutro para corredor
        case TileID::CHAO:          r=125; g= 95; b=111; break; // #7d5f6f
        case TileID::ESCADA:        r=119; g=151; b= 88; break; // #779758
        case TileID::ITEM_POCAO:    r=139; g= 72; b= 74; break; // #8b484a
        case TileID::ITEM_ESPADA:   r=197; g=164; b=107; break; // #c5a46b
        case TileID::ITEM_ARMADURA: r=123; g=125; b=106; break; // #7b7d6a
        case TileID::ITEM_AMULETO:  r= 98; g=124; b=128; break; // #627c80
        default:                    r=255; g=255; b=255; break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// aplicarCorTile — tinta o sprite via SDL_SetTextureColorMod com brilho
// ─────────────────────────────────────────────────────────────────────────────
static void aplicarCorTile(SDL_Texture* tex, TileID id, Uint8 brilho) {
    Uint8 r, g, b;
    corDoTile(id, r, g, b);
    SDL_SetTextureColorMod(tex,
        static_cast<Uint8>(r * brilho / 255),
        static_cast<Uint8>(g * brilho / 255),
        static_cast<Uint8>(b * brilho / 255));
}

// ─────────────────────────────────────────────────────────────────────────────
// desenharTile — consulta TileAtlas, seleciona a textura correta
// ─────────────────────────────────────────────────────────────────────────────
void SDLRenderer::desenharTile(TileID id, int pixelX, int pixelY, Uint8 brilho) {
    if (!renderer_) return;

    TileInfo info = TileAtlas::getInfo(id);
    SDL_Rect dst  = { pixelX, pixelY, TileAtlas::TILE_W, TileAtlas::TILE_H };

    // Tenta encontrar a textura do sheet correto
    auto it  = sheets_.find(info.sheet);
    SDL_Texture* tex = (it != sheets_.end()) ? it->second : nullptr;

    if (!tex) {
        // Fallback: retângulo colorido por tipo quando o sheet não carregou
        switch (id) {
            case TileID::PLAYER:        SDL_SetRenderDrawColor(renderer_, 106, 52, 37, 1); break;
            case TileID::PAREDE:        SDL_SetRenderDrawColor(renderer_, 100, 100, 100, 255); break;
            case TileID::CHAO:          SDL_SetRenderDrawColor(renderer_,  69, 47, 41, 1); break;
            case TileID::ESCADA:        SDL_SetRenderDrawColor(renderer_, 100, 200, 100, 255); break;
            case TileID::GOBLIN:        SDL_SetRenderDrawColor(renderer_,   0, 255,   0, 255); break;
            case TileID::ORC:           SDL_SetRenderDrawColor(renderer_, 255, 165,   0, 255); break;
            case TileID::TROLL:         SDL_SetRenderDrawColor(renderer_, 200,   0,   0, 255); break;
            case TileID::ITEM_POCAO:    SDL_SetRenderDrawColor(renderer_,   0, 180, 255, 255); break;
            case TileID::ITEM_ESPADA:   SDL_SetRenderDrawColor(renderer_, 200, 200,  80, 255); break;
            case TileID::ITEM_ARMADURA: SDL_SetRenderDrawColor(renderer_, 160, 160, 255, 255); break;
            case TileID::ITEM_AMULETO:  SDL_SetRenderDrawColor(renderer_, 255, 100, 200, 255); break;
            default:                    SDL_SetRenderDrawColor(renderer_, 180, 180, 180, 255); break;
        }
        SDL_RenderFillRect(renderer_, &dst);
        return;
    }

    aplicarCorTile(tex, id, brilho);
    SDL_RenderCopy(renderer_, tex, &info.rect, &dst);
    SDL_SetTextureColorMod(tex, 255, 255, 255); // reseta para branco
}

// ─────────────────────────────────────────────────────────────────────────────
// renderTileParede — escolhe sprite de parede por orientação
// ─────────────────────────────────────────────────────────────────────────────
// Parede HORIZONTAL (tem chão acima OU abaixo): thema + variantes + cor
//   → top da sala (chão abaixo)  → t.paredeFrente
//   → base da sala (chão acima)  → t.paredeFrente (mesmo sprite, visão simétrica)
// Parede VERTICAL (nenhum chão acima/abaixo): neutro (0,2) + cor do thema
//   → lados de sala e corredores verticais
// ─────────────────────────────────────────────────────────────────────────────
void SDLRenderer::renderTileParede(const Map& mapa, int x, int y,
                                   int pixelX, int pixelY, Uint8 brilho) {
    const SalaThema& t = SalaThemaFactory::getThema(mapa.getEstilo());

    bool chaoAbaixo = (mapa.getTile(x, y + 1) == '.');
    bool chaoAcima  = (mapa.getTile(x, y - 1) == '.');
    bool horizontal = chaoAbaixo || chaoAcima;
    int  v          = mapa.getWallVariant(x, y);

    // Canto adjacente a uma parede horizontal: herda o estilo thema
    // para não quebrar a continuidade visual da borda.
    if (!horizontal) {
        for (int d : { -1, 1 }) {
            char viz = mapa.getTile(x + d, y);
            if (viz == '#'
                    && (mapa.getTile(x + d, y - 1) == '.'
                        || mapa.getTile(x + d, y + 1) == '.')) {
                horizontal = true;
                break;
            }
        }
    }

    // Parede horizontal com parede vertical diretamente abaixo:
    // torna simples para manter continuidade com a parede abaixo.
    // Não se aplica a paredes com chão acima (base de sala) — essas
    // nunca devem ser simples independente do que há abaixo.
    if (horizontal && !chaoAcima && !chaoAbaixo
            && mapa.getTile(x, y + 1) == '#'
            && mapa.getTile(x, y + 2) != '.') {
        horizontal = false;
    }

    SDL_Rect src;
    if (horizontal) {
        // Parede horizontal (topo ou base de sala): usa thema com variantes
        src = t.paredeFrente;
        if (v >= 5 && t.numVariantes > 0)
            src = t.variantes[(v - 5) % t.numVariantes];
    } else {
        // Parede vertical (lado de sala ou corredor vertical): neutro (0,2) + cor
        src = TileAtlas::getInfo(TileID::PAREDE_LATERAL).rect;
    }

    auto it = sheets_.find(SpriteSheet::Terrain);
    if (!it->second) { desenharTile(TileID::PAREDE_LATERAL, pixelX, pixelY, brilho); return; }

    SDL_Rect dst = { pixelX, pixelY, TileAtlas::TILE_W, TileAtlas::TILE_H };
    SDL_SetTextureColorMod(it->second,
        static_cast<Uint8>(t.corR * brilho / 255),
        static_cast<Uint8>(t.corG * brilho / 255),
        static_cast<Uint8>(t.corB * brilho / 255));
    SDL_RenderCopy(renderer_, it->second, &src, &dst);
    SDL_SetTextureColorMod(it->second, 255, 255, 255);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mapeamento TileID ← tipo de entidade
// ─────────────────────────────────────────────────────────────────────────────

TileID SDLRenderer::tileIDParaInimigo(const Enemy& e) const {
    std::string nome = e.getTipoNome();
    if (nome == "Goblin") return TileID::GOBLIN;
    if (nome == "Orc")    return TileID::ORC;
    if (nome == "Troll")  return TileID::TROLL;
    return TileID::GOBLIN;
}

TileID SDLRenderer::tileIDParaItem(const Item& item) const {
    switch (item.getTipo()) {
        case ItemType::PocaoDeVidaPequena: return TileID::ITEM_POCAO;
        case ItemType::PocaoDeVida:        return TileID::ITEM_POCAO;
        case ItemType::PocaoDeForça:       return TileID::ITEM_POCAO;
        case ItemType::PocaoDeRegeneracao: return TileID::ITEM_POCAO;
        case ItemType::Espada:             return TileID::ITEM_ESPADA;
        case ItemType::EspadaGrande:       return TileID::ITEM_ESPADA;
        case ItemType::Armadura:           return TileID::ITEM_ARMADURA;
        case ItemType::Amuleto:            return TileID::ITEM_AMULETO;
        default:                           return TileID::ITEM_POCAO;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderMapa — terreno + inimigos + itens (fog of war)
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::renderMapa(const Map& mapa,
                              const std::vector<std::unique_ptr<Enemy>>& inimigos,
                              const std::vector<std::unique_ptr<Item>>& itens,
                              const Player& player)
{
    for (int y = 0; y < mapa.getHeight(); ++y) {
        for (int x = 0; x < mapa.getWidth(); ++x) {
            if (!mapa.isExplored(x, y)) continue;

            int pixelX = x * TileAtlas::TILE_W;
            int pixelY = y * TileAtlas::TILE_H;

            bool visivel = mapa.isVisible(x, y);
            Uint8 brilho = visivel ? 255 : 80;

            char tile = mapa.getTile(x, y);

            if (tile == '#') {
                // Parede: delega ao helper que escolhe sprite por contexto
                renderTileParede(mapa, x, y, pixelX, pixelY, brilho);
            } else if (tile == '>') {
                desenharTile(TileID::ESCADA, pixelX, pixelY, brilho);
            } else {
                 // Chão: usa SalaThema do andar para cor e sprite
                 // Salas: usa t.chao (sprite específico do thema)
                 // Corredores: usa CHAO padrão mas com a mesma cor do thema
                 int roomIdx = mapa.getRoomIndex(x, y);
                 const SalaThema& t = SalaThemaFactory::getThema(mapa.getEstilo());
                 auto it = sheets_.find(SpriteSheet::Terrain);
                 if (it != sheets_.end() && it->second) {
                     SDL_Rect dst = { pixelX, pixelY, TileAtlas::TILE_W, TileAtlas::TILE_H };
                     SDL_SetTextureColorMod(it->second,
                         static_cast<Uint8>(t.corR * brilho / 255),
                         static_cast<Uint8>(t.corG * brilho / 255),
                         static_cast<Uint8>(t.corB * brilho / 255));
                     // Sala: sprite de chão do thema; Corredor: CHAO padrão
                     SDL_Rect srcChao = (roomIdx >= 0)
                         ? t.chao
                         : TileAtlas::getInfo(TileID::CHAO).rect;
                     SDL_RenderCopy(renderer_, it->second, &srcChao, &dst);
                     SDL_SetTextureColorMod(it->second, 255, 255, 255);
                 } else {
                     desenharTile(TileID::CHAO, pixelX, pixelY, brilho);
                 }
            }

            if (!visivel) continue; // entidades só aparecem em tiles visíveis

            for (const auto& inimigo : inimigos) {
                if (inimigo->isAlive() && inimigo->getX() == x && inimigo->getY() == y)
                    desenharTile(tileIDParaInimigo(*inimigo), pixelX, pixelY);
            }
            for (const auto& item : itens) {
                if (item->getX() == x && item->getY() == y)
                    desenharTile(tileIDParaItem(*item), pixelX, pixelY);
            }
        }
    }

    desenharTile(TileID::PLAYER,
                 player.getX() * TileAtlas::TILE_W,
                 player.getY() * TileAtlas::TILE_H);
}

// ─────────────────────────────────────────────────────────────────────────────
// desenharSlot — moldura de slot de inventário + ícone (ou vazio)
// ─────────────────────────────────────────────────────────────────────────────
static void desenharSlotVazio(SDL_Renderer* r, int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(r, 50, 40, 35, 255);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 90, 70, 60, 255);
    SDL_RenderDrawRect(r, &bg);
}

static void desenharSlotAtivo(SDL_Renderer* r, int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(r, 40, 55, 45, 255);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 119, 151, 88, 255); // #779758 — verde destaque
    SDL_RenderDrawRect(r, &bg);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderInventario — painel completo sobre o mapa
// ─────────────────────────────────────────────────────────────────────────────
void SDLRenderer::renderInventario(const Inventario& inv) {
    if (!renderer_) return;

    // ── Overlay escuro semitransparente ──────────────────────────────────────
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 170);
    SDL_Rect tela = {0, 0, WIN_W, WIN_H};
    SDL_RenderFillRect(renderer_, &tela);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);

    // ── Painel central ───────────────────────────────────────────────────────
    const int PW = 760, PH = 390;
    const int PX = (WIN_W - PW) / 2;
    const int PY = (WIN_H - PH) / 2;

    // Fundo do painel
    SDL_SetRenderDrawColor(renderer_, 34, 26, 12, 255); // #221a0c
    SDL_Rect painel = {PX, PY, PW, PH};
    SDL_RenderFillRect(renderer_, &painel);

    // ── Frame ornamentado com tiles do Interface.png ──────────────────────────
    // Tiles 16×24 confirmados:
    //   Cantos: TL(0,0) TR(1,0) BL(2,0) BR(3,0)
    //   Borda H: (4,0) — top e bottom
    //   Borda V esq: (5,0)  dir: (5,1)
    const int TW = 16, TH = 24;
    const int escBorda = 1; // escala 1× para a borda

    // Bordas horizontais (top e bottom)
    for (int x = PX + TW; x < PX + PW - TW; x += TW) {
        desenharTileInterface(4, 0, x, PY, escBorda);           // topo
        desenharTileInterface(4, 0, x, PY + PH - TH, escBorda); // base
    }
    // Bordas verticais (left e right)
    for (int y = PY + TH; y < PY + PH - TH; y += TH) {
        desenharTileInterface(5, 0, PX, y, escBorda);            // esquerda
        desenharTileInterface(5, 1, PX + PW - TW, y, escBorda);  // direita
    }
    // Cantos
    desenharTileInterface(0, 0, PX,           PY,           escBorda); // TL
    desenharTileInterface(1, 0, PX + PW - TW, PY,           escBorda); // TR
    desenharTileInterface(2, 0, PX,           PY + PH - TH, escBorda); // BL
    desenharTileInterface(3, 0, PX + PW - TW, PY + PH - TH, escBorda); // BR

    // ── Cores ────────────────────────────────────────────────────────────────
    SDL_Color cor_titulo = {197, 164, 107, 255}; // #c5a46b — dourado
    SDL_Color cor_nome   = {205, 190, 172, 255}; // #cdbeac — texto claro
    SDL_Color cor_vazio  = {90,  80,  70,  255};
    SDL_Color cor_tecla  = {98, 124, 128, 255};  // #627c80 — azul acinzentado

    // ── Título central com ornamento ─────────────────────────────────────────
    int titY = PY + TH + 4;
    desenharTileInterface(1, 4, PX + PW/2 - 60, titY, 1); // ornamento esq
    drawText("INVENTARIO", PX + PW/2 - 36, titY + 8, cor_titulo);
    desenharTileInterface(1, 4, PX + PW/2 + 36, titY, 1); // ornamento dir

    // ── Dimensões dos ícones (escala 2×) ─────────────────────────────────────
    const int ICO_W = TILE_W * 2; // 32
    const int ICO_H = TILE_H * 2; // 48
    const int SLOT_W = ICO_W + 8;
    const int SLOT_H = ICO_H + 8;

    // ── SEÇÃO EQUIPAMENTOS (esquerda) ────────────────────────────────────────
    int eqX = PX + TW + 8;
    int eqY = PY + TH + 36;

    // Ícone de escudo + label
    desenharTileInterface(0, 5, eqX, eqY, 1); // escudo
    drawText("EQUIPAMENTOS", eqX + TW + 4, eqY + 8, cor_titulo);
    eqY += TH + 8;

    const struct { ItemSlot slot; const char* label; } slots_eq[] = {
        { ItemSlot::Arma,     "Arma"     },
        { ItemSlot::Armadura, "Armadura" },
        { ItemSlot::Acessorio,"Acessorio"},
    };

    for (const auto& se : slots_eq) {
        const Item* eq = inv.getEquipado(se.slot);
        if (eq) {
            desenharSlotAtivo(renderer_, eqX, eqY, SLOT_W, SLOT_H);
            TileInfo info = TileAtlas::getInfo(tileIDParaItem(*eq));
            auto it = sheets_.find(info.sheet);
            if (it != sheets_.end() && it->second) {
                Uint8 r2, g2, b2;
                corDoTile(tileIDParaItem(*eq), r2, g2, b2);
                SDL_SetTextureColorMod(it->second, r2, g2, b2);
                SDL_Rect dst = {eqX + 4, eqY + 4, ICO_W, ICO_H};
                SDL_RenderCopy(renderer_, it->second, &info.rect, &dst);
                SDL_SetTextureColorMod(it->second, 255, 255, 255);
            }
            drawText(eq->getNome(), eqX + SLOT_W + 6, eqY + 16, cor_nome);
        } else {
            // Slot vazio: fundo escuro + tile de slot vazio do Interface
            desenharSlotVazio(renderer_, eqX, eqY, SLOT_W, SLOT_H);
            desenharTileInterface(6, 2, eqX + (SLOT_W - TW)/2, eqY + (SLOT_H - TH)/2, 1);
            drawText(se.label, eqX + SLOT_W + 6, eqY + 16, cor_vazio);
        }
        eqY += SLOT_H + 20;
    }

    // ── Separador vertical ───────────────────────────────────────────────────
    int sepX = PX + PW / 2 - 20;
    for (int y = PY + TH + 36; y < PY + PH - TH - 30; y += TH)
        desenharTileInterface(1, 4, sepX, y, 1);

    // ── SEÇÃO CONSUMÍVEIS (direita) ───────────────────────────────────────────
    int cX = PX + PW / 2 + 10;
    int cY = PY + TH + 36;

    // Ícone de poção + label
    desenharTileInterface(7, 5, cX, cY, 1); // poção
    drawText("CONSUMIVEIS", cX + TW + 4, cY + 8, cor_titulo);
    cY += TH + 8;

    const auto& cons = inv.getConsumiveis();
    for (int i = 0; i < 5; ++i) {
        int sx = cX + i * (SLOT_W + 8);
        if (i < static_cast<int>(cons.size())) {
            desenharSlotAtivo(renderer_, sx, cY, SLOT_W, SLOT_H);
            TileID tid = tileIDParaItem(*cons[i]);
            TileInfo info = TileAtlas::getInfo(tid);
            auto it = sheets_.find(info.sheet);
            if (it != sheets_.end() && it->second) {
                Uint8 r2, g2, b2;
                corDoTile(tid, r2, g2, b2);
                SDL_SetTextureColorMod(it->second, r2, g2, b2);
                SDL_Rect dst = {sx + 4, cY + 4, ICO_W, ICO_H};
                SDL_RenderCopy(renderer_, it->second, &info.rect, &dst);
                SDL_SetTextureColorMod(it->second, 255, 255, 255);
            }
            std::string num = std::to_string(i + 1);
            drawText(num, sx + 2, cY + 2, cor_tecla);
            drawText(cons[i]->getNome(), sx, cY + SLOT_H + 2, cor_nome);
        } else {
            desenharSlotVazio(renderer_, sx, cY, SLOT_W, SLOT_H);
            desenharTileInterface(6, 2, sx + (SLOT_W - TW)/2, cY + (SLOT_H - TH)/2, 1);
            std::string num = std::to_string(i + 1);
            drawText(num, sx + 2, cY + 2, cor_vazio);
        }
    }

    // ── Legenda de teclas ─────────────────────────────────────────────────────
    int legY = PY + PH - TH - 4;

    drawText("[1-5]Usar  [E]Equipar  [X]Desequipar  [R]Descartar  [I]Fechar",
             PX + TW + 8, legY, cor_tecla);
}

// ─────────────────────────────────────────────────────────────────────────────
// drawText
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::drawText(const std::string& texto, int x, int y, SDL_Color cor) {
    if (!font_ || !renderer_) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font_, texto.c_str(), cor);
    if (!surface) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer_, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderHUD
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::renderHUD(const Player& player,
                             const std::deque<std::string>& messageLog,
                             int andarAtual)
{
    if (!renderer_) return;
    SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
    SDL_Rect linha = { 0, HUD_Y, WIN_W, 2 };
    SDL_RenderFillRect(renderer_, &linha);

    SDL_Color branco = {255, 255, 255, 255};
    SDL_Color cinza  = {180, 180, 180, 255};

    std::string stats =
        "HP:" + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) +
        " ATK:" + std::to_string(player.getAttack()) +
        " DEF:" + std::to_string(player.getDefense()) +
        " Andar:" + std::to_string(andarAtual) +
        " Lv:" + std::to_string(player.getLevel()) +
        " XP:" + std::to_string(player.getXP());
    drawText(stats, 8, HUD_Y + 8, branco);

    int logY = HUD_Y + 30;
    for (const auto& msg : messageLog) {
        drawText(msg, 8, logY, cinza);
        logY += 18;
        if (logY > WIN_H - 10) break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// render
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::render(const Map& mapa,
                         const Player& player,
                         const std::vector<std::unique_ptr<Enemy>>& inimigos,
                         const std::vector<std::unique_ptr<Item>>& itens,
                         const std::deque<std::string>& messageLog,
                         bool inventarioAberto,
                         int andarAtual,
                         const Inventario* inv)
{
    if (!renderer_) return;
    renderMapa(mapa, inimigos, itens, player);
    renderHUD(player, messageLog, andarAtual);
    if (inventarioAberto && inv)
        renderInventario(*inv);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderMenu
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::renderMenu(bool temSave) {
    if (!renderer_) return;

    SDL_Color amarelo  = {255, 220,   0, 255};
    SDL_Color branco   = {255, 255, 255, 255};
    SDL_Color cinza    = {140, 140, 140, 255};
    SDL_Color vermelho = {220,  60,  60, 255};

    drawText("ROGUELIKE",              WIN_W / 2 - 72,  180, amarelo);
    drawText("- - - - - - - - - - -",  WIN_W / 2 - 88,  210, cinza);
    drawText("[N] Nova Partida",        WIN_W / 2 - 64,  280, branco);

    if (temSave)
        drawText("[C] Continuar",       WIN_W / 2 - 52,  320, branco);
    else
        drawText("[C] Continuar",       WIN_W / 2 - 52,  320, cinza);

    drawText("[ESC] Sair",              WIN_W / 2 - 40,  360, vermelho);
    drawText("WASD mover  G coletar  U usar  I inventario",
             WIN_W / 2 - 172, WIN_H - 40, cinza);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderTelaDerrota
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::renderTelaDerrota(int andar, int level, int xp, int inimigosDestruidos) {
    if (!renderer_) return;

    SDL_Color vermelho = {220,  60,  60, 255};
    SDL_Color branco   = {255, 255, 255, 255};
    SDL_Color cinza    = {140, 140, 140, 255};

    drawText("VOCE MORREU",            WIN_W / 2 - 44,  200, vermelho);
    drawText("- - - - - - - - - - -",  WIN_W / 2 - 88,  230, cinza);

    drawText(("Andar:    " + std::to_string(andar)).c_str(),              WIN_W / 2 - 60, 290, branco);
    drawText(("Nivel:    " + std::to_string(level)).c_str(),              WIN_W / 2 - 60, 318, branco);
    drawText(("XP:       " + std::to_string(xp)).c_str(),                 WIN_W / 2 - 60, 346, branco);
    drawText(("Inimigos: " + std::to_string(inimigosDestruidos)).c_str(), WIN_W / 2 - 60, 374, branco);

    drawText("[H] Historico  [M] Menu",  WIN_W / 2 - 96,  440, cinza);
}

// ─────────────────────────────────────────────────────────────────────────────
// renderHistorico
// ─────────────────────────────────────────────────────────────────────────────

void SDLRenderer::renderHistorico(const std::vector<std::string>& log) {
    if (!renderer_) return;

    SDL_Color amarelo = {255, 220,   0, 255};
    SDL_Color cinza   = {180, 180, 180, 255};
    SDL_Color escuro  = {100, 100, 100, 255};

    drawText("HISTORICO DE MENSAGENS", 8,  8, amarelo);
    drawText("[H / ESC] Fechar",       8, 28, escuro);

    int y = 58;
    for (int i = static_cast<int>(log.size()) - 1; i >= 0; --i) {
        drawText(log[i], 8, y, cinza);
        y += 18;
        if (y > WIN_H - 20) break;
    }
}
