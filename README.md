
# Dado-Burguer

Jogo de hamburgueria em turnos feito para rodar no terminal, desenvolvido em C++20 com módulos para a disciplina de Estruturas de Dados

## Sobre o jogo

Você trabalha no balcão do Dado-Burguer e precisa montar os pedidos de uma fila de clientes (entre 10 e 20 por partida) antes que a paciência deles acabe.

Os lanches são montados de baixo para cima a partir do pão base. Cada ação (preparar ingrediente, empilhar, desfazer ou esperar na chapa) consome 1 turno. Carnes, ovos e bacons levam turnos para assar na chapa e vencem se ficarem tempo demais parados. Se a paciência do cliente atual zerar, você perde; se atender todo mundo, vence. A pontuação final é o número de clientes atendidos.

## Especificações

- **Linguagem:** C++20 com módulos (`.cppm`) e build via CMake.
- **Estruturas próprias (sem containers da STL):** Árvore AVL (pedidos), Tabela Hash com encadeamento separado (cache), Lista Contígua + Merge Sort (histórico), Lista Encadeada (receita), Pilha (montagem), Fila (clientes) e Heap Mínimo (validade dos perecíveis).
- **Interface:** TUI no terminal com sprites animados em `assets/` e cores ANSI.
- **Métricas:** salvas automaticamente em `medicoes.log` ao carregar os arquivos, encerrar partidas e ordenar o histórico.

## Requisitos

Como o projeto usa módulos do C++20, é preciso ter ferramentas atualizadas:

- CMake 3.28 ou superior
- Compilador com suporte a módulos: Clang 16+ (com `clang-scan-deps`), GCC 14+ ou MSVC (Visual Studio 2022)
- Ninja (no Linux e macOS)
- Terminal com suporte a UTF-8 e cores ANSI (maximize a janela antes de rodar para o desenho não quebrar linha)

## Como compilar e rodar

O executável recebe dois argumentos obrigatórios: o arquivo de pedidos e o arquivo de histórico. Execute sempre a partir da pasta raiz do projeto para que ele encontre a pasta `assets/`.

### Linux (Ubuntu / Debian / Pop!_OS)

1. Instale as dependências (apenas na primeira vez):

```bash
sudo apt update
sudo apt install cmake ninja-build clang clang-tools
```

*(Se preferir GCC, instale o `g++-14` e troque `clang++` por `g++-14` no passo seguinte.)*

2. Configure e compile:

```bash
cmake -G Ninja -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

3. Rode o jogo:

```bash
./build/dado_burguer pedidos.txt historico.txt
```

### Windows (10 / 11)

1. Instale o **Visual Studio 2022** com o pacote "Desenvolvimento para desktop com C++" (já inclui MSVC e CMake).
2. Abra o **Developer PowerShell for VS 2022** na pasta do projeto e compile:

```powershell
cmake -B build
cmake --build build --config Release
```

3. Rode (de preferência no Windows Terminal):

```powershell
.\build\Release\dado_burguer.exe pedidos.txt historico.txt
```

### macOS

1. Instale as dependências pelo Homebrew (o Clang padrão da Apple não inclui o scanner de módulos):

```bash
brew install cmake ninja llvm
```

2. Compile usando o Clang do Homebrew:

```bash
cmake -G Ninja -B build -DCMAKE_CXX_COMPILER=$(brew --prefix llvm)/bin/clang++
cmake --build build
```

3. Rode o jogo:

```bash
./build/dado_burguer pedidos.txt historico.txt
```

## Controles

- `1`, `2`, `3`: colocar Carne, Ovo ou Bacon na chapa (assam e vencem com os turnos)
- `4`, `5`, `6`, `7`: preparar Queijo suíço, Cheddar, Cebola ou Tomate
- **Empilhar:** `n` (carne), `o` (ovo), `b` (bacon), `s` (suíço), `q` (cheddar), `c` (cebola), `t` (tomate), `k` (ketchup), `m` (mostarda), `p` (picles), `a` (alface)
- `z`: colocar o pão de cima (fecha e entrega o pedido)
- `Espaço`: esperar 1 turno
- `Backspace`: remover o ingrediente do topo
- `0` ou `ESC`: sair da partida
