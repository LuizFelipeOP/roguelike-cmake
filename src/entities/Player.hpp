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
    void move(int dx, int dy, class Map& map);

    // Getters dos atributos do personagem
    int getHp()      const;
    int getMaxHp()   const;
    int getAttack()  const;
    int getDefense() const;

private:
    int hp_;       // Vida atual
    int maxHp_;    // Vida máxima
    int attack_;   // Poder de ataque
    int defense_;  // Poder de defesa
};
