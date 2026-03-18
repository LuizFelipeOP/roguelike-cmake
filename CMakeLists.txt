# cmake_minimum_required: versão mínima do CMake necessária para rodar este arquivo
cmake_minimum_required(VERSION 3.16)

# project(): define o nome do projeto e a linguagem usada
project(Roguelike CXX)

# CMAKE_CXX_STANDARD: força o uso do C++17 (necessário para std::optional, structured bindings, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# No Windows com MSVC, ativa unicode e desativa warnings de funções "unsafe" da Microsoft
if(MSVC)
    add_compile_options(/W4 /utf-8)
endif()

# file(GLOB_RECURSE ...): coleta automaticamente todos os .cpp dentro de src/
# Isso significa que qualquer .cpp novo que você criar em src/ será incluído no build
file(GLOB_RECURSE SOURCES "src/*.cpp")

# add_executable(): cria o executável "roguelike" a partir de todos os .cpp coletados
add_executable(roguelike ${SOURCES})

# target_include_directories(): diz ao compilador onde estão os headers (.hpp)
# Assim você pode fazer #include "core/Game.hpp" de qualquer lugar dentro de src/
target_include_directories(roguelike PRIVATE src)
