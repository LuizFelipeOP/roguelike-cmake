#pragma once 
#include <string>
#include <vector>

struct EfeitoMemento {
    std::string tipo;    // "Veneno", "Paralisia", "Regeneracao", "Enfraquecimento"
    int duracaoRestante;
    int potencia;        // dano (Veneno), cura (Regeneracao), reducao (Enfraquecimento)
                         // Para Paralisia: potencia não é usado (usar 0)
};

struct PlayerMemento {
    int x, y;
    int hp, maxHp;
    int attack, defense, luck;
    int xp, level, xpProxLevel;
    int attackBonus, defenseBonus;
    std::vector<EfeitoMemento> efeitos;
};

struct ItemMemento {
    std::string tipo;    // nome do enum ItemType serializado como string
    int x, y;            // -1,-1 se estiver no inventário
    bool equipado;
    std::string slot;    // "Arma", "Armadura", "Acessorio", "Consumivel"
};

struct EnemyMemento {
    std::string tipo;    // "Goblin", "Troll", "Orc"
    int x, y;
    int hp;
    std::string iaEstado; // "Perseguindo", "Fugindo"
    std::vector<EfeitoMemento> efeitos;
};

struct MapMemento {
    unsigned int seed;
    int andar;
    std::vector<std::vector<bool>> explored; // grade completa do fog of war
    int escadaX, escadaY;                    // posição da escada atual
};

struct GameMemento {
    PlayerMemento player;
    std::vector<ItemMemento> itensInventario;  // itens dentro do inventário
    std::vector<ItemMemento> itensMapa;        // itens ainda no chão
    std::vector<EnemyMemento> inimigos;
    MapMemento mapa;
};