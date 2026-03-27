// Player.cpp — Implementação do jogador

#include "Player.hpp"
#include "map/Map.hpp"
#include "Enemy.hpp"

Player::Player(int x, int y)
    : Entity(x, y)          // Chama o construtor da classe pai (Entity)
    , hp_(50)
    , maxHp_(50)
    , attack_(5)
    , defense_(2)
    , luck_(2)
    , xp_(0)
    , level_(1)
    , xpProxLevel_(20)
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
int Player::getHp()             const { return hp_; }
int Player::getMaxHp()          const { return maxHp_; }
int Player::getAttack()         const { return attack_; }
int Player::getDefense()        const { return defense_; }
int Player::getXP()             const { return xp_; }
int Player::getLevel()          const { return level_; }
int Player::getXPProxLevel()    const { return xpProxLevel_; }
//adiciona xp ao jogador
std::string Player::addXP(int xpRecebido){
    xp_ = xp_ + xpRecebido;
    std::string notificacao = "";
    while(xp_ >= xpProxLevel_){
        xp_ -= xpProxLevel_;
        xpProxLevel_ = static_cast<int>(xpProxLevel_ * 1.5);
        level_++;
        
        //status melhorados aleatoriamente
        std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);
        std::uniform_int_distribution<int> randType(0, 3);

        std::string statusUpado = "";
        switch (randType(rng)) {
            case 0: 
                // raiseHP();
                maxHp_ = maxHp_ + 2;
                hp_ = hp_ + 2;
                statusUpado = "HP";
                break;
            case 1: 
                // raiseAttack();
                attack_ = attack_ + 1;
                statusUpado = "Ataque"; 
                break;
            case 2: 
                // raiseDefense();
                defense_ = defense_ + 1;
                statusUpado = "Defesa"; 
                break;
            case 3: 
                // raiseLuck();
                luck_ = luck_ + 1;
                statusUpado = "Sorte";  
                break;
        }
        notificacao = "Subiu de nivel! " + statusUpado + " aumentado";
    }
    return notificacao;
}



void Player::curar(int quantidade) {
    if(hp_ + quantidade >=  maxHp_){
        hp_ = maxHp_;
    }else{
        hp_ = hp_ + quantidade;
    }
}
void Player::raiseAttack() {
    attack_ = attack_ + 1;
}
void Player::raiseDefense() {
    defense_ = defense_ + 1;
}
// //melhor HP
// void Player::raiseHP(){
//     maxHp_ = maxHp_ + 2;
//     hp_ = hp_ + 2;
// }

// void Player::raiseAttack(){
//     attack_ = attack_ + 1;
// }

// void Player::raiseDefense(){
//     defense_ = defense_ + 1;
// }

// void Player::raiseLuck(){
//     luck_ = luck_ + 1;
// }
