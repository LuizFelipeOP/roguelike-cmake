#pragma once

#include "Entity.hpp"
class Map;
class Player;

class Enemy : public Entity {
public:
    // Construtor: posição inicial + atributos base
    Enemy(int x, int y, int hp, int attack, int defense, int xp);

    virtual ~Enemy() = default;

    // Satisfaz o contrato abstrato de Entity — inimigos não usam a versão sem argumento
    void update() override {}

    // IA do inimigo — chamada pelo Game a cada turno
    virtual void update(Map& map, Player& player) = 0;

    int getHp()      const;
    int getAttack()  const;
    int getDefense() const;

    virtual int getXPReward() = 0;
    int getXP() const;
    
    void takeDamage(int amount);
    bool isAlive() const;


private:
    int hp_;       // Vida atual
    int attack_;   // Poder de ataque
    int defense_;  // Poder de defesa
    int xp_;        // XP de cada inimigo
};
