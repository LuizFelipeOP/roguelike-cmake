#include "Enemy.hpp"
#include "map/Map.hpp"
#include "Player.hpp"
#include <algorithm> 

Enemy::Enemy(int x, int y, int hp, int attack, int defense, int xp)
    : Entity(x, y)          // Chama o construtor da classe pai (Entity)
    , hp_(hp)
    , attack_(attack)
    , defense_(defense)
    , xp_(xp)
{}

//logica de receber dano
void Enemy::takeDamage(int amount){
    hp_ = hp_ - std::max(1, amount - defense_);
}
//logica de saber se player esta vivo
bool Enemy::isAlive() const { return hp_ > 0; }


int Enemy::getHp()          const { return hp_; }
int Enemy::getAttack()      const { return attack_; }
int Enemy::getDefense()     const { return defense_; }
int Enemy::getXP()    const { return xp_; }