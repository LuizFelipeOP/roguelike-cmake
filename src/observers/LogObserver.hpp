#pragma once
#include "Observer.hpp"
#include <deque>
#include <string>

class Inventario;

// LogObserver registra eventos de inventário no log do jogo
class LogObserver : public Observer {
    std::deque<std::string>& log_;

    public:
        LogObserver(std::deque<std::string>& log);
        
        void onNotify(Player& player) override;
        void onEvent(const std::string& mensagem);
};
