#pragma once
#include "Item.hpp"
#include <memory> 

class ItemFactory{
    public:
        static std::unique_ptr<Item> create(ItemType tipo, int x, int y);
};