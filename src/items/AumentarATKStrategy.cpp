#include "../entities/Player.hpp"
#include "ItemStrategy.hpp"
#include "AumentarATKStrategy.hpp"
#include <algorithm> 

AumentarATKStrategy::AumentarATKStrategy(int quantidade)
    : quantidade_(quantidade)
{}

void AumentarATKStrategy::usar(Player& player){
    player.raiseAttack(quantidade_);
}