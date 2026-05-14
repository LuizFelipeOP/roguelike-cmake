#include "FugirStrategy.hpp"
#include <algorithm>
#include "../entities/Enemy.hpp"
#include "../entities/Player.hpp"
#include "../map/Map.hpp"

void FugirStrategy::agir(Enemy& self, Map& map, Player& player) {

    int distX = player.getX() - self.getX();
    int distY = player.getY() - self.getY();

    int distancia = std::max(std::abs(distX), std::abs(distY));

    if (distancia <= 6) {

        int stepX = (distX > 0) ? 1 : (distX < 0) ? -1 : 0;
        int stepY = (distY > 0) ? 1 : (distY < 0) ? -1 : 0;

        if (map.isWalkable(self.getX() - stepX, self.getY() - stepY)) {
            self.moverPara(self.getX() - stepX, self.getY() - stepY);
        } else if (map.isWalkable(self.getX(), self.getY() - stepY)) {
            self.moverPara(self.getX(), self.getY() - stepY);
        } else if (map.isWalkable(self.getX() - stepX, self.getY())) {
            self.moverPara(self.getX() - stepX, self.getY());
        }
    }
}
 