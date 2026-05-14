#pragma once
#include <vector>
#include <memory>
#include <random>
#include "Enemy.hpp"

enum class EnemyType { Goblin, Troll, Orc };
// Futuros inimigos: adicionar aqui e na tabelaSpawn em EnemyFactory.cpp

// Define o peso de spawn de um tipo de inimigo por faixa de andar
// Peso maior = maior probabilidade de aparecer naquela faixa
struct SpawnEntry {
    EnemyType tipo;
    int pesoAndar1a2;   // andares 1-2
    int pesoAndar3a4;   // andares 3-4
    int pesoAndar5a6;   // andares 5-6
    int pesoAndar7mais; // andares 7+
};

class EnemyFactory {
public:
    // Cria um inimigo do tipo especificado na posição e andar dados
    static std::unique_ptr<Enemy> create(EnemyType type, int x, int y, int andar);

    // Sorteia qual tipo de inimigo deve spawnar baseado no andar atual
    // Usa pesos definidos na tabelaSpawn — adicionar novos inimigos lá
    static EnemyType sortear(int andar, std::mt19937& rng);
};
