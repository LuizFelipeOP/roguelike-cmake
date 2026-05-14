#pragma once
#include "IAStrategy.hpp"
#include <functional> 

class PerseguirStrategy : public IAStrategy{
    std::function<void(Player&)> onAtaque_;
    public:
        explicit PerseguirStrategy(std::function<void(Player&)> onAtaque = nullptr);
        void agir(Enemy& self, Map& map, Player& player) override;
};