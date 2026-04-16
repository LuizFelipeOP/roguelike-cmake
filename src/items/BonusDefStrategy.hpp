#pragma once

#include "ItemStrategy.hpp"

class Player;

class BonusDefStrategy : public ItemStrategy{
    public:
        void usar(Player& player) override;

};