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
