#pragma once
#include <memory>
#include <vector>
#include <functional>
#include "Item.hpp"

class Inventario {
    /*
    - Tentar adicionar item de equip com slot ocupado → retorna `false` (item fica no chão)
    - Tentar adicionar consumível com 5 slots cheios → retorna `false`
    - Desequipar libera o slot; o item retorna para o vetor de consumíveis. Se os consumíveis estiverem cheios, 
    o item é descartado e o `LogObserver` emite: `"Inventario cheio! Item descartado."`
    - equipar(ItemSlot slot, int index) valida o index antes de agir: index fora do range ou 
    slot do item incompatível com o slot destino → operação ignorada silenciosamente, sem quebrar o código
    */
    
    public:
        Inventario();
        bool adicionarItem(std::unique_ptr<Item> item); //returna false se estiver com inventario cheio
        void equipar(ItemSlot slot, int index);         //move consumivel e equip no slot em determinado indice
        void desequipar(ItemSlot slot);                 //desequipa item, libera slot              
        std::unique_ptr<Item> removerConsumivel(int index); //remove item e retorna
        
        Item* getEquipado(ItemSlot slot) const;             //retorna item equipado
        const std::vector<std::unique_ptr<Item>>& getConsumiveis() const; //retorna todos os consumiveis
        bool cheio(ItemSlot slot) const;            //retorna se inventario esta totalmente cheio

        // ItemType tipo, 
        // ItemSlot slot,

        // Callback chamado quando item é descartado por inventário cheio
        // Deve ser conectado ao LogObserver pelo Game
        std::function<void(const std::string&)> onDescarte;

    private:
    std::unique_ptr<Item> arma_;
    std::unique_ptr<Item> armadura_;
    std::unique_ptr<Item> acessorio_;
    std::vector<std::unique_ptr<Item>> consumiveis_; //vetor de maximo 5 itens

    std::unique_ptr<Item>& slotRef(ItemSlot slot);
};