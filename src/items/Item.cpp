#include "Item.hpp"
#include <algorithm> 
#include <string>
#include <memory>
#include "../entities/Player.hpp"

Item::Item(
        std::string nome, 
        ItemType tipo, 
        ItemSlot slot,
        int x,
        int y, 
        std::unique_ptr<ItemStrategy> strategy)
    : nome_(std::move(nome))
    , tipo_(tipo)
    , slot_(slot)
    , x_(x)
    , y_(y)
    , strategy_(std::move(strategy))

{}

ItemSlot    Item::getSlot()   const { return slot_; }
ItemType    Item::getTipo()   const { return tipo_; }
std::string Item::getNome()   const { return nome_; }
int Item::getX()      const { return x_; }
int Item::getY()      const { return y_; }

char Item::getSymbol() const {
    switch (slot_)
    {
        case ItemSlot::Arma: return '/';
        case ItemSlot::Armadura: return ']';
        case ItemSlot::Acessorio: return '"';
        case ItemSlot::Consumivel: return '6';
        default: return '?';
    }
}

void Item::usar(Player& player) {
    if(strategy_){
        strategy_->usar(player);
    }
}

void Item::setPosicao(int x, int y) {
    x_ = x;
    y_ = y;
}