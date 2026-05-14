#include "PerseguirStrategy.hpp"
#include "../entities/Enemy.hpp"
#include "../entities/Player.hpp"
#include "../map/Map.hpp"
#include <algorithm>
#include <cstdlib>

PerseguirStrategy::PerseguirStrategy(std::function<void(Player&)> onAtaque)
    : onAtaque_(std::move(onAtaque)) {}

void PerseguirStrategy::agir(Enemy& self, Map& map, Player& player) {
    int playerX = player.getX();
    int playerY = player.getY();

    int distX = playerX - self.getX();
    int distY = playerY - self.getY();

    int distancia = std::max(std::abs(distX), std::abs(distY));

    if (distancia == 1) {
        player.takeDamage(self.getAttack());
        if (onAtaque_) onAtaque_(player);

    } else if (distancia <= 8) {
        int stepX = (distX > 0) ? 1 : (distX < 0) ? -1 : 0;
        int stepY = (distY > 0) ? 1 : (distY < 0) ? -1 : 0;

        if (map.isWalkable(self.getX() + stepX, self.getY() + stepY)) {
            self.moverPara(self.getX() + stepX, self.getY() + stepY);
        } else if (map.isWalkable(self.getX(), self.getY() + stepY)) {
            self.moverPara(self.getX(), self.getY() + stepY);
        } else if (map.isWalkable(self.getX() + stepX, self.getY())) {
            self.moverPara(self.getX() + stepX, self.getY());
        }
    }
}
