#pragma once

#include "StatusEffect.hpp"
#include <string>

class RegeneracaoEffect : public StatusEffect {
public:
    RegeneracaoEffect(int cura, int duracao);

    void tick(Entity& alvo) override;
    bool isExpired() const override;
    std::string getNome() const override;

private:
    int cura_;
    int duracaoRestante_;
};
