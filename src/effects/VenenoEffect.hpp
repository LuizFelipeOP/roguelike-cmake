#pragma once

#include "StatusEffect.hpp"
#include <string>

class VenenoEffect : public StatusEffect {
    public: 
        VenenoEffect(int dano, int duracao);

        void tick(Entity& alvo) override;
        bool isExpired() const override;
        std::string getNome() const override;

    private:
        int duracaoRestante_;
        int dano_;
};