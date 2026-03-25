#include "Goblin.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include <algorithm> 

Goblin::Goblin(int x, int y)
    :Enemy(x, y, 6, 3, 1, 10)
{
    symbol_ = 'g';
}


void Goblin::update(Map& map, Player& player) {
    int playerX = player.getX();
    int playerY = player.getY();

    int distX = playerX - x_;
    int distY = playerY - y_;

    int distancia = std::max(std::abs(distX), std::abs(distY));

    if(distancia == 1){
        player.takeDamage(getAttack());
    }else if(distancia <= 8){
        int stepX = (distX > 0) ? 1 : (distX < 0) ? -1 : 0;
        int stepY = (distY > 0) ? 1 : (distY < 0) ? -1 : 0;

        if(map.isWalkable(x_ + stepX, y_ + stepY)){
            x_ = x_ + stepX;
            y_ = y_ + stepY;
        }else if(map.isWalkable(x_, y_ + stepY)){
            y_ = y_ + stepY;    

        }else if(map.isWalkable(x_ + stepX, y_)){
            x_ = x_ + stepX;
        }
    }
}
int Goblin::getXPReward(){ return getXP(); }