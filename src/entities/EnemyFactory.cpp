#include "EnemyFactory.hpp"
#include "Goblin.hpp"
#include "Troll.hpp"
#include <numeric>

// ─────────────────────────────────────────────────────────────────
// Tabela de spawn — define os pesos de cada inimigo por faixa de andar
//
// Para adicionar um novo inimigo (ex: Orc):
//   1. Adicionar EnemyType::Orc em EnemyFactory.hpp
//   2. Adicionar linha abaixo com os pesos desejados
//   3. Adicionar case EnemyType::Orc no switch do create()
//   4. Criar Orc.hpp / Orc.cpp herdando de Enemy
//
// Exemplo de linha para Orc (inimigo médio, aparece a partir do andar 5):
//   { EnemyType::Orc, 0, 0, 15, 25 }
// ─────────────────────────────────────────────────────────────────
static const std::vector<SpawnEntry> tabelaSpawn = {
    //  tipo               andar 1-2  andar 3-4  andar 5-6  andar 7+
    { EnemyType::Goblin,      90,        70,        50,        30 },
    { EnemyType::Troll,       10,        30,        50,        70 },
};

// Retorna o peso correto da entrada para o andar atual
static int pesoPorAndar(const SpawnEntry& entrada, int andar) {
    if (andar <= 2) return entrada.pesoAndar1a2;
    if (andar <= 4) return entrada.pesoAndar3a4;
    if (andar <= 6) return entrada.pesoAndar5a6;
    return entrada.pesoAndar7mais;
}

EnemyType EnemyFactory::sortear(int andar, std::mt19937& rng) {
    // Soma total dos pesos do andar atual
    int total = 0;
    for (const auto& entrada : tabelaSpawn)
        total += pesoPorAndar(entrada, andar);

    // Sorteia um valor entre 1 e o total
    std::uniform_int_distribution<int> dist(1, total);
    int sorteio = dist(rng);

    // Percorre a tabela até encontrar em qual faixa o sorteio caiu
    int acumulado = 0;
    for (const auto& entrada : tabelaSpawn) {
        acumulado += pesoPorAndar(entrada, andar);
        if (sorteio <= acumulado)
            return entrada.tipo;
    }

    // Fallback — nunca deve chegar aqui se os pesos estiverem corretos
    return tabelaSpawn.front().tipo;
}

std::unique_ptr<Enemy> EnemyFactory::create(EnemyType type, int x, int y, int andar) {
    switch (type)
    {
    case EnemyType::Goblin:
        return std::make_unique<Goblin>(x, y, andar);
    case EnemyType::Troll:
        return std::make_unique<Troll>(x, y, andar);
    // case EnemyType::Orc:
    //     return std::make_unique<Orc>(x, y, andar);
    default:
        return nullptr;
    }
}
