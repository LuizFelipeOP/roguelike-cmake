#include "Enemy.hpp"
#include "map/Map.hpp"
#include "Player.hpp"
#include <algorithm> 
#include "ia/FugirStrategy.hpp"

Enemy::Enemy(int x, int y, int hp, int attack, int defense, int xp)
    : Entity(x, y)
    , maxHp_(hp)          // Chama o construtor da classe pai (Entity)
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

void Enemy::curar(int quantidade) {
    hp_ += quantidade; // inimigos não têm maxHp definido — cura sem teto
}

void Enemy::reduzirDanoAtaque(int quantidade) {
    attack_ = std::max(1, attack_ - quantidade);
}

int Enemy::getHp()          const { return hp_; }
int Enemy::getAttack()      const { return attack_; }
int Enemy::getDefense()     const { return defense_; }
int Enemy::getXP()    const { return xp_; }

void Enemy::moverPara(int x, int y){
    x_ = x;
    y_ = y;
}

void Enemy::setIA(std::unique_ptr<IAStrategy> ia) {
    ia_ = std::move(ia);
}

void Enemy::update(Map& map, Player& player) {
    if (isParalisado()) return;
    if (ia_) ia_->agir(*this, map, player);
}

int Enemy::getMaxHP() const {
    return maxHp_;
}

IAEstado Enemy::getIAEstado() const {
    if (dynamic_cast<FugirStrategy*>(ia_.get())) return IAEstado::Fugindo;
    return IAEstado::Perseguindo;
}