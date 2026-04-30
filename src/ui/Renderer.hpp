#pragma once

// Renderer.hpp — Responsável por TODA saída visual do jogo
//
// Por que separar o Renderer?
//   Dependency Inversion Principle: o Game não deve saber que estamos
//   usando o console. Se amanhã quisermos usar SDL ou ncurses,
//   só trocamos o Renderer — o Game não muda.
//
//   Hoje: ConsoleRenderer (chars no terminal)
//   Futuro possível: SDLRenderer, CursesRenderer, etc.
//
// Por enquanto é uma classe concreta simples.
// Na Fase 3 vamos transformá-la em interface (IRenderer)
// e criar implementações concretas — isso é o padrão Strategy.

#include "map/Map.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "items/Item.hpp"
#include "items/Inventario.hpp"
#include <deque>

class Renderer {
public:
    Renderer() = default;

    // render(): desenha o estado completo do jogo no console
    // Recebe o mapa e o jogador para saber o que desenhar
    void render(
        const Map& map, 
        const Player& player, 
        const std::vector<std::unique_ptr<Enemy>>& enemies, 
        const std::vector<std::unique_ptr<Item>>& items,
        const std::deque<std::string>& messageLog,
        bool inventarioAberto,
        int andarAtual
    );

    // renderHUD(): desenha a barra de status (HP, nível, etc.)
    // HUD = Heads-Up Display — informações fixas na tela
    void renderHUD(const Player& , const std::deque<std::string>&  messageLog, bool inventarioAberto, int andarAtual);

    // renderiza UI de inventario
    void renderInventario(const Inventario& inv);

private:
    // clearScreen(): limpa o terminal antes de cada frame
    // Técnica simples para animação no console
    void clearScreen();

    // Aplica cor no console baseada nos efeitos ativos do player
    void setCorEfeito(const std::vector<std::string>& nomes);

    // Reseta a cor do console para o padrão (branco)
    void resetarCor();

    // Aplica cor no console baseada no nome de um efeito específico
    void setCorPorNome(const std::string& nome);
};
