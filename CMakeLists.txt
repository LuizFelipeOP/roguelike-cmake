# cmake_minimum_required: versão mínima do CMake necessária para rodar este arquivo
# Necessário para CMake 4.x: deve vir ANTES de cmake_minimum_required para que
# dependências externas (doctest, nlohmann_json) com versões antigas não causem erro
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

# cmake_minimum_required: versão mínima do CMake necessária para rodar este arquivo
cmake_minimum_required(VERSION 3.16)

# Necessário para CMake 4.x: permite que dependências externas (doctest, nlohmann_json)
# usem cmake_minimum_required com versão antiga sem causar erro de configuração
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# project(): define o nome do projeto e a linguagem usada
project(Roguelike CXX)

# CMAKE_CXX_STANDARD: força o uso do C++17 (necessário para std::optional, structured bindings, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# No Windows com MSVC, ativa unicode e desativa warnings de funções "unsafe" da Microsoft
if(MSVC)
    # /W4: warnings nível 4  /utf-8: encoding  /EHsc: habilita exceções C++ (necessário para doctest e std::)
    add_compile_options(/W4 /utf-8 /EHsc)
endif()

# ─────────────────────────────────────────────
# Dependências externas via FetchContent
# Devem ser declaradas ANTES de add_executable para que os targets existam
# quando target_link_libraries for chamado
# ─────────────────────────────────────────────

include(FetchContent)

# doctest v2.4.11 — framework de testes header-only
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(doctest)

# nlohmann/json v3.11.3 — biblioteca JSON header-only para save/load
# GIT_SHALLOW TRUE: clona apenas a tag, sem histórico completo (mais rápido)
# FetchContent_MakeAvailable processa o CMakeLists da lib e cria o target
# nlohmann_json::nlohmann_json, que expõe os headers automaticamente
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-2.30.3
)
FetchContent_Declare(
  SDL2_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG        release-2.8.2
)
FetchContent_Declare(
  SDL2_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG        release-2.22.0
)
FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_ttf)

# ─────────────────────────────────────────────
# Executável principal
# ─────────────────────────────────────────────

# file(GLOB_RECURSE ...): coleta automaticamente todos os .cpp dentro de src/
# Isso significa que qualquer .cpp novo que você criar em src/ será incluído no build
file(GLOB_RECURSE SOURCES "src/*.cpp")

# add_executable(): cria o executável "roguelike" a partir de todos os .cpp coletados
add_executable(roguelike ${SOURCES})

# target_include_directories(): diz ao compilador onde estão os headers (.hpp)
# Assim você pode fazer #include "core/Game.hpp" de qualquer lugar dentro de src/
target_include_directories(roguelike PRIVATE src)

# Linka nlohmann_json ao executável principal — isso expõe os headers da lib
# Após esse target_link_libraries, você pode usar #include <nlohmann/json.hpp> em qualquer .cpp
target_link_libraries(roguelike PRIVATE
    nlohmann_json::nlohmann_json
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)

# Macro que habilita o código SDL dentro de Game.cpp (não definido nos testes)
target_compile_definitions(roguelike PRIVATE ROGUELIKE_SDL)

# Copia DLLs do SDL2 para junto do executável após cada build
add_custom_command(TARGET roguelike POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE:SDL2_image::SDL2_image>
        $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
        $<TARGET_FILE_DIR:roguelike>
)

# Copia a pasta assets/ (fontes) para junto do executável após cada build
add_custom_command(TARGET roguelike POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:roguelike>/assets
)

# Copia os spritesheets para build/Debug/assets/sprites/
add_custom_command(TARGET roguelike POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:roguelike>/assets/sprites
)
foreach(SHEET Monsters Items Terrain Terrain_Objects Interface)
    add_custom_command(TARGET roguelike POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/src/sprites/${SHEET}.png
            $<TARGET_FILE_DIR:roguelike>/assets/sprites/${SHEET}.png
    )
endforeach()
# Runtime estático: embute as DLLs do MSVC no .exe
# /MT em Release, /MTd em Debug — o .exe Release roda sem Visual Studio instalado
if(MSVC)
    set_property(TARGET roguelike PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ─────────────────────────────────────────────
# Testes
# ─────────────────────────────────────────────

enable_testing()

# Sources de produção reutilizadas nos testes — exclui:
#   main.cpp        — conflito de ponto de entrada com o main do doctest
#   SDLRenderer.cpp — depende de SDL2 (não linkado nos testes)
#   TileAtlas.cpp   — depende de SDL2 (não linkado nos testes)
#   SalaThema.cpp   — depende de SDL2 (não linkado nos testes)
file(GLOB_RECURSE TEST_GAME_SOURCES "src/*.cpp")
list(FILTER TEST_GAME_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER TEST_GAME_SOURCES EXCLUDE REGEX ".*SDLRenderer\\.cpp$")
list(FILTER TEST_GAME_SOURCES EXCLUDE REGEX ".*TileAtlas\\.cpp$")
list(FILTER TEST_GAME_SOURCES EXCLUDE REGEX ".*SalaThema\\.cpp$")

# Arquivos de teste em tests/
file(GLOB TEST_SOURCES "tests/*.cpp")

add_executable(roguelike_tests ${TEST_SOURCES} ${TEST_GAME_SOURCES})

target_include_directories(roguelike_tests PRIVATE src)
target_link_libraries(roguelike_tests PRIVATE doctest::doctest nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(roguelike_tests PRIVATE /W4 /utf-8)
endif()

# Registra cada TEST_CASE do doctest como um teste CTest individual
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(roguelike_tests)
