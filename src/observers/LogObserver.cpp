#include "LogObserver.hpp"
#include "../entities/Player.hpp"

LogObserver::LogObserver(std::deque<std::string>& log)
    : log_(log) {}

void LogObserver::onEvent(const std::string& mensagem){
    if(log_.size() >= 3) log_.pop_front();
    log_.push_back(mensagem);
}

void LogObserver::onNotify(Player& player){
}