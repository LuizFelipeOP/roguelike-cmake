#include "EnemyFactory.hpp"
#include <vector>
#include <memory>
#include "Goblin.hpp"
#include "Troll.hpp"

std::unique_ptr<Enemy> EnemyFactory::create(EnemyType type, int x, int y){
    switch (type)
    {
    case EnemyType::Goblin: 
        return std::make_unique<Goblin>(x, y);
    case EnemyType::Troll: 
        return std::make_unique<Troll>(x, y);    
    default:
        return nullptr;
    }
}