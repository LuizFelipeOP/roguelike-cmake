#pragma once
// SDLRenderer.hpp — Renderer gráfico usando SDL2 + múltiplos spritesheets
//
// Cada categoria de tile usa seu próprio PNG (Monsters, Items, Terrain, etc.).
// O SDLRenderer carrega uma SDL_Texture* por SpriteSheet e as indexa em um map.
// Não é dono do SDL_Renderer* (criado em main.cpp), mas é dono das SDL_Texture*.

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <memory>
#include <deque>
#include <map>
#include <string>
#include "ui/TileAtlas.hpp"
#include "ui/TileID.hpp"
#include "ui/SpriteSheet.hpp"

// Forward declarations — evita incluir headers pesados aqui
class Map;
class Player;
class Enemy;
class Item;
class Inventario;

// Dimensões da janela e tiles — usadas por SDLRenderer e main.cpp
static constexpr int TILE_W   = 16;
static constexpr int TILE_H   = 24;
static constexpr int MAP_COLS = 60;
static constexpr int MAP_ROWS = 22;
static constexpr int HUD_Y    = MAP_ROWS * TILE_H;  // 528
static constexpr int WIN_W    = MAP_COLS * TILE_W;  // 960
static constexpr int WIN_H    = HUD_Y + 120;        // 648

class SDLRenderer {
public:
    // renderer:   ponteiro não-owning para o SDL_Renderer criado em main.cpp
    // spriteDir:  diretório base dos spritesheets, ex: "assets/sprites/"
    // fontPath:   caminho para a fonte TTF
    SDLRenderer(SDL_Renderer* renderer,
                const std::string& spriteDir,
                const std::string& fontPath);
    ~SDLRenderer();

    // Não copiável — SDL_Texture é recurso exclusivo
    SDLRenderer(const SDLRenderer&)            = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

    void renderInventario(const Inventario& inv);

    void clear();
    void present();

    void drawText(const std::string& texto, int x, int y, SDL_Color cor);

    void render(const Map& mapa,
                const Player& player,
                const std::vector<std::unique_ptr<Enemy>>& inimigos,
                const std::vector<std::unique_ptr<Item>>& itens,
                const std::deque<std::string>& messageLog,
                bool inventarioAberto,
                int andarAtual,
                const Inventario* inv = nullptr);

    void renderMenu(bool temSave);
    void renderTelaDerrota(int andar, int level, int xp, int inimigos);
    void renderHistorico(const std::vector<std::string>& log);

private:
    SDL_Renderer* renderer_;  // não-owning

    // Uma textura por spritesheet — owning (destruídas no destrutor)
    std::map<SpriteSheet, SDL_Texture*> sheets_;

    TTF_Font* font_;
    TTF_Font* fontSmall_;

    // Helpers internos
    void renderMapa(const Map& mapa,
                    const std::vector<std::unique_ptr<Enemy>>& inimigos,
                    const std::vector<std::unique_ptr<Item>>& itens,
                    const Player& player);

    void renderHUD(const Player& player,
                   const std::deque<std::string>& messageLog,
                   int andarAtual);

    // Desenha um tile na posição de tela (pixelX, pixelY)
    // brilho: 0-255 — controla SDL_SetTextureColorMod para fog of war (255 = pleno)
    void desenharTile(TileID id, int pixelX, int pixelY, Uint8 brilho = 255);

    // Desenha um tile do Interface.png diretamente por col/row (tiles 16×24)
    // escala: 1 = tamanho original, 2 = dobro, etc.
    void desenharTileInterface(int col, int row, int pixelX, int pixelY, int escala = 1);

    // Desenha tile de parede escolhendo sprite por contexto (sala vs corredor)
    void renderTileParede(const Map& mapa, int x, int y,
                          int pixelX, int pixelY, Uint8 brilho);

    TileID tileIDParaInimigo(const Enemy& e) const;
    TileID tileIDParaItem(const Item& item) const;
};
