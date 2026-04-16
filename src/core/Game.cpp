// Game.cpp — Implementação do loop principal do jogo

#include "core/Game.hpp"
#include <iostream>
#include <ctime>    // time() — para seed aleatória baseada no relógio
#include <random>
#include <vector>
#include <memory>
#include "entities/EnemyFactory.hpp"
#include <algorithm> 

#ifdef _WIN32
    #include <conio.h>   // _getch() — lê tecla sem precisar apertar Enter (Windows)
#else
    #include <termios.h> // Para leitura de tecla sem Enter no Linux/Mac
    #include <unistd.h>
#endif
#include <string>
#include <deque>
// Função auxiliar: lê uma tecla sem aguardar Enter
// Encapsula a diferença entre Windows e Linux
static char readKey() {
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    // Modo "raw" no terminal Linux: desativa o buffer de linha
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char ch = static_cast<char>(getchar());
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

Game::Game()
    : isRunning_(true)
    , map_(60, 22)        // Mapa maior para caber mais salas
    , player_(2, 2)       // Posição inicial fixa — será ajustada após generate()
    , renderer_()
    , enemies_()            //vetor de ponteiros de inimigos
    , messageLog_()
    , items_()
    , inventarioAberto_(false)
    , statsObserver_(player_.getInventario())
    , logObserver_(messageLog_)
{
    // Gera o dungeon com uma seed baseada no tempo — mapa diferente a cada execução
    // Na Fase 7 (persistência) vamos salvar a seed para recriar o mesmo dungeon
    map_.generate(static_cast<unsigned int>(time(nullptr)));
    
    //variaveis de aletoriedade do spawn de inimigos
    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);
    std::uniform_int_distribution<int> randType(0, 1);
    
    //vetor com tipos de item 
    std::vector enemyTypes = {
        ItemType::PocaoDeForça, 
        ItemType::PocaoDeVidaPequena,
        ItemType::PocaoDeVida,
        ItemType::Espada,
        ItemType::Armadura,
        ItemType::Amuleto
    };
    //id para o vetor de enemyTypes ^

    int tipoIdx = 0;

    //randomizar vetor de inimigos e pegar porcentagem de chance de spawn
    std::shuffle(enemyTypes.begin(), enemyTypes.end(), rng);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    // Posiciona o jogador no centro da primeira sala gerada
    if (!map_.getRooms().empty()) {
        Point start = map_.getRooms().front().center();
        player_ = Player(start.x, start.y);

        //inicializar a visualização do mapa antes do jogador dar o primeiro input
        map_.updateVisibility(player_.getX(), player_.getY());
        
        //conectar observer ao player
        player_.adicionarObserver(&statsObserver_);
        player_.getInventario().onDescarte = [this](const std::string& msg){
            logObserver_.onEvent(msg);
        };

        const std::vector rooms = map_.getRooms();
        //começa na sala 1 (int i = 1) para pular a sala do jogador
        for (int i = 1; i < rooms.size(); i++)
        {
            // pega o centro da room atual
            Point centroRoom = rooms[i].center();

            //spam de enemy no centro da sala
            EnemyType tipo = (randType(rng) == 0) ? EnemyType::Goblin : EnemyType::Troll;
            enemies_.push_back(EnemyFactory::create(tipo, centroRoom.x, centroRoom.y));

            //chance de um inimigo spawnar
            if (chance(rng) < 0.9f) {
                //pegar espaço aleatorio da sala
                const Room& salaAtual = rooms[i];
                std::uniform_int_distribution<int> rx(salaAtual.x + 1, salaAtual.x + salaAtual.width - 2);
                std::uniform_int_distribution<int> ry(salaAtual.y + 1, salaAtual.y + salaAtual.height - 2);
                int salaX = rx(rng), salaY = ry(rng);

                //escolher o tipo de item pelo indice
                ItemType tipoInimigo = enemyTypes[tipoIdx % enemyTypes.size()];
                tipoIdx++;
                //criar o item na sala
                items_.push_back(ItemFactory::create(tipoInimigo, salaX, salaY));
            }

        }
    }

    
}

void Game::run() {
    // O loop de turno do roguelike:
    // Diferente de um jogo de tempo real, só processamos quando o jogador age.
    // O mundo "para" enquanto o jogador pensa — característica central do roguelike.
    while (isRunning_) {
        render();        // 1. Mostra o estado atual
        processInput();  // 2. Espera e lê a ação do jogador
        update();        // 3. Atualiza o mundo em resposta à ação
    }
    if (!player_.isAlive()) {
        std::cout << "\nVoce morreu... Tente novamente!\n";
    }else{
        std::cout << "\nAte a proxima aventura!\n";
    }
}

void Game::processInput() {
    char key = readKey();

    // Converte para minúsculo para aceitar WASD e wasd
    key = static_cast<char>(tolower(key));

    switch (key) {
        case 'w': player_.move( 0, -1, map_, enemies_); break;  // cima
        case 's': player_.move( 0,  1, map_, enemies_); break;  // baixo
        case 'a': player_.move(-1,  0, map_, enemies_); break;  // esquerda
        case 'd': player_.move( 1,  0, map_, enemies_); break;  // direita

        case 'g': coletarItem(); break;
        case 'u': usarConsumivel(); break;
        case 'i': inventarioAberto_ = !inventarioAberto_; break;
        case '1': case '2': case '3': case '4': case '5':
            if (inventarioAberto_) usarConsumivelInventario(key - '1'); break;

        case 'e': if (inventarioAberto_) equiparSelecionado(); break;
        case 'x': if (inventarioAberto_) desequiparSelecionado(); break; 

        case 'q': isRunning_ = false;          break;  // sair
        default: break;  // Tecla desconhecida — ignora, não faz nada
    }
}
void Game::coletarItem() {
    //se houver item no mesmo lugar que o player esta então tenta adicionar ao inventario dele;
    int playerX = player_.getX();
    int playerY = player_.getY();

    for(int i = 0; i < items_.size(); i++ ){
        if(
            items_[i]->getX() == playerX &&
            items_[i]->getY() == playerY )
        {
            std::string nomeItem = items_[i]->getNome();
            bool coletado = player_.getInventario().adicionarItem(std::move(items_[i]));
            items_.erase(items_.begin() + i); 
            if(coletado){
                logObserver_.onEvent("Item coletado: " + nomeItem);
            }else{
                logObserver_.onEvent("Inventario cheio...");
            }
            return;
        }
    }

}
void Game::usarConsumivel(){
    for(int i = 0; i < items_.size(); i++ ){
        if(
            items_[i]->getX() == player_.getX() &&
            items_[i]->getY() == player_.getY()  &&
            items_[i]->getSlot() == ItemSlot::Consumivel)
        {
            std::string nomeItem = items_[i]->getNome();
            items_[i]->usar(player_);
            items_.erase(items_.begin() + i);
            logObserver_.onEvent("Item " + nomeItem + " consumido.");
        }
    }
}


void Game::usarConsumivelInventario(int index){
    auto item = player_.getInventario().removerConsumivel(index);
    if (!item) return;

    std::string nomeItem = item->getNome();
    item->usar(player_);
    logObserver_.onEvent("Item " + nomeItem + " usado.");
}

void Game::equiparSelecionado(){
    auto& inventario = player_.getInventario();
    auto& consumivel = inventario.getConsumiveis();
    if(consumivel.empty()) {
        logObserver_.onEvent("Nenhum item no inventario");
        return;
    };
    ItemSlot slot = consumivel[0]->getSlot();
    std::string nomeItem = consumivel[0]->getNome();

    inventario.equipar(slot, 0);
    player_.notificarObservers();
    logObserver_.onEvent("Item " + nomeItem + " equipado.");
}

void Game::desequiparSelecionado(){
    auto& inventario = player_.getInventario();
    for (ItemSlot s : {ItemSlot::Arma, ItemSlot::Armadura, ItemSlot::Acessorio}){
        if(inventario.getEquipado(s)){
            inventario.desequipar(s);
            player_.notificarObservers();

            logObserver_.onEvent("Item desequipado.");
        }
    }
    
}

void Game::update() {

    player_.update();
            
    //loop para o inimigo reagir quando o jogador consegue vê-lo
    for (auto& enemy : enemies_) {
        if(enemy->isAlive()){
            if(map_.isExplored(enemy->getX(), enemy->getY())){
                enemy->update(map_, player_);
            }
        }
    }
    //loop para jogador receber xp de inimigo derrotado
    for (auto& enemy : enemies_) {
        if(!enemy->isAlive()){
            std::string mensagem = player_.addXP(enemy->getXPReward());
            if(!mensagem.empty()){
                pushMessage(mensagem);
            }
        }
    }
    //vialização do mapa pelo jogador
    map_.updateVisibility(player_.getX(), player_.getY());


    //limpar inimigos caso tenham morrido
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<Enemy>& e) { 
                return !e->isAlive(); 
            }),
        enemies_.end()
    );
    if (!player_.isAlive()) {
        isRunning_ = false;
    }
}

void Game::render() {
    renderer_.render(map_, player_, enemies_, items_ ,messageLog_, inventarioAberto_);
}

void Game::pushMessage(const std::string& message) {
    if(messageLog_.size() == 3) messageLog_.pop_front();
    messageLog_.push_back(message);
}
