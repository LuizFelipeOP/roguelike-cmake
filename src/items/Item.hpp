#pragma once
#include <string>
#include <memory>
#include "ItemStrategy.hpp"

class Player;

enum class ItemType { PocaoDeVida, PocaoDeForça, Espada, Armadura, Amuleto };
enum class ItemSlot { Arma, Armadura, Acessorio, Consumivel };

class Item {
    
    public:
    Item(
        std::string nome, 
        ItemType tipo, 
        ItemSlot slot, 
        int x,
        int y, 
        std::unique_ptr<ItemStrategy> strategy);

    void usar(Player& player);  // delega para strategy_

    std::string getNome() const;
    ItemType getTipo() const;
    ItemSlot getSlot() const;
    int getX() const;
    int getY() const;

    char getSymbol() const;
    
    void setPosicao(int x, int y);
    private:
        std::string nome_;
        ItemType tipo_;
        ItemSlot slot_; 
        std::unique_ptr<ItemStrategy> strategy_;
        int x_;
        int y_;

};
