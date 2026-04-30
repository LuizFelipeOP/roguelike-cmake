#include "AplicarEfeitoStrategy.hpp"
#include "../entities/Player.hpp"

AplicarEfeitoStrategy::AplicarEfeitoStrategy(std::function<std::unique_ptr<StatusEffect>()> fabrica)
    : fabrica_(std::move(fabrica)) {}

void AplicarEfeitoStrategy::usar(Player& player) {
    auto efeito = fabrica_();
    player.adicionarEfeito(std::move(efeito));
}
