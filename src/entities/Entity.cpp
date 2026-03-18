// Entity.cpp — Implementação da classe base Entity

#include "Entity.hpp"

// Construtor usa "member initializer list" (: x_(x), y_(y), symbol_(symbol))
// Isso é mais eficiente que atribuir dentro do corpo do construtor,
// especialmente para tipos complexos. Para int e char faz pouca diferença,
// mas é uma boa prática adotar desde o início.
Entity::Entity(int x, int y)
    : x_(x), y_(y), symbol_('?') {
    // '?' como padrão — subclasses devem sobrescrever symbol_ no seu construtor
}

int Entity::getX() const {
    return x_;
}

int Entity::getY() const {
    return y_;
}

char Entity::getSymbol() const {
    return symbol_;
}
