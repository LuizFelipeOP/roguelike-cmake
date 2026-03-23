// Game.cpp — Implementação do loop principal do jogo

#include "core/Game.hpp"
#include <iostream>
#include <ctime>    // time() — para seed aleatória baseada no relógio
#include <random>
#include <vector>
#include <memory>
#include "entities/EnemyFactory.hpp"

#ifdef _WIN32
    #include <conio.h>   // _getch() — lê tecla sem precisar apertar Enter (Windows)
#else
    #include <termios.h> // Para leitura de tecla sem Enter no Linux/Mac
    #include <unistd.h>
#endif

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
{
    // Gera o dungeon com uma seed baseada no tempo — mapa diferente a cada execução
    // Na Fase 7 (persistência) vamos salvar a seed para recriar o mesmo dungeon
    map_.generate(static_cast<unsigned int>(time(nullptr)));
    
    //variaveis de aletoriedade do spawn de inimigos
    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);
    std::uniform_int_distribution<int> randType(0, 1);

    // Posiciona o jogador no centro da primeira sala gerada
    if (!map_.getRooms().empty()) {
        Point start = map_.getRooms().front().center();
        player_ = Player(start.x, start.y);

        const std::vector rooms = map_.getRooms();
        //começa na sala 1 (int i = 1) para pular a sala do jogador
        for (int i = 1; i < rooms.size(); i++)
        {
            // pega o centro da room atual
            Point centroRoom = rooms[i].center();
            EnemyType tipo = (randType(rng) == 0) ? EnemyType::Goblin : EnemyType::Troll;
            enemies_.push_back(EnemyFactory::create(tipo, centroRoom.x, centroRoom.y));
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

    std::cout << "\nAté a próxima aventura!\n";
}

void Game::processInput() {
    char key = readKey();

    // Converte para minúsculo para aceitar WASD e wasd
    key = static_cast<char>(tolower(key));

    switch (key) {
        case 'w': player_.move( 0, -1, map_); break;  // cima
        case 's': player_.move( 0,  1, map_); break;  // baixo
        case 'a': player_.move(-1,  0, map_); break;  // esquerda
        case 'd': player_.move( 1,  0, map_); break;  // direita
        case 'q': isRunning_ = false;          break;  // sair
        default: break;  // Tecla desconhecida — ignora, não faz nada
    }
}

void Game::update() {
    // Por enquanto apenas atualiza o jogador
    // Na Fase 3 este método vai iterar por todos os inimigos também
    player_.update();

    for (auto& enemy : enemies_) {
        if(enemy->isAlive()){
            enemy->update(map_, player_);
        }
    }
}

void Game::render() {
    renderer_.render(map_, player_);
}
