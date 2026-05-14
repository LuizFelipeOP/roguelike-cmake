#pragma once

#include "Enemy.hpp"
#include "Player.hpp"

class Troll : public Enemy {
public:
    Troll(int x, int y, int andar);
    int getXPReward() override;
};
