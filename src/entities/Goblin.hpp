#pragma once

#include "Enemy.hpp"
#include "Player.hpp"

class Goblin : public Enemy {
public:
    Goblin(int x, int y, int andar);
    int getXPReward() override;
    std::string getTipoNome() const override { return "Goblin"; }

    void takeDamage(int amount) override;
};
