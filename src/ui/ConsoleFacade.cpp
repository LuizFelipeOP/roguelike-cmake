#include "ui/ConsoleFacade.hpp"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

void ConsoleFacade::setColor(Cor cor){
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (cor){
        case Cor::Amarelo:   SetConsoleTextAttribute(h, 14); break;
        case Cor::Verde:     SetConsoleTextAttribute(h, 10); break;
        case Cor::Ciano:     SetConsoleTextAttribute(h, 11); break;
        case Cor::Vermelho:  SetConsoleTextAttribute(h, 12); break;
        case Cor::Magenta:   SetConsoleTextAttribute(h, 13); break;
        case Cor::AzulClaro: SetConsoleTextAttribute(h, 9);  break;
        default:             SetConsoleTextAttribute(h, 7);  break;
    }
#else
    switch (cor) {
        case Cor::Amarelo:   std::cout << "\033[33m"; break;
        case Cor::Verde:     std::cout << "\033[32m"; break;
        case Cor::Ciano:     std::cout << "\033[36m"; break;
        case Cor::Vermelho:  std::cout << "\033[31m"; break;
        case Cor::Magenta:   std::cout << "\033[35m"; break;
        case Cor::AzulClaro: std::cout << "\033[34m"; break;
        default:             std::cout << "\033[0m";  break;
    }
#endif
}

void ConsoleFacade::resetColor() {
    setColor(Cor::Padrao);
}

// Move cursor para (0,0) sem apagar — usado no loop do jogo.
// O frame seguinte sobrescreve o conteúdo existente no lugar,
// eliminando o flash de tela em branco.
void ConsoleFacade::limpar() {
#ifdef _WIN32
    COORD origem = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), origem);
#else
    std::cout << "\033[H";
#endif
}

// Apaga a tela inteira — usado apenas em transições de estado
// (menu → jogo, jogo → morte). O flicker em eventos únicos é imperceptível.
void ConsoleFacade::limparTotal() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD tamanho = csbi.dwSize.X * csbi.dwSize.Y;
    COORD origem = {0, 0};
    DWORD escrito;
    FillConsoleOutputCharacter(hConsole, ' ', tamanho, origem, &escrito);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, tamanho, origem, &escrito);
    SetConsoleCursorPosition(hConsole, origem);
#else
    std::cout << "\033[2J\033[H";
#endif
}

void ConsoleFacade::ocultarCursor() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = {1, FALSE};
    SetConsoleCursorInfo(h, &info);
#else
    std::cout << "\033[?25l";
#endif
}

// Preenche do cursor até o fim da linha com espaços, depois quebra.
// Garante que conteúdo antigo mais longo não deixe artefatos visíveis.
void ConsoleFacade::novaLinha() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    int restante = csbi.dwSize.X - csbi.dwCursorPosition.X;
    if (restante > 0) {
        DWORD escrito;
        FillConsoleOutputCharacter(h, ' ', restante, csbi.dwCursorPosition, &escrito);
    }
    std::cout << '\n';
#else
    std::cout << "\033[K\n"; // ANSI: apaga até fim da linha, depois quebra
#endif
}

int ConsoleFacade::getLargura() const {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwSize.X;
#else
    return 80; // fallback razoável para terminais POSIX sem consulta
#endif
}
