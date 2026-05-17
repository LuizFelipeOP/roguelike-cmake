#include "VenenoEffect.hpp"
#include "../entities/Entity.hpp"

VenenoEffect::VenenoEffect(int dano, int duracao)
    : dano_(dano), duracaoRestante_(duracao) {}

void VenenoEffect::tick(Entity& alvo) {
    alvo.takeDamage(dano_);
    --duracaoRestante_;
}

bool VenenoEffect::isExpired() const {
    return duracaoRestante_ <= 0;
}

std::string VenenoEffect::getNome() const {
    return "Veneno";
}

int VenenoEffect::getPotencia() const {
    return dano_;
}
int VenenoEffect::getDuracao() const {
    return duracaoRestante_;
}