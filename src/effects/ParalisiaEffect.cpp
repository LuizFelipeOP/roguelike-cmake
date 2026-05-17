#include "ParalisiaEffect.hpp"
#include "../entities/Entity.hpp"

ParalisiaEffect::ParalisiaEffect(int duracao)
    : duracaoRestante_(duracao) {}

void ParalisiaEffect::tick(Entity& alvo) {
    alvo.setParalisado(true);
    --duracaoRestante_;
}

bool ParalisiaEffect::isExpired() const {
    return duracaoRestante_ <= 0;
}

std::string ParalisiaEffect::getNome() const {
    return "Paralisia";
}
int ParalisiaEffect::getPotencia() const {
    return 0; // Paralisia não tem potência, apenas duração
}
int ParalisiaEffect::getDuracao() const {
    return duracaoRestante_;
}