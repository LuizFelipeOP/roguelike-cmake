#include "EnemyFactory.hpp"
#include <vector>
#include <memory>
#include "Goblin.hpp"
#include "Troll.hpp"

std::unique_ptr<Enemy> EnemyFactory::create(EnemyType type, int x, int y, int andar){
    switch (type)
    {
    case EnemyType::Goblin: 
        return std::make_unique<Goblin>(x, y, andar);
    case EnemyType::Troll: 
        return std::make_unique<Troll>(x, y, andar);    
    default:
        return nullptr;
    }
}