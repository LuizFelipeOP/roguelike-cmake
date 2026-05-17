// Entity.cpp — Implementação da classe base Entity

#include "Entity.hpp"
#include <algorithm>

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

void Entity::adicionarEfeito(std::unique_ptr<StatusEffect> efeito) {
    if (onEfeitoEvento)
        onEfeitoEvento("Voce foi afetado por " + efeito->getNome());
    efeitos_.push_back(std::move(efeito));
}

std::vector<std::string> Entity::getEfeitosNomes() const {
    std::vector<std::string> nomes;
    for (const auto& e : efeitos_)
        nomes.push_back(e->getNome());
    return nomes;
}

void Entity::tickEfeitos() {
    for (auto& efeito : efeitos_)
        efeito->tick(*this);

    // Notifica expiração antes de remover
    if (onEfeitoEvento) {
        for (const auto& e : efeitos_)
            if (e->isExpired())
                onEfeitoEvento(e->getNome() + " expirou.");
    }

    // Remove efeitos expirados
    efeitos_.erase(
        std::remove_if(efeitos_.begin(), efeitos_.end(),
            [](const std::unique_ptr<StatusEffect>& e) { return e->isExpired(); }),
        efeitos_.end()
    );

    // Se nenhum ParalisiaEffect ativo restou, desparalisa
    bool temParalisia = std::any_of(efeitos_.begin(), efeitos_.end(),
        [](const std::unique_ptr<StatusEffect>& e) {
            return e->getNome() == "Paralisia";
        });
    if (!temParalisia)
        paralisado_ = false;
}

bool Entity::isParalisado() const {
    return paralisado_;
}

void Entity::setParalisado(bool valor) {
    paralisado_ = valor;
}

const std::vector<std::unique_ptr<StatusEffect>>& Entity::getEfeitos() const {
    return efeitos_;
}

void Entity::limparEfeitos() {
    efeitos_.clear();
    paralisado_ = false;
}

