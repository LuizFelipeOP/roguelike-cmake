
#include "SaveSystem.hpp"
#include "../items/Item.hpp"


void SaveSystem::escrever(const GameMemento& memento, const std::string& caminho){

}
GameMemento ler(const std::string& caminho){

}
bool existeSave(const std::string& caminho){

}

std::string itemTipoParaString(ItemType t);
ItemType stringParaItemTipo(const std::string& s);
// - `escrever()` serializa `GameMemento` para JSON e grava em disco.
// - `ler()` lê o JSON e preenche um `GameMemento`.
// - `existeSave()` verifica se o arquivo existe antes de tentar carregar.

// ### Estratégia de serialização de enums

// Enums são salvos como strings para legibilidade:

// ```json
// { "tipo": "Goblin", "iaEstado": "Perseguindo" }
// ```

// Funções auxiliares internas no `SaveSystem.cpp`:
// ```cpp
// static std::string itemTipoParaString(ItemType t);
// static ItemType stringParaItemTipo(const std::string& s);
// idem para EnemyType, IAEstado, ItemSlot