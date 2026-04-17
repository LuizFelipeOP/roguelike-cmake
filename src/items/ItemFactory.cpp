#include "ItemFactory.hpp" 
#include "CuraStrategy.hpp" 
#include "AumentarATKStrategy.hpp"
#include "BonusDefStrategy.hpp"


std::unique_ptr<Item> ItemFactory::create(ItemType tipo, int x, int y){
    switch (tipo)
    {
    case ItemType::PocaoDeVidaPequena:  
        return std::make_unique<Item>("Pocao de vida pequena", tipo, ItemSlot::Consumivel, x, y, std::make_unique<CuraStrategy>(10));
    case ItemType::PocaoDeVida:  
        return std::make_unique<Item>("Pocao de vida", tipo, ItemSlot::Consumivel, x, y, std::make_unique<CuraStrategy>(20));
    case ItemType::PocaoDeForça:  
        return std::make_unique<Item>("Pocao de forca", tipo, ItemSlot::Consumivel, x, y, std::make_unique<AumentarATKStrategy>(1));
    case ItemType::Espada:  
        return std::make_unique<Item>("Espada", tipo, ItemSlot::Arma, x,y, std::make_unique<AumentarATKStrategy>(2));
    case ItemType::EspadaGrande:  
        return std::make_unique<Item>("Espada Grande", tipo, ItemSlot::Arma, x,y, std::make_unique<AumentarATKStrategy>(4));
    case ItemType::Armadura:  
        return std::make_unique<Item>("Armadura", tipo, ItemSlot::Armadura, x, y, std::make_unique<BonusDefStrategy>());
    case ItemType::Amuleto:  
        return std::make_unique<Item>("Amuleto", tipo, ItemSlot::Acessorio, x, y, std::make_unique<BonusDefStrategy>());
    default: return nullptr;
    }
}

