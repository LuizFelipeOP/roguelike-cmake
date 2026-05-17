#pragma once

#include "StatusEffect.hpp"
#include <string>

class EnfraquecimentoEffect : public StatusEffect {
public:
    EnfraquecimentoEffect(int reducaoDano, int duracao);

    void tick(Entity& alvo) override;
    bool isExpired() const override;
    std::string getNome() const override;
    int getPotencia() const override;
    int getDuracao() const override;
private:
    int reducaoDano_;
    int duracaoRestante_;
};
