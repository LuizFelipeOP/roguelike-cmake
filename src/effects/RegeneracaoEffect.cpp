#include "RegeneracaoEffect.hpp"
#include "../entities/Entity.hpp"

RegeneracaoEffect::RegeneracaoEffect(int cura, int duracao)
    : cura_(cura), duracaoRestante_(duracao) {}

void RegeneracaoEffect::tick(Entity& alvo) {
    alvo.curar(cura_);
    --duracaoRestante_;
}

bool RegeneracaoEffect::isExpired() const {
    return duracaoRestante_ <= 0;
}

std::string RegeneracaoEffect::getNome() const {
    return "Regeneracao";
}

int RegeneracaoEffect::getPotencia() const {
    return cura_;
}
int RegeneracaoEffect::getDuracao() const {
    return duracaoRestante_;
}