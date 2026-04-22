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

# ─────────────────────────────────────────────
# Testes — doctest via FetchContent
# ─────────────────────────────────────────────

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(doctest)

enable_testing()

# Sources de produção reutilizadas nos testes — exclui main.cpp para evitar
# conflito de ponto de entrada com o main gerado pelo doctest
file(GLOB_RECURSE TEST_GAME_SOURCES "src/*.cpp")
list(FILTER TEST_GAME_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# Arquivos de teste em tests/
file(GLOB TEST_SOURCES "tests/*.cpp")

add_executable(roguelike_tests ${TEST_SOURCES} ${TEST_GAME_SOURCES})

target_include_directories(roguelike_tests PRIVATE src)
target_link_libraries(roguelike_tests PRIVATE doctest::doctest)

if(MSVC)
    target_compile_options(roguelike_tests PRIVATE /W4 /utf-8)
endif()

# Registra cada TEST_CASE do doctest como um teste CTest individual
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(roguelike_tests)
