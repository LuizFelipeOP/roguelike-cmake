// test_save.cpp — Testes de persistência: SaveSystem + GameMemento
//
// Cobre:
//  1. existeSave retorna false para arquivo inexistente
//  2. escrever cria o arquivo em disco
//  3. existeSave retorna true após escrever
//  4. Roundtrip completo de GameMemento (salvar → ler → comparar campo a campo)
//  5. Roundtrip com coleções vazias (sem inimigos, itens e efeitos)
//  6. Roundtrip preservando efeitos no player e no inimigo
//  7. Roundtrip da grade explored (fog-of-war)
//  8. Sobrescrever: segundo escrever substitui o save anterior

#include "doctest/doctest.h"
#include "persistence/SaveSystem.hpp"
#include "persistence/GameMemento.hpp"
#include <filesystem>
#include <string>
#include <vector>

// Caminho temporário isolado dos saves reais do jogo
static const std::string SAVE_TESTE = "test_savegame_temp.json";

// Limpa o arquivo antes/depois para não contaminar outros testes
static void removerSave() {
    if (std::filesystem::exists(SAVE_TESTE))
        std::filesystem::remove(SAVE_TESTE);
}

// ----------------------------------------------------------------
// Helpers para montar mementos de teste
// ----------------------------------------------------------------

static PlayerMemento playerPadrao() {
    PlayerMemento p;
    p.x = 5;   p.y = 10;
    p.hp = 80; p.maxHp = 100;
    p.attack = 12; p.defense = 6; p.luck = 3;
    p.xp = 250; p.level = 3; p.xpProxLevel = 400;
    p.attackBonus = 2; p.defenseBonus = 1;
    p.efeitos = {};
    return p;
}

static MapMemento mapaPadrao() {
    MapMemento m;
    m.seed   = 42u;
    m.andar  = 2;
    m.escadaX = 7; m.escadaY = 9;
    // Grade 3×3 com padrão xadrez
    m.explored = {
        {true,  false, true},
        {false, true,  false},
        {true,  false, true}
    };
    return m;
}

static GameMemento mementoCompleto() {
    GameMemento g;

    g.player = playerPadrao();

    // Item no inventário (equipado)
    ItemMemento espada;
    espada.tipo    = "Espada";
    espada.x       = -1; espada.y = -1;
    espada.equipado = true;
    espada.slot    = "Arma";
    g.itensInventario.push_back(espada);

    // Item no chão do mapa (não equipado)
    ItemMemento pocao;
    pocao.tipo    = "Pocao";
    pocao.x       = 3; pocao.y = 4;
    pocao.equipado = false;
    pocao.slot    = "Consumivel";
    g.itensMapa.push_back(pocao);

    // Inimigo simples
    EnemyMemento goblin;
    goblin.tipo     = "Goblin";
    goblin.x = 15; goblin.y = 8;
    goblin.hp       = 20;
    goblin.iaEstado = "Perseguindo";
    goblin.efeitos  = {};
    g.inimigos.push_back(goblin);

    g.mapa = mapaPadrao();
    return g;
}

// ================================================================
// TEST CASES
// ================================================================

TEST_CASE("SaveSystem::existeSave — false antes de criar o arquivo") {
    removerSave();
    CHECK_FALSE(SaveSystem::existeSave(SAVE_TESTE));
}

TEST_CASE("SaveSystem::escrever — cria o arquivo em disco") {
    removerSave();
    GameMemento g = mementoCompleto();

    SaveSystem::escrever(g, SAVE_TESTE);

    CHECK(std::filesystem::exists(SAVE_TESTE));
    removerSave();
}

TEST_CASE("SaveSystem::existeSave — true após escrever") {
    removerSave();
    SaveSystem::escrever(mementoCompleto(), SAVE_TESTE);

    CHECK(SaveSystem::existeSave(SAVE_TESTE));
    removerSave();
}

TEST_CASE("Roundtrip completo — PlayerMemento") {
    removerSave();
    GameMemento original = mementoCompleto();
    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    const PlayerMemento& p = lido.player;
    CHECK(p.x          == original.player.x);
    CHECK(p.y          == original.player.y);
    CHECK(p.hp         == original.player.hp);
    CHECK(p.maxHp      == original.player.maxHp);
    CHECK(p.attack     == original.player.attack);
    CHECK(p.defense    == original.player.defense);
    CHECK(p.luck       == original.player.luck);
    CHECK(p.xp         == original.player.xp);
    CHECK(p.level      == original.player.level);
    CHECK(p.xpProxLevel== original.player.xpProxLevel);
    CHECK(p.attackBonus == original.player.attackBonus);
    CHECK(p.defenseBonus== original.player.defenseBonus);

    removerSave();
}

TEST_CASE("Roundtrip completo — MapMemento") {
    removerSave();
    GameMemento original = mementoCompleto();
    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    const MapMemento& m = lido.mapa;
    CHECK(m.seed    == original.mapa.seed);
    CHECK(m.andar   == original.mapa.andar);
    CHECK(m.escadaX == original.mapa.escadaX);
    CHECK(m.escadaY == original.mapa.escadaY);

    // Grade explored deve ser idêntica
    REQUIRE(m.explored.size() == original.mapa.explored.size());
    for (size_t row = 0; row < m.explored.size(); ++row) {
        REQUIRE(m.explored[row].size() == original.mapa.explored[row].size());
        for (size_t col = 0; col < m.explored[row].size(); ++col) {
            CHECK(m.explored[row][col] == original.mapa.explored[row][col]);
        }
    }

    removerSave();
}

TEST_CASE("Roundtrip completo — itens (inventário e mapa)") {
    removerSave();
    GameMemento original = mementoCompleto();
    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    REQUIRE(lido.itensInventario.size() == 1);
    CHECK(lido.itensInventario[0].tipo    == "Espada");
    CHECK(lido.itensInventario[0].x       == -1);
    CHECK(lido.itensInventario[0].y       == -1);
    CHECK(lido.itensInventario[0].equipado == true);
    CHECK(lido.itensInventario[0].slot    == "Arma");

    REQUIRE(lido.itensMapa.size() == 1);
    CHECK(lido.itensMapa[0].tipo     == "Pocao");
    CHECK(lido.itensMapa[0].x        == 3);
    CHECK(lido.itensMapa[0].y        == 4);
    CHECK(lido.itensMapa[0].equipado == false);
    CHECK(lido.itensMapa[0].slot     == "Consumivel");

    removerSave();
}

TEST_CASE("Roundtrip completo — inimigos") {
    removerSave();
    GameMemento original = mementoCompleto();
    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    REQUIRE(lido.inimigos.size() == 1);
    const EnemyMemento& e = lido.inimigos[0];
    CHECK(e.tipo     == "Goblin");
    CHECK(e.x        == 15);
    CHECK(e.y        == 8);
    CHECK(e.hp       == 20);
    CHECK(e.iaEstado == "Perseguindo");
    CHECK(e.efeitos.empty());

    removerSave();
}

TEST_CASE("Roundtrip — efeitos no player") {
    removerSave();
    GameMemento original = mementoCompleto();

    EfeitoMemento veneno;
    veneno.tipo           = "Veneno";
    veneno.duracaoRestante = 3;
    veneno.potencia       = 5;
    original.player.efeitos.push_back(veneno);

    EfeitoMemento paralisia;
    paralisia.tipo           = "Paralisia";
    paralisia.duracaoRestante = 1;
    paralisia.potencia       = 0;
    original.player.efeitos.push_back(paralisia);

    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    REQUIRE(lido.player.efeitos.size() == 2);

    CHECK(lido.player.efeitos[0].tipo            == "Veneno");
    CHECK(lido.player.efeitos[0].duracaoRestante == 3);
    CHECK(lido.player.efeitos[0].potencia        == 5);

    CHECK(lido.player.efeitos[1].tipo            == "Paralisia");
    CHECK(lido.player.efeitos[1].duracaoRestante == 1);
    CHECK(lido.player.efeitos[1].potencia        == 0);

    removerSave();
}

TEST_CASE("Roundtrip — efeitos no inimigo") {
    removerSave();
    GameMemento original = mementoCompleto();

    EfeitoMemento enfr;
    enfr.tipo           = "Enfraquecimento";
    enfr.duracaoRestante = 2;
    enfr.potencia       = 3;
    original.inimigos[0].efeitos.push_back(enfr);

    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    REQUIRE(lido.inimigos[0].efeitos.size() == 1);
    CHECK(lido.inimigos[0].efeitos[0].tipo            == "Enfraquecimento");
    CHECK(lido.inimigos[0].efeitos[0].duracaoRestante == 2);
    CHECK(lido.inimigos[0].efeitos[0].potencia        == 3);

    removerSave();
}

TEST_CASE("Roundtrip — coleções vazias (sem inimigos, itens e efeitos)") {
    removerSave();
    GameMemento original;
    original.player            = playerPadrao();
    original.mapa              = mapaPadrao();
    original.itensInventario   = {};
    original.itensMapa         = {};
    original.inimigos          = {};

    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    CHECK(lido.itensInventario.empty());
    CHECK(lido.itensMapa.empty());
    CHECK(lido.inimigos.empty());
    CHECK(lido.player.efeitos.empty());

    removerSave();
}

TEST_CASE("Sobrescrever save — segundo escrever substitui o primeiro") {
    removerSave();

    // Primeiro save: player no nível 1
    GameMemento primeiro = mementoCompleto();
    primeiro.player.level = 1;
    primeiro.player.hp    = 50;
    SaveSystem::escrever(primeiro, SAVE_TESTE);

    // Segundo save: player no nível 5
    GameMemento segundo = mementoCompleto();
    segundo.player.level = 5;
    segundo.player.hp    = 99;
    SaveSystem::escrever(segundo, SAVE_TESTE);

    GameMemento lido = SaveSystem::ler(SAVE_TESTE);
    CHECK(lido.player.level == 5);
    CHECK(lido.player.hp    == 99);

    removerSave();
}

TEST_CASE("Roundtrip — iaEstado Fugindo é preservado") {
    removerSave();
    GameMemento original = mementoCompleto();
    original.inimigos[0].iaEstado = "Fugindo";

    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    CHECK(lido.inimigos[0].iaEstado == "Fugindo");
    removerSave();
}

TEST_CASE("Roundtrip — múltiplos inimigos são preservados na ordem") {
    removerSave();
    GameMemento original = mementoCompleto();

    EnemyMemento troll;
    troll.tipo = "Troll"; troll.x = 20; troll.y = 5;
    troll.hp = 60; troll.iaEstado = "Perseguindo"; troll.efeitos = {};
    original.inimigos.push_back(troll);

    EnemyMemento orc;
    orc.tipo = "Orc"; orc.x = 1; orc.y = 2;
    orc.hp = 35; orc.iaEstado = "Fugindo"; orc.efeitos = {};
    original.inimigos.push_back(orc);

    SaveSystem::escrever(original, SAVE_TESTE);
    GameMemento lido = SaveSystem::ler(SAVE_TESTE);

    REQUIRE(lido.inimigos.size() == 3);
    CHECK(lido.inimigos[0].tipo == "Goblin");
    CHECK(lido.inimigos[1].tipo == "Troll");
    CHECK(lido.inimigos[1].hp   == 60);
    CHECK(lido.inimigos[2].tipo == "Orc");
    CHECK(lido.inimigos[2].iaEstado == "Fugindo");

    removerSave();
}
