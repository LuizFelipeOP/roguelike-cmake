// Map.cpp — Implementação do mapa com geração procedural

#include "map/Map.hpp"
#include <algorithm>  // std::min, std::max

// ============================================================
//  Métodos da Fase 1 — seus, não mude
// ============================================================

// constroi o mapa, onde temos y e x, onde os indices 0 e Max
// são paredes (#) e o resto é chão '.'
Map::Map(int width, int height)
    : width_(width), height_(height) {
    tiles_.assign(height, std::vector<char>(width, '.'));
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1) {
                tiles_[y][x] = '#';
            }
        }
    }
}

// Checa se o chão é '.', faz checagem de bounds antes
bool Map::isWalkable(int x, int y) const {
    if (y >= 0 && y < height_ && x >= 0 && x < width_) {
        return tiles_[y][x] != '#';
    }
    return false;
}

// retorna o piso solicitado, dentro dos bounds corretos
char Map::getTile(int x, int y) const {
    if (y >= 0 && y < height_ && x >= 0 && x < width_) {
        return tiles_[y][x];
    }
    return '#';
}

int Map::getWidth()  const { return width_; }
int Map::getHeight() const { return height_; }

const std::vector<Room>& Map::getRooms() const { return rooms_; }

// ============================================================
//  Método auxiliar — já implementado
// ============================================================

// fill(): reseta o mapa inteiro para um único tile
void Map::fill(char tile) {
    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
            tiles_[y][x] = tile;
}


// ------------------------------------------------------------
// "Escava" a sala: preenche a área interna com '.'
// -------------------------------------------------------------
void Map::carveRoom(const Room& room) {
     for (int y = room.y; y < room.y + room.height; ++y) {
        for (int x = room.x; x < room.x + room.width; ++x) {
            // if (x > room.x || x == width_ - 1 || y == 0 || y == height_ - 1) {
                tiles_[y][x] = '.';
            // }
        }
    }
}

// -------------------------------------------------------------
// traça o corredor horizontalmente para jogador
// -------------------------------------------------------------
void Map::carveHorizontalCorridor(int x1, int x2, int y) {
    for (int x = std::min(x1,x2); x <= std::max(x1,x2); ++x){
        tiles_[y][x] = '.';
    }
}

// -------------------------------------------------------------
// traça o corredor verticalmente para jogador
// -------------------------------------------------------------
void Map::carveVerticalCorridor(int y1, int y2, int x) {
    for (int y = std::min(y1,y2); y <= std::max(y1,y2); ++y){
        tiles_[y][x] = '.';
    }
}

// -------------------------------------------------------------
// metode que gera o mapa e traça corredores para o jogador
// navegar
// -------------------------------------------------------------
void Map::generate(unsigned int seed) {
    rooms_.clear();
    fill('#');

    //gerar sala espaço aleatorio para o jogo
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> randW(4, 10);
    std::uniform_int_distribution<int> randH(3, 7);
    std::uniform_int_distribution<int> randX(1, width_  - 2);
    std::uniform_int_distribution<int> randY(1, height_ - 2);

    //constroi salas aleatorias em volta do mapa gerado aleatoriamente
    const int MAX_ROOMS = 15;
    for (int i = 0; i < MAX_ROOMS; ++i) {
        int rw = randW(rng), rh=randH(rng), rx=randX(rng), ry=randY(rng);
        Room candidate(rx, ry, rw, rh);

        if(candidate.x + candidate.width  < width_  - 1 &&
            candidate.y + candidate.height < height_ - 1)
        {
            bool overlap = false;
            for (const Room& existing : rooms_){
                if(existing.intersects(candidate)){
                    overlap = true;
                    break;
                }
            }
            //se essa sala criada esta dentro de outra sala tente novamente na proxima iteração
            if (overlap) continue;

            carveRoom(candidate);
            //se não estiver vazio a o vetor rooms_ então incremente uma nova sala
            if(!rooms_.empty()){
                Point curr = candidate.center();
                Point prev = rooms_.back().center();
                carveHorizontalCorridor(prev.x, curr.x, prev.y);
                carveVerticalCorridor(prev.y, curr.y, curr.x);
            }
            rooms_.push_back(candidate);
        }

    }
    if(rooms_.empty()){
        rooms_.push_back(Room(width_/2 - 3, height_/2 - 2, 6, 4));
        carveRoom(rooms_.back());
    }
}
