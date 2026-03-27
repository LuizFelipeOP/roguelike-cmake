#include "../entities/Player.hpp"
#include "ItemStrategy.hpp"
#include "BonusDefStrategy.hpp"
#include <algorithm> 


void BonusDefStrategy::usar(Player& player){
    player.raiseDefense();
}