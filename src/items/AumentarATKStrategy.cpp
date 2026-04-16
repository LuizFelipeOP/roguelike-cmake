#include "../entities/Player.hpp"
#include "ItemStrategy.hpp"
#include "AumentarATKStrategy.hpp"
#include <algorithm> 


void AumentarATKStrategy::usar(Player& player){
    player.raiseAttack();
}