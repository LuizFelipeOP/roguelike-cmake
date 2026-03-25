#pragma once

#include "Enemy.hpp"
#include "Player.hpp"

class Troll : public Enemy {
public:
    // Construtor: posição inicial + atributos base
    Troll(int x, int y);

    void update(Map& map, Player& player) override;
    int getXPReward() override;

};
