#include "Troll.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include "effects/ParalisiaEffect.hpp"
#include <algorithm>
#include <random>
#include <ctime>
#include <ia/PerseguirStrategy.hpp>

Troll::Troll(int x, int y, int andar)
    :Enemy(x, y, 12 + (andar - 1) * 5, 5 + (andar - 1) * 2, 3, 25)
{
    symbol_ = 'T';

    setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> chance(1, 100);
        if (chance(rng) <= 20) {
            p.adicionarEfeito(std::make_unique<ParalisiaEffect>(2));
        }
    }));
}

int Troll::getXPReward(){ return getXP(); }