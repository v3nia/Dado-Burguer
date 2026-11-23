cmake_minimum_required(VERSION 3.28)

project(DadoBurguer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gera build/compile_commands.json para o clangd reconhecer os módulos
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dado_burguer
    src/main.cpp
    src/timer.cpp
)

# Garante que #include "timer.hpp" funcione em qualquer arquivo do projeto
target_include_directories(dado_burguer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Registra TODOS os módulos C++20 para o CMake resolver a ordem de compilação
target_sources(dado_burguer
    PRIVATE
        FILE_SET CXX_MODULES FILES
            # TADs
            TADs/avl.cppm
            TADs/fila_clientes.cppm
            TADs/hash_cache.cppm
            TADs/lista_contigua.cppm
            TADs/montagem_burguer.cppm
            TADs/pereciveis_minheap.cppm
            TADs/receita.cppm

            # Sistema, interface e gameplay
            src/arquivos.cppm
            src/coleta_metricas.cppm
            src/gameplay.cppm
            src/metricas.cppm
            src/ordenacao.cppm
            src/renderizacao.cppm
            src/tela_historico.cppm
)

# Flags por compilador e garantia de UTF-8 para a TUI no Windows
if(MSVC)
    target_compile_options(dado_burguer PRIVATE /W4 /utf-8)
else()
    target_compile_options(dado_burguer PRIVATE -Wall -Wextra -Wpedantic)
endif()