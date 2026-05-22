#include "ui/Renderer.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#endif

static Cor corDoEfeito(const std::string& nome) {
    if (nome == "Paralisia")       return Cor::Amarelo;
    if (nome == "Veneno")          return Cor::Verde;
    if (nome == "Regeneracao")     return Cor::Ciano;
    if (nome == "Enfraquecimento") return Cor::Vermelho;
    return Cor::Padrao;
}

void Renderer::render(
        const Map& map, 
        const Player& player, 
        const std::vector<std::unique_ptr<Enemy>>& enemies, 
        const std::vector<std::unique_ptr<Item>>& items,        
        const std::deque<std::string>& messageLog,
        bool inventarioAberto,
        int andarAtual
    ) {
    console_.ocultarCursor();
    console_.limpar();  // move cursor para (0,0) — sem apagar, sem flicker

    // Mapa: largura fixa, '\n' simples é suficiente
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            if (x == player.getX() && y == player.getY()) {
                auto nomes = player.getEfeitosNomes();
                setCorEfeito(nomes);
                std::cout << player.getSymbol();
                console_.resetColor();
                continue;
            }
            
            if (map.isExplored(x, y)) {
                bool hadEnemy = false;
                for (const auto& enemy : enemies) {
                    if (enemy->isAlive() && enemy->getX() == x && enemy->getY() == y) {
                        std::cout << enemy->getSymbol();
                        hadEnemy = true;
                        break;
                    }
                }

                bool hadItem = false;
                for (const auto& item : items) {
                    if (item->getX() == x && item->getY() == y) {
                        std::cout << item->getSymbol();
                        hadItem = true;
                    }
                }
                if (!hadEnemy && !hadItem)
                    std::cout << map.getTile(x, y);
            } else {
                std::cout << ' ';
            }
        }
        // Linha do mapa: largura fixa — novaLinha() garante limpeza do resto
        console_.novaLinha();
    }

    renderHUD(player, messageLog, inventarioAberto, andarAtual);
}

void Renderer::renderHUD(const Player& player, const std::deque<std::string>& messageLog, bool inventarioAberto, int andarAtual) {
    if (inventarioAberto) {
        renderInventario(player.getInventario());
        return;
    }

    // Todas as linhas do HUD usam novaLinha() para sobrescrever artefatos de frames anteriores
    std::cout << std::string(40, '-');
    console_.novaLinha();

    std::cout << " Level: " << player.getLevel();
    std::cout << " XP: " << player.getXP() << "/" << player.getXPProxLevel();
    std::cout << " Andar: " << andarAtual;
    console_.novaLinha();

    std::cout << " HP: " << player.getHp() << "/" << player.getMaxHp();
    std::cout << "   ATK: " << player.getAttack();
    std::cout << "   DEF: " << player.getDefense();
    console_.novaLinha();

    auto efeitos = player.getEfeitosNomes();
    std::cout << " Efeitos:";
    if (!efeitos.empty()) {
        for (const auto& nome : efeitos) {
            std::cout << ' ';
            console_.setColor(corDoEfeito(nome));
            std::cout << '[' << nome << ']';
            console_.resetColor();
        }
    }
    console_.novaLinha();

    std::cout << std::string(40, '-');
    console_.novaLinha();
    std::cout << " [W/A/S/D] Mover   [ESC] Sair";
    console_.novaLinha();

    for (const auto& message : messageLog) {
        std::cout << message;
        console_.novaLinha();
    }

    // Linhas em branco para sobrescrever resíduos do inventário (que tem ~16 linhas vs ~9 do HUD)
    for (int i = 0; i < 8; ++i) console_.novaLinha();
}

void Renderer::renderInventario(const Inventario& inv) {
    std::cout << std::string(40, '-');
    console_.novaLinha();
    std::cout << "=== INVENTARIO ===";
    console_.novaLinha();
    std::cout << std::string(40, '-');
    console_.novaLinha();
    std::cout << " Equipados:";
    console_.novaLinha();

    auto printSlot = [&](const char* label, ItemSlot slot) {
        Item* item = inv.getEquipado(slot);
        std::cout << "   " << label << (item ? item->getNome() : "--");
        console_.novaLinha();
    };
    printSlot("Arma: ",     ItemSlot::Arma);
    printSlot("Armadura: ", ItemSlot::Armadura);
    printSlot("Acessorio: ",ItemSlot::Acessorio);

    std::cout << " Consumiveis:";
    console_.novaLinha();
    const auto& cons = inv.getConsumiveis();
    for (int i = 0; i < 5; i++) {
        std::cout << " [" << (i + 1) << "] ";
        if (i < static_cast<int>(cons.size()))
            std::cout << cons[i]->getNome();
        else
            std::cout << "--";
        console_.novaLinha();
    }

    std::cout << std::string(40, '-');
    console_.novaLinha();
    std::cout << " [1-5] Usar  [E] Equipar  [X] Desequipar  [I] Fechar";
    console_.novaLinha();
    std::cout << std::string(40, '-');
    console_.novaLinha();
}

void Renderer::setCorEfeito(const std::vector<std::string>& nomes) {
    bool temParalisia = false;
    bool temVeneno    = false;
    for (const auto& nome : nomes) {
        if (nome == "Paralisia") temParalisia = true;
        if (nome == "Veneno")    temVeneno    = true;
    }
    if      (temParalisia) console_.setColor(Cor::Amarelo);
    else if (temVeneno)    console_.setColor(Cor::Verde);
}

void Renderer::renderMenu(bool temSave) {
    console_.limparTotal();
    console_.setColor(Cor::Magenta);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║          ROGUELIKE  v0.9             ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  [N] Nova Partida                    ║\n";
    if (temSave) {
        std::cout << "║  [C] Carregar Jogo                   ║\n";
    } else {
        std::cout << "║  [C] Carregar Jogo  (sem save)       ║\n";
    }
    std::cout << "║  [ESC] Sair                          ║\n";
    console_.setColor(Cor::Magenta);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}

void Renderer::renderTelaDerrota(int andar, int nivel, int xp, int inimigos) {
    console_.limparTotal();
    console_.setColor(Cor::Vermelho);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║           VOCE MORREU                ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  Andar alcancado:   " << andar    << "\n";
    std::cout << "║  Nivel final:       " << nivel    << "\n";
    std::cout << "║  XP acumulado:      " << xp       << "\n";
    std::cout << "║  Inimigos mortos:   " << inimigos << "\n";
    console_.setColor(Cor::Vermelho);
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "║  [qualquer tecla] -> Menu            ║\n";
    console_.setColor(Cor::Vermelho);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}

void Renderer::renderHistorico(const std::vector<std::string>& historico) {
    console_.limparTotal();
    console_.setColor(Cor::AzulClaro);
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║       HISTORICO DE MENSAGENS         ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();

    const int MAX_LINHAS = 20;
    int inicio = static_cast<int>(historico.size()) - MAX_LINHAS;
    if (inicio < 0) inicio = 0;
    for (int i = inicio; i < static_cast<int>(historico.size()); ++i) {
        std::cout << "  > " << historico[i] << '\n';
    }

    console_.setColor(Cor::AzulClaro);
    std::cout << "╠══════════════════════════════════════╣\n";
    console_.resetColor();
    std::cout << "  [H / ESC] Fechar\n";
    console_.setColor(Cor::AzulClaro);
    std::cout << "╚══════════════════════════════════════╝\n";
    console_.resetColor();
}
