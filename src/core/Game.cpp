// Game.cpp — Implementação do loop principal do jogo

#include "core/Game.hpp"
#ifdef ROGUELIKE_SDL
#include <SDL.h>
#include "ui/SDLRenderer.hpp"
#endif
#include <iostream>
#include <ctime>
#include <random>
#include <vector>
#include <memory>
#include "entities/EnemyFactory.hpp"
#include <algorithm>
#include "persistence/SaveSystem.hpp"
#include "effects/VenenoEffect.hpp"
#include "effects/ParalisiaEffect.hpp"
#include "effects/RegeneracaoEffect.hpp"
#include "effects/EnfraquecimentoEffect.hpp"
#include <string>
#include <deque>

static const std::string SAVE_PATH = "savegame.json";

Game::Game(SDL_Renderer* sdlRenderer,
     const std::string& spriteDir,
     const std::string& fontPath)
    : isRunning_(true)
    , estado_(EstadoJogo::Menu)
    , inimigosDestruidos_(0)
    , historicoLog_()
    , map_(60, 22)
    , player_(2, 2)
#ifdef ROGUELIKE_SDL
    , renderer_(sdlRenderer ? new SDLRenderer(sdlRenderer, spriteDir, fontPath) : nullptr)
#else
    , renderer_(nullptr)
#endif
    , enemies_()
    , messageLog_()
    , items_()
    , inventarioAberto_(false)
    , aguardandoDescarte_(false)
    , statsObserver_(player_.getInventario())
    , logObserver_(messageLog_)
    , andarAtual_(1)
{
    //conectar observer ao player
    player_.adicionarObserver(&statsObserver_);
    player_.getInventario().onDescarte = [this](const std::string& msg){
        logObserver_.onEvent(msg);
    };
    player_.onEfeitoEvento = [this](const std::string& msg){
        pushMessage(msg);
    };
    // NÃO chama inicializarAndar() aqui — o andar só inicia ao escolher "Nova Partida"
}

Game::~Game() {
#ifdef ROGUELIKE_SDL
    delete renderer_;
#endif
}

void Game::run() {
#ifdef ROGUELIKE_SDL
    while (isRunning_) {
        SDL_Event evento;
        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_QUIT) { isRunning_ = false; break; }
            if(evento.type == SDL_KEYDOWN)
                processInput(evento.key.keysym.sym);
        }
        if (renderer_) renderer_->clear();
        render();
        if (renderer_) renderer_->present();
        if (estado_ == EstadoJogo::Jogando)
            update();
        SDL_Delay(16);
    }
    std::cout << "\nAte a proxima aventura!\n";
#endif
}

void Game::processInput(int key) {
#ifdef ROGUELIKE_SDL
    switch (estado_) {

        case EstadoJogo::Menu:
            if (key == SDLK_ESCAPE) { isRunning_ = false; return; }
            if (key == SDLK_n) {
                inimigosDestruidos_ = 0;
                historicoLog_.clear();
                messageLog_.clear();
                player_ = Player(2, 2);
                player_.adicionarObserver(&statsObserver_);
                player_.getInventario().onDescarte = [this](const std::string& msg){
                    logObserver_.onEvent(msg);
                };
                player_.onEfeitoEvento = [this](const std::string& msg){
                    pushMessage(msg);
                };
                andarAtual_ = 1;
                enemies_.clear();
                items_.clear();
                inicializarAndar();
                estado_ = EstadoJogo::Jogando;
            }
            if (key == SDLK_c) {
                if (SaveSystem::existeSave(SAVE_PATH)) {
                    carregar();
                    estado_ = EstadoJogo::Jogando;
                }
            }
            return;

        case EstadoJogo::Morto:
            if (key == SDLK_ESCAPE) { isRunning_ = false; return; }
            if (key == SDLK_h) { estado_ = EstadoJogo::Historico; return; }
            if (key == SDLK_m) { estado_ = EstadoJogo::Menu; return; }
            return;
        case EstadoJogo::Historico:
            if (key == SDLK_ESCAPE || key == SDLK_h) { estado_ = EstadoJogo::Morto; return; }
            return;

        case EstadoJogo::Jogando:
            if (key == 27) { isRunning_ = false; return; }
            if (key == SDLK_s && SDL_GetModState() & KMOD_SHIFT) { 
                salvar();   return; 
            }
            if (key == SDLK_l && SDL_GetModState() & KMOD_SHIFT) { 
                carregar(); return; 
            }
            if (key == SDLK_h) {
                estado_ = EstadoJogo::Historico;
                return;
            }
            key = static_cast<char>(tolower(key));
            switch (key) {
                case SDLK_w: player_.move( 0, -1, map_, enemies_); break;
                case SDLK_s: player_.move( 0,  1, map_, enemies_); break;
                case SDLK_a: player_.move(-1,  0, map_, enemies_); break;
                case SDLK_d: player_.move( 1,  0, map_, enemies_); break;
                case SDLK_g: coletarItem(); break;
                case SDLK_u: usarConsumivel(); break;
                case SDLK_i: inventarioAberto_ = !inventarioAberto_; break;
                case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
                    if (aguardandoDescarte_) {
                        descartarItem(key - SDLK_1);
                        aguardandoDescarte_ = false;
                    } else if (inventarioAberto_) {
                        usarConsumivelInventario(key - SDLK_1);
                    }
                    break;
                case SDLK_e: if (inventarioAberto_) equiparSelecionado(); break;
                case SDLK_x: if (inventarioAberto_) desequiparSelecionado(); break;
                case SDLK_r: if (inventarioAberto_) { aguardandoDescarte_ = true; pushMessage("Pressione 1-5 para descartar."); } break;
                default: break;
            }
            
            return;
    }
#endif
}

void Game::inicializarAndar(){
    // Gera o dungeon com uma seed baseada no tempo — mapa diferente a cada execução
    map_.generate(static_cast<unsigned int>(time(nullptr)));
    
    Point posEscada = map_.getPosicaoEscada();
    
    //variaveis de aletoriedade do spawn de inimigos
    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)) + 1);
    
    //randomizar vetor de inimigos e pegar porcentagem de chance de spawn
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    // Posiciona o jogador no centro da primeira sala gerada
    if (!map_.getRooms().empty()) {
        Point start = map_.getRooms().front().center();
        player_.setPosition(start.x, start.y);

        //inicializar a visualização do mapa antes do jogador dar o primeiro input
        map_.calcularVisibilidade(player_.getX(), player_.getY());
        

        const std::vector rooms = map_.getRooms();
        //começa na sala 1 (int i = 1) para pular a sala do jogador
        for (int i = 1; i < rooms.size(); i++)
        {
            // pega o centro da room atual
            Point centroRoom = rooms[i].center();

            //spam de enemy no centro da sala — probabilidade varia por andar
            EnemyType tipo = EnemyFactory::sortear(andarAtual_, rng);
            // enemies_.push_back(EnemyFactory::create(tipo, centroRoom.x, centroRoom.y, andarAtual_));

            //chance de um item spawnar
            if (chance(rng) < 0.7f) {
                //pegar espaço aleatorio da sala
                const Room& salaAtual = rooms[i];
                std::uniform_int_distribution<int> rx(salaAtual.x + 1, salaAtual.x + salaAtual.width - 2);
                std::uniform_int_distribution<int> ry(salaAtual.y + 1, salaAtual.y + salaAtual.height - 2);
                int salaPosicaoX, salaPosicaoY;

                do{ 
                    salaPosicaoX = rx(rng);
                    salaPosicaoY = ry(rng);
                } while (
                    (salaPosicaoX == posEscada.x && salaPosicaoY == posEscada.y) ||
                    std::any_of(items_.begin(), items_.end(), [&](const auto& item){
                        return item->getX() == salaPosicaoX && item->getY() == salaPosicaoY;
                    })
                );

                //criar o item na sala
                items_.push_back(ItemFactory::create(salaPosicaoX, salaPosicaoY, andarAtual_));
            }

        }
    }
}

void Game::descerAndar(){
    andarAtual_++;
    // map_.generate(static_cast<unsigned int>(time(nullptr)));

    enemies_.clear();
    items_.clear();

    // Point start = map_.getRooms().front().center();
    // player_ = Player(start.x, start.y);

    inicializarAndar();
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
            if(player_.getInventario().cheio(items_[i]->getSlot())){ //retorna se ja tem slot cheio do mesmo item
                logObserver_.onEvent("Slot de item cheio...");
                return;
            }
            bool coletado = player_.getInventario().adicionarItem(std::move(items_[i]));
            items_.erase(items_.begin() + i); 
            if(coletado){
                logObserver_.onEvent("Item coletado: " + nomeItem);
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
    auto& consumiveis = player_.getInventario().getConsumiveis();
    if(index < 0 || index >= consumiveis.size()) return;

    if(consumiveis[index]->getSlot() != ItemSlot::Consumivel){
        logObserver_.onEvent("Este item n eh consumivel.");
        return;
    }


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

void Game::descartarItem(int index){
    auto& consumiveis = player_.getInventario().getConsumiveis();
    if(index < 0 || index >= consumiveis.size()) return;
    
    std::string nomeItem = consumiveis[index]->getNome();

    auto item = player_.getInventario().removerConsumivel(index);
    if (!item) return;

    item->setPosicao(player_.getX(), player_.getY());
    
    items_.push_back(std::move(item));

    logObserver_.onEvent("Item " + nomeItem + " removido.");
}

void Game::update() {

    player_.update();
    player_.tickEfeitos();
    //posição da escada do andar atual.
    Point escada = map_.getPosicaoEscada();

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
    map_.calcularVisibilidade(player_.getX(), player_.getY());


    //limpar inimigos caso tenham morrido
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [this](const std::unique_ptr<Enemy>& e) { 
                if (!e->isAlive()) {
                    inimigosDestruidos_++;
                    return true;
                }
                return false;
            }),
        enemies_.end()
    );
    if (!player_.isAlive()) {
        estado_ = EstadoJogo::Morto;
    }

    if(player_.getX() == escada.x && 
        player_.getY() == escada.y){
            descerAndar();
        }
}

void Game::render() {
#ifdef ROGUELIKE_SDL
    if (!renderer_) return;
    switch (estado_) {
        case EstadoJogo::Menu:
            renderer_->renderMenu(SaveSystem::existeSave(SAVE_PATH));
            break;
        case EstadoJogo::Jogando:
            renderer_->render(map_, player_, enemies_, items_, messageLog_, inventarioAberto_, andarAtual_, &player_.getInventario());
            break;
        case EstadoJogo::Morto:
            renderer_->renderTelaDerrota(andarAtual_, player_.getLevel(), player_.getXP(), inimigosDestruidos_);
            break;
        case EstadoJogo::Historico:
            renderer_->renderHistorico(historicoLog_);
            break;
    }
#endif
}

void Game::pushMessage(const std::string& message) {
    if(messageLog_.size() == 3) messageLog_.pop_front();
    messageLog_.push_back(message);
    historicoLog_.push_back(message);
}

void Game::salvar() {
    // ── Helpers de serialização (enum → string) ──────────────────────────────
    auto itemTipoStr = [](ItemType t) -> std::string {
        switch (t) {
            case ItemType::PocaoDeVidaPequena:  return "PocaoDeVidaPequena";
            case ItemType::PocaoDeVida:         return "PocaoDeVida";
            case ItemType::PocaoDeForça:        return "PocaoDeForça";
            case ItemType::PocaoDeRegeneracao:  return "PocaoDeRegeneracao";
            case ItemType::Espada:              return "Espada";
            case ItemType::EspadaGrande:        return "EspadaGrande";
            case ItemType::Armadura:            return "Armadura";
            case ItemType::Amuleto:             return "Amuleto";
            default:                            return "Desconhecido";
        }
    };
    auto itemSlotStr = [](ItemSlot s) -> std::string {
        switch (s) {
            case ItemSlot::Arma:       return "Arma";
            case ItemSlot::Armadura:   return "Armadura";
            case ItemSlot::Acessorio:  return "Acessorio";
            case ItemSlot::Consumivel: return "Consumivel";
            default:                   return "Consumivel";
        }
    };

    GameMemento m;

    // ── Player ───────────────────────────────────────────────────────────────
    m.player.x            = player_.getX();
    m.player.y            = player_.getY();
    m.player.hp           = player_.getHp();
    m.player.maxHp        = player_.getMaxHp();
    m.player.attack       = player_.getBaseAttack();
    m.player.defense      = player_.getBaseDefense();
    m.player.luck         = player_.getLuck();
    m.player.xp           = player_.getXP();
    m.player.level        = player_.getLevel();
    m.player.xpProxLevel  = player_.getXPProxLevel();
    m.player.attackBonus  = player_.getAttackBonus();
    m.player.defenseBonus = player_.getDefenseBonus();
    for (const auto& ef : player_.getEfeitos())
        m.player.efeitos.push_back({ ef->getNome(), ef->getDuracao(), ef->getPotencia() });

    // ── Inventário (consumíveis não equipados) ────────────────────────────────
    for (const auto& item : player_.getInventario().getConsumiveis()) {
        ItemMemento im;
        im.tipo     = itemTipoStr(item->getTipo());
        im.x = -1; im.y = -1;
        im.equipado = false;
        im.slot     = itemSlotStr(item->getSlot());
        m.itensInventario.push_back(im);
    }
    // Itens equipados
    for (ItemSlot s : {ItemSlot::Arma, ItemSlot::Armadura, ItemSlot::Acessorio}) {
        const Item* eq = player_.getInventario().getEquipado(s);
        if (eq) {
            ItemMemento im;
            im.tipo     = itemTipoStr(eq->getTipo());
            im.x = -1; im.y = -1;
            im.equipado = true;
            im.slot     = itemSlotStr(eq->getSlot());
            m.itensInventario.push_back(im);
        }
    }

    // ── Itens no chão ────────────────────────────────────────────────────────
    for (const auto& item : items_) {
        ItemMemento im;
        im.tipo     = itemTipoStr(item->getTipo());
        im.x        = item->getX();
        im.y        = item->getY();
        im.equipado = false;
        im.slot     = itemSlotStr(item->getSlot());
        m.itensMapa.push_back(im);
    }

    // ── Inimigos ─────────────────────────────────────────────────────────────
    for (const auto& enemy : enemies_) {
        EnemyMemento em;
        em.tipo     = enemy->getTipoNome();
        em.x        = enemy->getX();
        em.y        = enemy->getY();
        em.hp       = enemy->getHp();
        em.iaEstado = (enemy->getIAEstado() == IAEstado::Fugindo) ? "Fugindo" : "Perseguindo";
        for (const auto& ef : enemy->getEfeitos())
            em.efeitos.push_back({ ef->getNome(), ef->getDuracao(), ef->getPotencia() });
        m.inimigos.push_back(em);
    }

    // ── Mapa ─────────────────────────────────────────────────────────────────
    Point escada       = map_.getPosicaoEscada();
    m.mapa.seed        = map_.getSeed();
    m.mapa.andar       = andarAtual_;
    m.mapa.explored    = map_.getExplored();
    m.mapa.escadaX     = escada.x;
    m.mapa.escadaY     = escada.y;

    SaveSystem::escrever(m, SAVE_PATH);
    pushMessage("Jogo salvo!");
}

void Game::carregar() {
    if (!SaveSystem::existeSave(SAVE_PATH)) {
        pushMessage("Nenhum save encontrado.");
        return;
    }

    GameMemento m = SaveSystem::ler(SAVE_PATH);

    // ── Helper: EfeitoMemento → StatusEffect ─────────────────────────────────
    auto criarEfeito = [](const EfeitoMemento& ef) -> std::unique_ptr<StatusEffect> {
        if (ef.tipo == "Veneno")           return std::make_unique<VenenoEffect>(ef.potencia, ef.duracaoRestante);
        if (ef.tipo == "Paralisia")        return std::make_unique<ParalisiaEffect>(ef.duracaoRestante);
        if (ef.tipo == "Regeneracao")      return std::make_unique<RegeneracaoEffect>(ef.potencia, ef.duracaoRestante);
        if (ef.tipo == "Enfraquecimento")  return std::make_unique<EnfraquecimentoEffect>(ef.potencia, ef.duracaoRestante);
        return nullptr;
    };

    // ── Helper: string → EnemyType ───────────────────────────────────────────
    auto enemyTipo = [](const std::string& s) -> EnemyType {
        if (s == "Troll") return EnemyType::Troll;
        if (s == "Orc")   return EnemyType::Orc;
        return EnemyType::Goblin;
    };

    // ── Restaurar mapa (reconstrói o layout a partir da seed) ────────────────
    map_.generate(m.mapa.seed);
    map_.setExplored(m.mapa.explored);
    andarAtual_ = m.mapa.andar;

    // ── Restaurar player ─────────────────────────────────────────────────────
    player_.setPosition(m.player.x, m.player.y);
    player_.setHp(m.player.hp);
    player_.setMaxHp(m.player.maxHp);
    player_.setAttack(m.player.attack);
    player_.setDefense(m.player.defense);
    player_.setLuck(m.player.luck);
    player_.setXP(m.player.xp);
    player_.setLevel(m.player.level);
    player_.setXPProxLevel(m.player.xpProxLevel);
    player_.setAttackBonus(m.player.attackBonus);
    player_.setDefenseBonus(m.player.defenseBonus);
    player_.limparEfeitos();
    for (const auto& ef : m.player.efeitos) {
        auto efeito = criarEfeito(ef);
        if (efeito) player_.adicionarEfeito(std::move(efeito));
    }

    // ── Restaurar inimigos ───────────────────────────────────────────────────
    enemies_.clear();
    for (const auto& em : m.inimigos) {
        auto enemy = EnemyFactory::create(enemyTipo(em.tipo), em.x, em.y, andarAtual_);
        enemy->setHp(em.hp);
        if (em.iaEstado == "Fugindo") enemy->setIAEstado(IAEstado::Fugindo);
        enemy->limparEfeitos();
        for (const auto& ef : em.efeitos) {
            auto efeito = criarEfeito(ef);
            if (efeito) enemy->adicionarEfeito(std::move(efeito));
        }
        enemies_.push_back(std::move(enemy));
    }

    // ── Restaurar itens no chão ──────────────────────────────────────────────
    items_.clear();
    for (const auto& im : m.itensMapa) {
        auto item = ItemFactory::createByTipo(im.tipo, im.x, im.y);
        if (item) items_.push_back(std::move(item));
    }

    // ── Restaurar inventário ─────────────────────────────────────────────────
    auto& inv = player_.getInventario();
    inv.limpar();

    // Equipados primeiro (adicionarItem → equip slot diretamente via adicionarItem)
    for (const auto& im : m.itensInventario) {
        if (!im.equipado) continue;
        auto item = ItemFactory::createByTipo(im.tipo, -1, -1);
        if (!item) continue;
        // adicionarItem de arma/armadura/acessorio vai direto ao slot (não consumível)
        inv.adicionarItem(std::move(item));
    }
    // Consumíveis depois
    for (const auto& im : m.itensInventario) {
        if (im.equipado) continue;
        auto item = ItemFactory::createByTipo(im.tipo, -1, -1);
        if (item) inv.adicionarItem(std::move(item));
    }

    pushMessage("Jogo carregado!");
}
