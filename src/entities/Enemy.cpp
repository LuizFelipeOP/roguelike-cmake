#include "Enemy.hpp"
#include "map/Map.hpp"
#include "Player.hpp"

Enemy::Enemy(int x, int y, int hp, int attack, int defense)
    : Entity(x, y)          // Chama o construtor da classe pai (Entity)
    , hp_(hp)
    , attack_(attack)
    , defense_(defense)
{}

//logica de receber dano
void Enemy::takeDamage(int amount){
    hp_ = hp_ - std::max(1, amount - defense_);
}
//logica de saber se player esta vivo
bool Enemy::isAlive(){
    return hp_ > 0;
}


int Enemy::getHp()      const { return hp_; }
int Enemy::getAttack()  const { return attack_; }
int Enemy::getDefense() const { return defense_; }