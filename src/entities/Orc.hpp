#pragma once
#include "Enemy.hpp"

class Orc : public Enemy {
    public: 
    Orc(int x, int y, int andar);
    int getXPReward() override;
};