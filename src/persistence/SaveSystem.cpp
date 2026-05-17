
#include "SaveSystem.hpp"
#include "../items/Item.hpp"
#include "../entities/EnemyFactory.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────────────────────
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
//
// Este macro gera automaticamente duas funções para cada struct:
//   - to_json(json& j, const T& obj)   → usado ao escrever:  j = memento
//   - from_json(const json& j, T& obj) → usado ao ler:       j.get<T>()
//
// Você lista os campos uma vez e a lib cuida do resto.
// Funciona com tipos primitivos (int, bool, string) e vetores aninhados.
// ─────────────────────────────────────────────────────────────────────────────

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EfeitoMemento,
    tipo, duracaoRestante, potencia)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerMemento,
    x, y, hp, maxHp, attack, defense, luck,
    xp, level, xpProxLevel, attackBonus, defenseBonus, efeitos)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ItemMemento,
    tipo, x, y, equipado, slot)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnemyMemento,
    tipo, x, y, hp, iaEstado, efeitos)

// vector<vector<bool>> é suportado nativamente pela nlohmann — vira array de arrays no JSON
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MapMemento,
    seed, andar, explored, escadaX, escadaY)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameMemento,
    player, itensInventario, itensMapa, inimigos, mapa)

// ─────────────────────────────────────────────────────────────────────────────
// escrever
//
// j = memento chama to_json() gerado pelo macro para GameMemento,
// que por sua vez chama to_json() de cada campo recursivamente.
// ─────────────────────────────────────────────────────────────────────────────

void SaveSystem::escrever(const GameMemento& memento, const std::string& caminho) {
    json j = memento;
    std::ofstream arquivo(caminho);
    arquivo << j.dump(4); // dump(4) = JSON formatado com 4 espaços de indentação
}

// ─────────────────────────────────────────────────────────────────────────────
// ler
//
// j.get<GameMemento>() chama from_json() gerado pelo macro,
// preenchendo todos os campos recursivamente a partir do JSON lido.
// ─────────────────────────────────────────────────────────────────────────────

GameMemento SaveSystem::ler(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    json j = json::parse(arquivo);
    return j.get<GameMemento>();
}

// ─────────────────────────────────────────────────────────────────────────────
// existeSave
// ─────────────────────────────────────────────────────────────────────────────

bool SaveSystem::existeSave(const std::string& caminho) {
    return std::filesystem::exists(caminho);
}
