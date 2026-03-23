#pragma once
#include <vector>
#include <memory>
#include "Enemy.hpp"

enum class EnemyType { Goblin, Troll };

class EnemyFactory {
public:
    static std::unique_ptr<Enemy> create(EnemyType type, int x, int y);
};
