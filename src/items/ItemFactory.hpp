#pragma once
#include "Item.hpp"
#include <memory> 

class ItemFactory{
    public:
        static std::unique_ptr<Item> create(int x, int y, int andar);
};