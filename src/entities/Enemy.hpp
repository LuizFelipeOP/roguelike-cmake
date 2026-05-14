#pragma once

#include "Entity.hpp"
#include "ia/IAStrategy.hpp"
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
    virtual void update(Map& map, Player& player);

    int getHp()      const;
    int getAttack()  const;
    int getDefense() const;

    virtual int getXPReward() = 0;
    int getXP() const;
    
    void takeDamage(int amount) override;
    bool isAlive() const;
    void curar(int quantidade) override;
    void reduzirDanoAtaque(int quantidade) override;
    void setIA(std::unique_ptr<IAStrategy> ia);
    void moverPara(int x, int y);
    int getMaxHP() const;


private:
    int maxHp_;    // Vida total
    int hp_;       // Vida atual
    int attack_;   // Poder de ataque
    int defense_;  // Poder de defesa
    int xp_;        // XP de cada inimigo

    std::unique_ptr<IAStrategy> ia_;

};
