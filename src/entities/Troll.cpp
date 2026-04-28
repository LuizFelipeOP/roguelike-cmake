#include "Troll.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include "effects/ParalisiaEffect.hpp"
#include <algorithm>
#include <random>
#include <ctime>

Troll::Troll(int x, int y, int andar)
    :Enemy(x, y, 12 + (andar - 1) * 3, 5 + (andar - 1) * 2, 3, 25)
{
    symbol_ = 'T';
}


void Troll::update(Map& map, Player& player) {
    int playerX = player.getX();
    int playerY = player.getY();

    int distX = playerX - x_;
    int distY = playerY - y_;

    int distancia = std::max(std::abs(distX), std::abs(distY));

    if(distancia == 1){
        player.takeDamage(getAttack());

        // 20% de chance de aplicar Paralisia ao atacar
        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> chance(1, 100);
        if (chance(rng) <= 20) {
            player.adicionarEfeito(std::make_unique<ParalisiaEffect>(2));
        }
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
int Troll::getXPReward(){ return getXP(); }