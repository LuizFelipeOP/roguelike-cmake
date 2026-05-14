#include "Goblin.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include "effects/VenenoEffect.hpp"
#include <algorithm>
#include <random>
#include <ctime>
#include <ia/PerseguirStrategy.hpp>
#include <ia/FugirStrategy.hpp>

Goblin::Goblin(int x, int y, int andar)
    :Enemy(x, y, 6 + (andar - 1) * 3, 3 + (andar - 1), 1, 10)
{
    symbol_ = 'g';

    setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> chance(0, 1);
        if (chance(rng) == 1) {
            p.adicionarEfeito(std::make_unique<VenenoEffect>(1, 3));
        }
    }));
}

int Goblin::getXPReward(){ return getXP(); }

void Goblin::takeDamage(int amount){
    Enemy::takeDamage(amount); 
    if (getHp() < getMaxHP() * 0.3f && isAlive()) {
        setIA(std::make_unique<FugirStrategy>());
    }
}