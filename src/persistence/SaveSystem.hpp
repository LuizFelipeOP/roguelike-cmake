#pragma once
#include "GameMemento.hpp"
#include <string>

class SaveSystem {
public:
    static void escrever(const GameMemento& memento, const std::string& caminho);
    static GameMemento ler(const std::string& caminho);
    static bool existeSave(const std::string& caminho);
};