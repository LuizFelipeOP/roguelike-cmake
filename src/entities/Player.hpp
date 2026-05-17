#pragma once

// Player.hpp — Declara o jogador, que é um tipo específico de Entity
//
// Herança: Player "é uma" Entity — faz sentido usar herança aqui.
// Regra prática: use herança para relação "é um", composição para "tem um".
//
// Player adiciona:
//   - Atributos de personagem (HP, ataque, defesa)
//   - Método para receber input de movimento
//   - Implementação de update() (obrigatória por ser abstrata em Entity)

#include "Entity.hpp"
class Enemy;   // forward declaration
#include <vector>
#include <memory>
#include <string>
#include "observers/Observer.hpp"
#include "items/Inventario.hpp"

class Player : public Entity {
public:
    // Construtor: posição inicial + atributos base
    Player(int x, int y);

    // Implementação obrigatória do método abstrato de Entity
    // "override" — keyword do C++11 que avisa o compilador:
    // "este método deve existir na classe pai". Se não existir, erro de compilação.
    // Isso evita bugs silenciosos de digitação errada no nome do método.
    void update() override;

    // move(): tenta mover o jogador na direção (dx, dy)
    // Recebe o mapa para verificar se a posição é caminhável
    // Declaramos o Map com forward declaration abaixo para evitar include circular
    void move(int dx, int dy, class Map& map, const std::vector<std::unique_ptr<Enemy>>& enemies);

    //posiciona player num lugar especifico
    void setPosition(int x, int y);
    
    // Getters dos atributos do personagem
    int getHp()         const;
    int getMaxHp()      const;
    int getAttack()     const;
    int getDefense()    const;
    int getLuck()       const;
    int getXP()         const;
    int getLevel()      const;
    int getXPProxLevel() const;
    int getBaseAttack()  const;   // attack_ sem bonus de equipamento
    int getBaseDefense() const;   // defense_ sem bonus de equipamento
    int getAttackBonus() const;
    int getDefenseBonus() const;

    void takeDamage(int amount) override;
    bool isAlive() const;

    void curar(int quantidade) override;
    void reduzirDanoAtaque(int quantidade) override;

    void setAttackBonus(int bonus);
    void setDefenseBonus(int bonus);

    // Setters para restaurar estado ao carregar save
    void setHp(int valor);
    void setMaxHp(int valor);
    void setAttack(int valor);
    void setDefense(int valor);
    void setLuck(int valor);
    void setXP(int valor);
    void setLevel(int valor);
    void setXPProxLevel(int valor);
    void adicionarObserver(Observer* obs);
    void notificarObservers();
    Inventario&       getInventario();
    const Inventario& getInventario() const;
    void raiseAttack(int quantidade);
    void raiseDefense();
    void raiseLuck();
    
    std::string addXP(int xpRecebido);

    
private:
    int hp_;       // Vida atual
    int maxHp_;    // Vida máxima
    int attack_;   // Poder de ataque
    int defense_;  // Poder de defesa
    int luck_;     // Sorte de items ou quantidade de itens

    int xp_;
    int level_;
    int xpProxLevel_;

    //logica de status e inventario (strategy e observer)
    int attackBonus_;
    int defenseBonus_;
    std::vector<Observer*> observers_;
    Inventario inventario_;

};
