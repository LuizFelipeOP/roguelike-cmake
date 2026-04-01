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

}

std::unique_ptr<Item> Inventario::removerConsumivel(int index) {
    // remove item e retorna
    return nullptr;
}

Item* Inventario::getEquipado(ItemSlot slot) const {
    // retorna item equipado
    return nullptr;
}

const std::vector<std::unique_ptr<Item>>& Inventario::getConsumiveis() const {
    // retorna todos os consumiveis
    return {};
}

bool Inventario::cheio(ItemSlot slot) const {
    // retorna se inventario esta totalmente cheio
    return false;
}