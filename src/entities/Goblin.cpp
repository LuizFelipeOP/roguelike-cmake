#include "Goblin.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include "effects/VenenoEffect.hpp"
#include <algorithm>
#include <random>
#include <ctime>

Goblin::Goblin(int x, int y, int andar)
    :Enemy(x, y, 6 + (andar - 1) * 3, 3 + (andar - 1), 1, 10)
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

        // 50% de chance de aplicar Veneno ao atacar
        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> chance(0, 1);
        if (chance(rng) == 1) {
            player.adicionarEfeito(std::make_unique<VenenoEffect>(1, 3));
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
int Goblin::getXPReward(){ return getXP(); }