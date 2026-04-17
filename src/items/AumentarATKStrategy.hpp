#pragma once

#include "ItemStrategy.hpp"

class Player;

class AumentarATKStrategy : public ItemStrategy{
    int quantidade_;

    public:
        AumentarATKStrategy(int quantidade);
        void usar(Player& player) override;

};