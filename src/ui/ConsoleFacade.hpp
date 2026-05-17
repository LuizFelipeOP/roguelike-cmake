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
    void setColor(Cor cor);   // aplica cor no terminal
    void resetColor();        // volta para Cor::Padrao
    void limpar();            // limpa a tela
    void ocultarCursor();     // remove o cursor piscante
};