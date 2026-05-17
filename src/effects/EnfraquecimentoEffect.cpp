#include "EnfraquecimentoEffect.hpp"
#include "../entities/Entity.hpp"

EnfraquecimentoEffect::EnfraquecimentoEffect(int reducaoDano, int duracao)
    : reducaoDano_(reducaoDano), duracaoRestante_(duracao) {}

void EnfraquecimentoEffect::tick(Entity& alvo) {
    alvo.reduzirDanoAtaque(reducaoDano_);
    --duracaoRestante_;
}

bool EnfraquecimentoEffect::isExpired() const {
    return duracaoRestante_ <= 0;
}

std::string EnfraquecimentoEffect::getNome() const {
    return "Enfraquecimento";
}

int EnfraquecimentoEffect::getPotencia() const {
    return reducaoDano_;
}
int EnfraquecimentoEffect::getDuracao() const {
    return duracaoRestante_;
}