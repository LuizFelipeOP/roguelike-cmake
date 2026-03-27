#pragma once

#include "ItemStrategy.hpp"

class Player;

class CuraStrategy : public ItemStrategy{
    int quantidade_;

    public:
        CuraStrategy(int quantidade);

        void usar(Player& player) override;

};