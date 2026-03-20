// Room.cpp — Implementação da struct Room

#include "map/Room.hpp"

Room::Room(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {}

// -------------------------------------------------------------
// Retorna o ponto central da sala como um Point{cx, cy}
// Para retornar: return Point{cx, cy};
// -------------------------------------------------------------
Point Room::center() const {
    return Point{x + width/2, y + height/2};
}

// -------------------------------------------------------------
// Retorna true se (px, py) está dentro da área da sala
// -------------------------------------------------------------
bool Room::contains(int px, int py) const {
    if(px >= x && px < width + x && py >= y && py < height + y){
        return true;
    }
    return false; 
}

// -------------------------------------------------------------
// checa se a sala esta insectando outra ja criada anteriormente
// -------------------------------------------------------------
bool Room::intersects(const Room& other) const {
    if(!(x          >= other.x + other.width || 
         x + width  <= other.x || 
         y          >= other.y + other.height || 
         y + height <= other.y)
    ){
        return true;
    }
    return false;
}
