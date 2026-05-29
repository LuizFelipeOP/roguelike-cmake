#include "map/Map.hpp"
#include <algorithm>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// constroi o mapa, onde temos y e x, onde os indices 0 e Max
// são paredes (#) e o resto é chão '.'
Map::Map(int width, int height)
    : width_(width), height_(height), estilo_(1) {

    tiles_.assign(height, std::vector<char>(width, '.'));
    explored_.assign(height, std::vector<bool>(width, false));
    visible_.assign(height, std::vector<bool>(width, false));
    wallVariant_.assign(height, std::vector<int>(width, 0));
    roomOf_.assign(height, std::vector<int>(width, -1));

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

    seed_ = (seed == 0) ? static_cast<unsigned int>(time(nullptr)) : seed;

    fill('#');
    for (auto& row : explored_)
        std::fill(row.begin(), row.end(), false);
    for (auto& row : visible_)
        std::fill(row.begin(), row.end(), false);
    for (auto& row : roomOf_)
        std::fill(row.begin(), row.end(), -1);

    // Estilo visual aleatório do andar (1–6)
    std::mt19937 rng(seed_);
    estilo_ = 1 + (static_cast<int>(rng() % 6));
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
    //logica de posicionar escada em lugar aleatorio.
    if(rooms_.size() > 1){
        std::uniform_int_distribution<int> salaAleatoria(1, rooms_.size() - 1);
        int indiceSalaAleatoria = salaAleatoria(rng);
        
        const Room& sala = rooms_[indiceSalaAleatoria];

        std::vector<Point> cantos = {
            {sala.x + 1              , sala.y + 1},
            {sala.x + sala.width - 2 , sala.y + 1},
            {sala.x + 1              , sala.y + sala.height - 2},
            {sala.x + sala.width - 2 , sala.y + sala.height - 2},
        };

        std::uniform_int_distribution<int> randCantoSala(0, 3);
        escada_ = cantos[randCantoSala(rng)];

        desenharEscada(escada_.x, escada_.y);
    }
    if(rooms_.empty()){
        rooms_.push_back(Room(width_/2 - 3, height_/2 - 2, 6, 4));
        carveRoom(rooms_.back());
    }

    // ── Preencher roomOf_: marca chão e paredes ao redor de cada sala
    // Estende o bounding box em 1 para incluir as paredes que cercam a sala,
    // assim renderTileParede sabe qual thema usar nessas paredes.
    for (int i = 0; i < static_cast<int>(rooms_.size()); ++i) {
        const Room& r = rooms_[i];
        for (int y = r.y - 1; y <= r.y + r.height; ++y)
            for (int x = r.x - 1; x <= r.x + r.width; ++x)
                if (y >= 0 && y < height_ && x >= 0 && x < width_)
                    roomOf_[y][x] = i;
    }

    // ── Preencher wallVariant_: hash por posição + seed para variantes aleatórias
    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
            if (tiles_[y][x] == '#')
                wallVariant_[y][x] = static_cast<int>(
                    (seed_ ^ (static_cast<unsigned>(x) * 2654435761u)
                            ^ (static_cast<unsigned>(y) * 2246822519u)) % 10
                );
}
// ─────────────────────────────────────────────────────────────────────────────
// calcularVisibilidade — ray casting com linha de Bresenham
//
// Para cada ângulo de 0° a 360° (NUM_RAIOS raios), avança tile a tile a partir
// do player. Cada tile atingido é marcado como visible_ e explored_.
// Paredes bloqueiam o raio (são marcadas, mas param a propagação).
// ─────────────────────────────────────────────────────────────────────────────
void Map::calcularVisibilidade(int px, int py, int raio) {
    // Zera visibilidade atual
    for (auto& row : visible_)
        std::fill(row.begin(), row.end(), false);

    const int NUM_RAIOS = 360;
    const double PASSO  = 2.0 * M_PI / NUM_RAIOS;

    for (int i = 0; i < NUM_RAIOS; ++i) {
        double angulo = i * PASSO;
        double dx = std::cos(angulo);
        double dy = std::sin(angulo);

        double rx = static_cast<double>(px) + 0.5;
        double ry = static_cast<double>(py) + 0.5;

        for (int passo = 0; passo < raio; ++passo) {
            int tx = static_cast<int>(rx);
            int ty = static_cast<int>(ry);

            if (tx < 0 || tx >= width_ || ty < 0 || ty >= height_) break;

            visible_[ty][tx]  = true;
            explored_[ty][tx] = true;

            if (tiles_[ty][tx] == '#') break; // parede bloqueia

            rx += dx;
            ry += dy;
        }
    }
}

bool Map::isVisible(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    return visible_[y][x];
}

bool Map::isExplored(int x, int y) const {
    //valida se esta fora dos limites
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    return explored_[y][x];
}

Point Map::getPosicaoEscada() const {
    return escada_;
}

// -------------------------------------------------------------
// Desenha escada no mapa
// -------------------------------------------------------------
void Map::desenharEscada(int x, int y) {
    tiles_[y][x] = '>';
}

const std::vector<std::vector<bool>>& Map::getExplored() const {
    return explored_;
}

void Map::setExplored(const std::vector<std::vector<bool>>& explored) {
    explored_ = explored;
}

unsigned int Map::getSeed() const {
    return seed_;
}

int Map::getEstilo() const {
    return estilo_;
}

int Map::getWallVariant(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return 0;
    return wallVariant_[y][x];
}

int Map::getRoomIndex(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return -1;
    return roomOf_[y][x];
}