#pragma once
#include "ItemStrategy.hpp"
#include "../effects/StatusEffect.hpp"
#include <functional>
#include <memory>

class AplicarEfeitoStrategy : public ItemStrategy {
    std::function<std::unique_ptr<StatusEffect>()> fabrica_;
public:
    explicit AplicarEfeitoStrategy(std::function<std::unique_ptr<StatusEffect>()> fabrica);
    void usar(Player& player) override;
};
