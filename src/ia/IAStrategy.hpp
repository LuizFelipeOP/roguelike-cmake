#pragma once
class Enemy;
class Map;
class Player;

class IAStrategy {
    public:
    virtual void agir(Enemy& self, Map& map, Player& player) = 0;
    virtual ~IAStrategy() = default;
};