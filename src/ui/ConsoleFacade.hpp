#pragma once
#include <string>

enum class Cor {
    Padrao,         // branco — reset
    Amarelo,        // Paralisia
    Verde,          // Veneno
    Ciano,          // Regeneracao
    Vermelho,       // Enfraquecimento
    Magenta,        // destaque de menu
    AzulClaro,      // HUD / info
};

class ConsoleFacade {
public:
    void setColor(Cor cor);     // aplica cor no terminal
    void resetColor();          // volta para Cor::Padrao
    void limpar();              // move cursor para (0,0) — sem apagar, para render do jogo
    void limparTotal();         // apaga tela inteira — para transições de estado (menu, morte)
    void ocultarCursor();       // remove o cursor piscante
    void novaLinha();           // preenche resto da linha com espaços e quebra — elimina artefatos
    int  getLargura() const;    // retorna largura atual do console em colunas
};