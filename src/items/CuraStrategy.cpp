#include "../entities/Player.hpp"
#include "ItemStrategy.hpp"
#include "CuraStrategy.hpp"
#include <algorithm> 

CuraStrategy::CuraStrategy(int quantidade)
    : quantidade_(quantidade)
{
}

void CuraStrategy::usar(Player& player){
    player.curar(quantidade_);
}