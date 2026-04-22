// test_factories.cpp — Testes de EnemyFactory e ItemFactory

#include <doctest/doctest.h>
#include "entities/EnemyFactory.hpp"
#include "entities/Goblin.hpp"
#include "entities/Troll.hpp"
#include "items/ItemFactory.hpp"
#include "items/Item.hpp"

// ─────────────────────────────────────────────
// EnemyFactory
// ─────────────────────────────────────────────

TEST_CASE("EnemyFactory::create Goblin retorna ponteiro nao-nulo com simbolo correto") {
    auto e = EnemyFactory::create(EnemyType::Goblin, 5, 5, 1);
    REQUIRE(e != nullptr);
    CHECK(e->getSymbol() == 'g');
    CHECK(e->isAlive() == true);
}

TEST_CASE("EnemyFactory::create Troll retorna ponteiro nao-nulo com simbolo correto") {
    auto e = EnemyFactory::create(EnemyType::Troll, 5, 5, 1);
    REQUIRE(e != nullptr);
    CHECK(e->getSymbol() == 'T');
    CHECK(e->isAlive() == true);
}

TEST_CASE("EnemyFactory::create Goblin posiciona nas coordenadas passadas") {
    auto e = EnemyFactory::create(EnemyType::Goblin, 3, 7, 1);
    REQUIRE(e != nullptr);
    CHECK(e->getX() == 3);
    CHECK(e->getY() == 7);
}

TEST_CASE("EnemyFactory::create Troll com andar 5 escala atributos") {
    auto e = EnemyFactory::create(EnemyType::Troll, 0, 0, 5);
    REQUIRE(e != nullptr);
    // HP = 12 + (5-1)*5 = 32; ATK = 5 + (5-1)*2 = 13
    CHECK(e->getHp()     == 32);
    CHECK(e->getAttack() == 13);
}

// ─────────────────────────────────────────────
// ItemFactory
// ─────────────────────────────────────────────

TEST_CASE("ItemFactory::create retorna item nao-nulo para andar 1") {
    auto item = ItemFactory::create(5, 5, 1);
    REQUIRE(item != nullptr);
}

TEST_CASE("ItemFactory::create posiciona nas coordenadas passadas") {
    auto item = ItemFactory::create(10, 15, 1);
    REQUIRE(item != nullptr);
    CHECK(item->getX() == 10);
    CHECK(item->getY() == 15);
}

TEST_CASE("ItemFactory::andar 1 nao gera itens de equipment") {
    // Pool do andar 1 so tem: PocaoDeVidaPequena, PocaoDeVida, PocaoDeForça
    // Nenhum item de slot Arma, Armadura ou Acessorio deve aparecer
    // Verificado em 200 amostras — probabilidade de falso positivo negligenciavel
    for (int i = 0; i < 200; ++i) {
        auto item = ItemFactory::create(0, 0, 1);
        REQUIRE(item != nullptr);
        CHECK(item->getSlot() == ItemSlot::Consumivel);
    }
}

TEST_CASE("ItemFactory::andar 3 pode gerar Espada (slot Arma)") {
    // Espada entra no pool a partir do andar 3 com peso 2
    // Em 200 amostras ao menos 1 Arma deve aparecer
    bool encontrouArma = false;
    for (int i = 0; i < 200; ++i) {
        auto item = ItemFactory::create(0, 0, 3);
        if (item && item->getSlot() == ItemSlot::Arma) {
            encontrouArma = true;
            break;
        }
    }
    CHECK(encontrouArma == true);
}

TEST_CASE("ItemFactory::andar 5 pode gerar itens raros (Armadura ou Acessorio)") {
    // EspadaGrande, Armadura, Amuleto entram no pool a partir do andar 5
    bool encontrouRaro = false;
    for (int i = 0; i < 300; ++i) {
        auto item = ItemFactory::create(0, 0, 5);
        if (item && (item->getSlot() == ItemSlot::Armadura ||
                     item->getSlot() == ItemSlot::Acessorio)) {
            encontrouRaro = true;
            break;
        }
    }
    CHECK(encontrouRaro == true);
}
