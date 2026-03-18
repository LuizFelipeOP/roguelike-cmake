#pragma once

// Entity.hpp — Classe base abstrata para todas as entidades do jogo
//
// Uma "entidade" é qualquer coisa que existe no mundo do jogo:
// jogador, inimigo, NPC, item no chão, etc.
//
// Por que classe abstrata?
//   - Define um "contrato": toda entidade TEM que ter posição e TEM que
//     saber se atualizar.
//   - Permite polimorfismo: podemos ter um vector<Entity*> com jogadores
//     e inimigos misturados, e chamar update() em todos sem saber o tipo.
//
// SOLID aplicado:
//   - Open/Closed: adicionar novo tipo de entidade não modifica Entity.
//   - Liskov Substitution: qualquer subclasse pode ser usada onde Entity é esperado.

class Entity {
public:
    // Construtor com posição inicial
    Entity(int x, int y);

    // Destrutor virtual — OBRIGATÓRIO em classes base com herança
    // Sem isso, deletar um Player* via Entity* causaria undefined behavior
    virtual ~Entity() = default;

    // update() — método puramente virtual (= 0)
    // Isso torna Entity uma classe abstrata: não pode ser instanciada diretamente.
    // Cada subclasse DEVE implementar sua própria versão.
    virtual void update() = 0;

    // Getters — retornam a posição atual da entidade
    // "const" no final: garante que não modificam o estado do objeto
    int getX() const;
    int getY() const;

    // Símbolo que representa a entidade no mapa (ex: '@' para jogador, 'G' para goblin)
    char getSymbol() const;

protected:
    // "protected": acessível pela própria classe E pelas subclasses,
    // mas não por código externo. Isso encapsula o estado interno.
    int  x_;
    int  y_;
    char symbol_;  // Subclasses definem seu próprio símbolo
};
