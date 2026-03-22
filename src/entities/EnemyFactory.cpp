#include "EnemyFactory.hpp"
#include "map/Map.hpp"
#include "Player.hpp"
#include <algorithm> 

EnemyFactory::EnemyFactory(int x, int y, int hp, int attack, int defense)
    : Entity(x, y)          // Chama o construtor da classe pai (Entity)
    , hp_(hp)
    , attack_(attack)
    , defense_(defense)
{}

//logica de receber dano
void EnemyFactory::takeDamage(int amount){
    hp_ = hp_ - std::max(1, amount - defense_);
}
//logica de saber se inimigo esta vivo
bool EnemyFactory::isAlive() const { return hp_ > 0; }


int EnemyFactory::getHp()      const { return hp_; }
int EnemyFactory::getAttack()  const { return attack_; }
int EnemyFactory::getDefense() const { return defense_; }