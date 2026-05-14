#include "Orc.hpp"
#include "Player.hpp"
#include "map/Map.hpp"
#include "ia/PerseguirStrategy.hpp"
#include "effects/EnfraquecimentoEffect.hpp"
#include <algorithm>
#include <random>
#include <ctime>

Orc::Orc(int x, int y, int andar)
    :Enemy(x, y, 8 + (andar - 1) * 4, 4 + (andar - 1) * 2, 2, 18)
{
    symbol_ = 'O';

    setIA(std::make_unique<PerseguirStrategy>([](Player& p) {
        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> chance(0, 100);
        if (chance(rng) <= 30) {
            p.adicionarEfeito(std::make_unique<EnfraquecimentoEffect>(2, 3));
        }
    }));
}

int Orc::getXPReward(){ return getXP(); }