cmake_minimum_required(VERSION 3.28)

project(DadoBurguer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gera build/compile_commands.json para o clangd reconhecer os módulos
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dado_burguer
    src/main.cpp
)

# Registra os módulos C++20 para o CMake resolver a ordem de compilação sozinho
target_sources(dado_burguer
    PRIVATE
        FILE_SET CXX_MODULES FILES
            src/TADs/fila_clientes.cppm
            src/TADs/montagem_burguer.cppm
            src/TADs/receita.cppm
)

# Flags por compilador e garantia de UTF-8 para a TUI no Windows
if(MSVC)
    target_compile_options(dado_burguer PRIVATE /W4 /utf-8)
else()
    target_compile_options(dado_burguer PRIVATE -Wall -Wextra -Wpedantic)
endif()