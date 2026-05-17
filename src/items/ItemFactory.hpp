#pragma once
#include "Item.hpp"
#include <memory>
#include <string>

class ItemFactory{
    public:
        static std::unique_ptr<Item> create(int x, int y, int andar);
        // Reconstrói um item específico pelo nome do tipo (usado ao carregar save)
        static std::unique_ptr<Item> createByTipo(const std::string& tipo, int x, int y);
};