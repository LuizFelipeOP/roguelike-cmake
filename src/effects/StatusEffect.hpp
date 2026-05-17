#pragma once

class Entity;
#include <string>

class StatusEffect {
    public:
        virtual void tick(Entity& alvo) = 0;
        virtual bool isExpired() const = 0;
        virtual std::string getNome() const = 0;
        virtual ~StatusEffect() = default;  
        virtual int getDuracao() const = 0;
        virtual int getPotencia() const = 0;
};