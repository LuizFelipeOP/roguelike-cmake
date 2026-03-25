#pragma once

#include "Enemy.hpp"
#include "Player.hpp"

class Goblin : public Enemy {
public:
    // Construtor: posição inicial + atributos base
    Goblin(int x, int y);

    void update(Map& map, Player& player) override;
    int getXPReward() override;

};
