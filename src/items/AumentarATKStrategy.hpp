#pragma once

#include "ItemStrategy.hpp"

class Player;

class AumentarATKStrategy : public ItemStrategy{
    public:
        void usar(Player& player) override;

};