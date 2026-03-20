#pragma once

// Room.hpp — Representa uma sala retangular no dungeon
//
// Usamos struct porque Room é essencialmente um agrupamento de dados
// com métodos auxiliares simples — sem herança, sem estado complexo.
//
// Uma sala é definida por:
//   (x, y)          — canto superior esquerdo
//   width, height   — largura e altura
//
// Exemplo: Room(2, 3, 5, 4) representa:
//
//   col: 0 1 2 3 4 5 6 7
//   row 3: # # . . . . . #   <- y=3, x=2, width=5
//   row 4: # # . . . . . #
//   row 5: # # . . . . . #
//   row 6: # # . . . . . #   <- y + height - 1 = 6

struct Point {
    int x, y;
};

struct Room {
    int x;       // coluna do canto superior esquerdo
    int y;       // linha do canto superior esquerdo
    int width;   // largura da sala
    int height;  // altura da sala

    // Construtor
    Room(int x, int y, int width, int height);

    // center(): retorna o ponto central da sala
    // Dica: centro x = x + width/2, centro y = y + height/2
    Point center() const;

    // contains(px, py): retorna true se o ponto está DENTRO da sala
    // Dica: px >= x && px < x + width && py >= y && py < y + height
    bool contains(int px, int py) const;

    // intersects(other): retorna true se esta sala se sobrepõe com 'other'
    //
    // Dica — é mais fácil verificar quando NÃO se sobrepõem e negar:
    //   this à esquerda:  x + width  <= other.x
    //   this à direita:   x          >= other.x + other.width
    //   this acima:       y + height <= other.y
    //   this abaixo:      y          >= other.y + other.height
    // Se nenhuma for verdade → sobrepõem. Use !(... || ... || ... || ...)
    bool intersects(const Room& other) const;
};
