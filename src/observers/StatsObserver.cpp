#include "../entities/Player.hpp"
#include "StatsObserver.hpp"
#include "../entities/Player.hpp"
#include "../items/Inventario.hpp"

StatsObserver::StatsObserver(Inventario& inventario)
    : Observer()
    , inventario_(inventario)
{}

void StatsObserver::onNotify(Player& player){
    // int atkBonus = 0, defBonus = 0;
    // if(inventario_.getEquipado(ItemSlot::Arma)) atkBonus += 1;
    // if(inventario_.getEquipado(ItemSlot::Armadura)) defBonus += 1;
    // if(inventario_.getEquipado(ItemSlot::Acessorio)) defBonus += 1;

    // player.setAttackBonus(atkBonus);
    // player.setDefenseBonus(defBonus);
}
