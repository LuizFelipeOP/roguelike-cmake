// test_entities.cpp — Testes de Player, Goblin, Troll e Enemy

#include <doctest/doctest.h>
#include "entities/Player.hpp"
#include "entities/Goblin.hpp"
#include "entities/Troll.hpp"

// ─────────────────────────────────────────────
// Player
// ─────────────────────────────────────────────

TEST_CASE("Player::atributos iniciais") {
    Player p(5, 5);
    CHECK(p.getHp()    == 50);
    CHECK(p.getMaxHp() == 50);
    CHECK(p.getAttack()  == 5);
    CHECK(p.getDefense() == 2);
    CHECK(p.getLevel() == 1);
    CHECK(p.getXP()    == 0);
    CHECK(p.isAlive()  == true);
    CHECK(p.getX() == 5);
    CHECK(p.getY() == 5);
}

TEST_CASE("Player::takeDamage reduz HP com mitigacao de defesa") {
    Player p(0, 0);
    // Defesa base = 2; dano 10 → HP reduz em 8
    p.takeDamage(10);
    CHECK(p.getHp() == 42);
}

TEST_CASE("Player::takeDamage dano minimo e 1 quando ataque <= defesa") {
    Player p(0, 0);
    // Dano 1 com defesa 2 → mitigação resultaria em 0, mas mínimo é 1
    p.takeDamage(1);
    CHECK(p.getHp() == 49);
}

TEST_CASE("Player::isAlive retorna false quando HP chega a zero") {
    Player p(0, 0);
    p.takeDamage(1000);
    CHECK(p.isAlive() == false);
}

TEST_CASE("Player::curar recupera HP corretamente") {
    Player p(0, 0);
    p.takeDamage(20);   // HP = 50 - (20-2) = 32
    p.curar(10);
    CHECK(p.getHp() == 42);
}

TEST_CASE("Player::curar nao ultrapassa maxHp") {
    Player p(0, 0);
    p.curar(999);
    CHECK(p.getHp() == p.getMaxHp());
    CHECK(p.getHp() == 50);
}

TEST_CASE("Player::setPosition muda coordenadas sem alterar atributos") {
    Player p(1, 1);
    p.setPosition(10, 20);
    CHECK(p.getX() == 10);
    CHECK(p.getY() == 20);
    CHECK(p.getHp() == 50);   // atributos intactos
    CHECK(p.getLevel() == 1);
}

TEST_CASE("Player::raiseAttack incrementa ataque base") {
    Player p(0, 0);
    int atkAntes = p.getAttack();
    p.raiseAttack(3);
    CHECK(p.getAttack() == atkAntes + 3);
}

TEST_CASE("Player::raiseDefense incrementa defesa base") {
    Player p(0, 0);
    int defAntes = p.getDefense();
    p.raiseDefense();
    CHECK(p.getDefense() == defAntes + 1);
}

TEST_CASE("Player::addXP acumula XP") {
    Player p(0, 0);
    p.addXP(5);
    CHECK(p.getXP() == 5);
    CHECK(p.getLevel() == 1);
}

TEST_CASE("Player::addXP sobe de nivel ao atingir threshold") {
    Player p(0, 0);
    // xpProxLevel_ inicial = 20
    p.addXP(20);
    CHECK(p.getLevel() == 2);
    CHECK(p.getXP() == 0);  // XP zerou apos level up
}

TEST_CASE("Player::addXP multiplos level ups em um unico addXP") {
    Player p(0, 0);
    // 20 sobe para nivel 2 (proximo threshold = 30), 50 total sobe para 3
    p.addXP(50);
    CHECK(p.getLevel() >= 2);
}

// ─────────────────────────────────────────────
// Goblin — scaling por andar
// ─────────────────────────────────────────────

TEST_CASE("Goblin::andar 1 tem atributos base corretos") {
    Goblin g(0, 0, 1);
    CHECK(g.getHp()     == 6);    // 6 + (1-1)*3
    CHECK(g.getAttack() == 3);    // 3 + (1-1)
    CHECK(g.getSymbol() == 'g');
    CHECK(g.isAlive()   == true);
}

TEST_CASE("Goblin::andar 3 escala HP e ATK corretamente") {
    Goblin g(0, 0, 3);
    CHECK(g.getHp()     == 12);   // 6 + (3-1)*3
    CHECK(g.getAttack() == 5);    // 3 + (3-1)
}

TEST_CASE("Goblin::getXPReward retorna valor positivo") {
    Goblin g(0, 0, 1);
    CHECK(g.getXPReward() == 10);
}

// ─────────────────────────────────────────────
// Troll — scaling por andar
// ─────────────────────────────────────────────

TEST_CASE("Troll::andar 1 tem atributos base corretos") {
    Troll t(0, 0, 1);
    CHECK(t.getHp()     == 12);   // 12 + (1-1)*5
    CHECK(t.getAttack() == 5);    // 5 + (1-1)*2
    CHECK(t.getSymbol() == 'T');
    CHECK(t.isAlive()   == true);
}

TEST_CASE("Troll::andar 3 escala HP e ATK corretamente") {
    Troll t(0, 0, 3);
    CHECK(t.getHp()     == 22);   // 12 + (3-1)*5
    CHECK(t.getAttack() == 9);    // 5 + (3-1)*2
}

TEST_CASE("Troll::getXPReward retorna valor positivo") {
    Troll t(0, 0, 1);
    CHECK(t.getXPReward() == 25);
}

// ─────────────────────────────────────────────
// Enemy — takeDamage / isAlive (via Goblin)
// ─────────────────────────────────────────────

TEST_CASE("Enemy::takeDamage reduz HP com mitigacao de defesa") {
    Goblin g(0, 0, 1);  // DEF = 1, HP = 6
    g.takeDamage(4);    // 4 - 1 = 3 de dano efetivo
    CHECK(g.getHp() == 3);
}

TEST_CASE("Enemy::takeDamage dano minimo e 1 quando ataque <= defesa") {
    Goblin g(0, 0, 1);
    g.takeDamage(1);    // 1 - 1 = 0 → mínimo 1
    CHECK(g.getHp() == 5);
}

TEST_CASE("Enemy::isAlive false quando HP zerado") {
    Goblin g(0, 0, 1);  // HP = 6
    g.takeDamage(100);
    CHECK(g.isAlive() == false);
}
