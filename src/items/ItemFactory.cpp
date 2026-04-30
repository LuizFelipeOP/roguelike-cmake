#include "ItemFactory.hpp" 
#include "CuraStrategy.hpp" 
#include "AumentarATKStrategy.hpp"
#include "BonusDefStrategy.hpp"
#include "AplicarEfeitoStrategy.hpp"
#include "../effects/RegeneracaoEffect.hpp"
#include <random>


std::unique_ptr<Item> ItemFactory::create(int x, int y, int andar){

    static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);

    // Tabela de pesos por andar: {tipo, peso}
    // Peso maior = mais comum. Itens de equipamento só entram no pool a partir de certo andar.
    std::vector<std::pair<ItemType, int>> pool = {
        { ItemType::PocaoDeVidaPequena, 5 },
        { ItemType::PocaoDeVida, 3 },
        { ItemType::PocaoDeForça, 3 },
        { ItemType::PocaoDeRegeneracao, 2 },
    };
    if (andar > 2) {
        pool.push_back({ ItemType::Espada, 2 });
    }
    if (andar > 4) {
        pool.push_back({ ItemType::EspadaGrande, 1 });
        pool.push_back({ ItemType::Armadura, 1 });
        pool.push_back({ ItemType::Amuleto, 1 });
    }

    // Extrai os pesos e sorteia usando distribuição ponderada
    std::vector<int> pesos;
    for (auto& par : pool) pesos.push_back(par.second);

    std::discrete_distribution<> dis(pesos.begin(), pesos.end());
    ItemType tipo_aleatorio = pool[dis(rng)].first;

    switch (tipo_aleatorio)
    {
        case ItemType::PocaoDeVidaPequena:  
            return std::make_unique<Item>("Pocao de vida pequena", tipo_aleatorio, ItemSlot::Consumivel, x, y, std::make_unique<CuraStrategy>(10));
        case ItemType::PocaoDeVida:  
            return std::make_unique<Item>("Pocao de vida", tipo_aleatorio, ItemSlot::Consumivel, x, y, std::make_unique<CuraStrategy>(20));
        case ItemType::PocaoDeForça:  
            return std::make_unique<Item>("Pocao de forca", tipo_aleatorio, ItemSlot::Consumivel, x, y, std::make_unique<AumentarATKStrategy>(1));
        case ItemType::PocaoDeRegeneracao:
            return std::make_unique<Item>("Pocao de Regeneracao", tipo_aleatorio, ItemSlot::Consumivel, x, y,
                std::make_unique<AplicarEfeitoStrategy>([]{ return std::make_unique<RegeneracaoEffect>(5, 3); }));
        case ItemType::Espada:  
            return std::make_unique<Item>("Espada", tipo_aleatorio, ItemSlot::Arma, x,y, std::make_unique<AumentarATKStrategy>(2));
        case ItemType::EspadaGrande:  
            return std::make_unique<Item>("Espada Grande", tipo_aleatorio, ItemSlot::Arma, x,y, std::make_unique<AumentarATKStrategy>(4));
        case ItemType::Armadura:  
            return std::make_unique<Item>("Armadura", tipo_aleatorio, ItemSlot::Armadura, x, y, std::make_unique<BonusDefStrategy>());
        case ItemType::Amuleto:  
            return std::make_unique<Item>("Amuleto", tipo_aleatorio, ItemSlot::Acessorio, x, y, std::make_unique<BonusDefStrategy>());
        default: return nullptr;
    }
}

