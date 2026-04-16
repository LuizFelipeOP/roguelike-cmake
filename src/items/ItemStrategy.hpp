#pragma once

class Player;

class ItemStrategy {
    public: 
        virtual void usar( Player& player) = 0;

        virtual ~ItemStrategy() = default;
};