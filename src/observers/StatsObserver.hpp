#pragma once

#include "Observer.hpp"
class Inventario;

class StatsObserver : public Observer{
    Inventario& inventario_;

public:
    // Construtor: 
    StatsObserver(Inventario& inventario);

    void onNotify(Player& player) override;
};