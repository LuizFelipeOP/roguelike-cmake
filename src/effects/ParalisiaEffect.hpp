#pragma once

#include "StatusEffect.hpp"
#include <string>

class ParalisiaEffect : public StatusEffect {
public:
    ParalisiaEffect(int duracao);

    void tick(Entity& alvo) override;
    bool isExpired() const override;
    std::string getNome() const override;
    int getPotencia() const override;// Paralisia não tem potência, apenas duração
    int getDuracao() const override;
private:
    int duracaoRestante_;
};
