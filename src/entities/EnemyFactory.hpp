#pragma once

#include "Entity.hpp"
class Map;
class Player;

class EnemyFactory : public Entity {
public:
    // Construtor: posição inicial + atributos base
    EnemyFactory(int x, int y, int hp, int attack, int defense);

    virtual ~EnemyFactory() = default;

    void update() override;

    virtual void update(Map& map, Player& player) = 0;


    int getHp()      const;
    int getAttack()  const;
    int getDefense() const;

    void takeDamage(int amount);
    bool isAlive() const;


private:
    int hp_;       // Vida atual
    int attack_;   // Poder de ataque
    int defense_;  // Poder de defesa

};
