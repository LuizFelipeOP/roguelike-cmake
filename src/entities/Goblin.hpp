#pragma once

#include "Enemy.hpp"
#include "Player.hpp"

class Goblin : public Enemy {
public:
    Goblin(int x, int y, int andar);
    int getXPReward() override;

    void takeDamage(int amount) override;
};
