// test_map.cpp — Testes de Room e Map

#include <doctest/doctest.h>
#include "map/Room.hpp"
#include "map/Map.hpp"

// ─────────────────────────────────────────────
// Room
// ─────────────────────────────────────────────

TEST_CASE("Room::center retorna ponto central correto") {
    Room r(10, 20, 6, 4);
    Point c = r.center();
    CHECK(c.x == 13);   // 10 + 6/2
    CHECK(c.y == 22);   // 20 + 4/2
}

TEST_CASE("Room::contains retorna true para ponto interno") {
    Room r(5, 5, 6, 4);
    CHECK(r.contains(6, 6) == true);
    CHECK(r.contains(5, 5) == true);     // canto superior-esquerdo
}

TEST_CASE("Room::contains retorna false para ponto fora") {
    Room r(5, 5, 6, 4);
    CHECK(r.contains(0, 0)   == false);
    CHECK(r.contains(20, 20) == false);
    CHECK(r.contains(11, 5)  == false);  // x == x+width, fora
}

TEST_CASE("Room::intersects detecta sobreposicao") {
    Room a(0, 0, 5, 5);
    Room b(3, 3, 5, 5);   // se sobrepoem
    Room c(10, 10, 4, 4); // sem sobreposicao
    CHECK(a.intersects(b) == true);
    CHECK(a.intersects(c) == false);
}

TEST_CASE("Room::intersects nao detecta falsa sobreposicao em salas adjacentes") {
    Room a(0, 0, 5, 5);
    Room b(5, 0, 5, 5);   // adjacente, sem sobreposicao
    CHECK(a.intersects(b) == false);
}

// ─────────────────────────────────────────────
// Map
// ─────────────────────────────────────────────

TEST_CASE("Map::generate produz ao menos uma sala") {
    Map m(60, 22);
    m.generate(42);
    CHECK(m.getRooms().size() >= 1);
}

TEST_CASE("Map::isWalkable tile de parede nao e walkable") {
    Map m(20, 10);
    m.generate(1);
    // Borda do mapa é sempre parede '#'
    CHECK(m.isWalkable(0, 0)  == false);
    CHECK(m.isWalkable(19, 9) == false);
}

TEST_CASE("Map::isWalkable tile fora dos limites nao e walkable") {
    Map m(20, 10);
    m.generate(1);
    CHECK(m.isWalkable(-1, 0) == false);
    CHECK(m.isWalkable(0, -1) == false);
    CHECK(m.isWalkable(20, 5) == false);
    CHECK(m.isWalkable(5, 10) == false);
}

TEST_CASE("Map::isWalkable interior de sala e walkable") {
    Map m(60, 22);
    m.generate(42);
    REQUIRE(m.getRooms().size() >= 1);
    // Centro da primeira sala deve ser walkable
    Point centro = m.getRooms().front().center();
    CHECK(m.isWalkable(centro.x, centro.y) == true);
}

TEST_CASE("Map::getTile retorna parede para coordenadas fora dos limites") {
    Map m(20, 10);
    m.generate(1);
    CHECK(m.getTile(-1, 0) == '#');
    CHECK(m.getTile(0, -1) == '#');
    CHECK(m.getTile(20, 5) == '#');
}

TEST_CASE("Map::getPosicaoEscada fica em tile '>' apos generate com 2+ salas") {
    // Gera vários seeds até obter 2+ salas (quase sempre na primeira)
    for (unsigned int seed = 1; seed <= 20; ++seed) {
        Map m(60, 22);
        m.generate(seed);
        if (m.getRooms().size() > 1) {
            Point esc = m.getPosicaoEscada();
            CHECK(m.getTile(esc.x, esc.y) == '>');
            break;
        }
    }
}

TEST_CASE("Map::Fog of War - isExplored false antes de calcularVisibilidade") {
    Map m(60, 22);
    m.generate(42);
    REQUIRE(m.getRooms().size() >= 1);
    Point centro = m.getRooms().front().center();
    // Tile da segunda sala não deve estar explorado antes da visibilidade ser atualizada
    if (m.getRooms().size() > 1) {
        Point outraSala = m.getRooms().back().center();
        CHECK(m.isExplored(outraSala.x, outraSala.y) == false);
    }
}

TEST_CASE("Map::Fog of War - calcularVisibilidade marca tiles no raio como explorados e visiveis") {
    Map m(60, 22);
    m.generate(42);
    REQUIRE(m.getRooms().size() >= 1);
    Point centro = m.getRooms().front().center();
    m.calcularVisibilidade(centro.x, centro.y);
    // Centro deve estar explorado e visível
    CHECK(m.isExplored(centro.x, centro.y) == true);
    CHECK(m.isVisible(centro.x, centro.y) == true);
}

TEST_CASE("Map::Fog of War - isVisible reseta apos nova calcularVisibilidade") {
    Map m(60, 22);
    m.generate(42);
    REQUIRE(m.getRooms().size() >= 1);
    Point centro = m.getRooms().front().center();
    m.calcularVisibilidade(centro.x, centro.y);
    CHECK(m.isVisible(centro.x, centro.y) == true);
    // Move para posição muito distante — centro não deve mais ser visível
    if (m.getRooms().size() > 1) {
        Point outra = m.getRooms().back().center();
        m.calcularVisibilidade(outra.x, outra.y, 1);
        CHECK(m.isExplored(centro.x, centro.y) == true);  // explorado permanece
        CHECK(m.isVisible(centro.x, centro.y) == false);  // mas não visível agora
    }
}

TEST_CASE("Map::isExplored retorna false para coordenadas fora dos limites") {
    Map m(20, 10);
    m.generate(1);
    CHECK(m.isExplored(-1, 0) == false);
    CHECK(m.isExplored(0, -1) == false);
    CHECK(m.isExplored(20, 5) == false);
}
