#include "Inventario.hpp"
#include <functional>


Inventario::Inventario(){}

bool Inventario::adicionarItem(std::unique_ptr<Item> item) {
    ItemSlot slot = item->getSlot();

    if(slot == ItemSlot::Consumivel){
        if(consumiveis_.size() >= 5) {
            return false;
        }
        consumiveis_.push_back(std::move(item));
        return true;
    }
    if(cheio(slot)) {
        return false;
    }
    slotRef(slot) = std::move(item);
    return true;
}

void Inventario::equipar(ItemSlot slot, int index) {
    //valida se o index esta dentro do range aceitavel de consumiveis
    if(index < 0 || index >= consumiveis_.size()){
        return;
    }
    // valida compatibilidade de item no indice com slot destino
    if(consumiveis_[index]->getSlot() != slot){
        return;
    }
    // validação se inventario esta cheio
    if(cheio(slot)) {
        return;
    }
    slotRef(slot) = std::move(consumiveis_[index]);
    consumiveis_.erase(consumiveis_.begin() + index);
    
}

void Inventario::desequipar(ItemSlot slot) {
    auto& item = slotRef(slot);
    if(!item) return;

    if(consumiveis_.size() < 5){
        consumiveis_.push_back(std::move(item));
    }else{
        if(onDescarte) onDescarte("Inventario cheio, remova um item para poder desequipar este!");
        item.reset();
    }
}

std::unique_ptr<Item> Inventario::removerConsumivel(int index) {
    if(index < 0 || index >= consumiveis_.size()) return nullptr;
    auto consumivel = std::move(consumiveis_[index]);
    consumiveis_.erase(consumiveis_.begin() + index);
    return consumivel;
}

Item* Inventario::getEquipado(ItemSlot slot) const {
    switch (slot)
    {
        case ItemSlot::Arma: return arma_.get();
        case ItemSlot::Armadura: return armadura_.get();
        case ItemSlot::Acessorio: return acessorio_.get();
        
        default: return nullptr;
    }
}

const std::vector<std::unique_ptr<Item>>& Inventario::getConsumiveis() const {
    // retorna todos os consumiveis
    return consumiveis_;
}

bool Inventario::cheio(ItemSlot slot) const {
    switch (slot)
    {
        case ItemSlot::Arma: return  arma_ != nullptr;
        case ItemSlot::Armadura: return armadura_ != nullptr;
        case ItemSlot::Acessorio: return acessorio_ != nullptr;

        case ItemSlot::Consumivel: return consumiveis_.size() >= 5;
        
        default: return false;
    }
}

std::unique_ptr<Item>& Inventario::slotRef(ItemSlot slot) {
    switch (slot) {
        case ItemSlot::Arma: return arma_;
        case ItemSlot::Armadura: return armadura_;
        case ItemSlot::Acessorio: return acessorio_;
        
        default: return arma_;
    }
}