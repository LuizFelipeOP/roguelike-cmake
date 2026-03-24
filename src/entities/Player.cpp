// Player.cpp — Implementação do jogador

#include "Player.hpp"
#include "map/Map.hpp"
#include "Enemy.hpp"

Player::Player(int x, int y)
    : Entity(x, y)          // Chama o construtor da classe pai (Entity)
    , hp_(2)
    , maxHp_(2)
    , attack_(5)
    , defense_(2)
{
    // Define o símbolo do jogador como '@' — tradição dos roguelikes desde Rogue (1980)
    symbol_ = '@';
}

void Player::update() {
    // Por enquanto vazio — o jogador é atualizado via move()
    // Na Fase 4 adicionaremos efeitos de status (veneno, etc.) aqui
}

void Player::move(int dx, int dy, Map& map, const std::vector<std::unique_ptr<Enemy>>& enemies) {
    int newX = x_ + dx;
    int newY = y_ + dy;

    // Primeiro avalia se existe um enemy no tile a se mover, se houver então ataca
    // Só move se a posição destino for caminhável (não é parede)
    // Princípio importante: o Player pergunta ao Map se pode andar ali.
    // O Player não conhece os detalhes internos do Map — só usa a interface pública.

    for (const auto& enemy : enemies)
    {
        if( enemy->isAlive() && 
            enemy->getX() == newX && 
            enemy->getY() == newY)
        {
            enemy->takeDamage(getAttack());
            return;
        }
    }
    if (map.isWalkable(newX, newY)) {
        x_ = newX;
        y_ = newY;
    }


}
//logica de receber dano
void Player::takeDamage(int amount){
    hp_ = hp_ - std::max(1, amount - defense_);
}
//logica de saber se player esta vivo
bool Player::isAlive() const {
    return hp_ > 0;
}
int Player::getHp()      const { return hp_; }
int Player::getMaxHp()   const { return maxHp_; }
int Player::getAttack()  const { return attack_; }
int Player::getDefense() const { return defense_; }

