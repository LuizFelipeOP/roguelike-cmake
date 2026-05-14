#pragma once
#include "IAStrategy.hpp"


class FugirStrategy : public IAStrategy {
    public:
        void agir(Enemy& self, Map& map, Player& player) override;
};