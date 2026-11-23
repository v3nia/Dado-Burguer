module;

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <poll.h>
#endif

#include <memory>
#include <utility>
#include <string>
#include <random>
#include <timer.hpp>
export module gameplay;
// imports:
import TAD_minheap;
import TAD_cliente;
import TAD_montagem;
import TAD_receita;
import TAD_avl;
import TAD_hash_cache;
import metricas;

using std::string;

export static std::mt19937 gen(std::random_device{}());

export struct Estoque {
    minHeap carne;   
    minHeap ovo;     
    minHeap bacon;   

    int queijoPrato = 0;
    int queijoCheddar = 0;
    int cebola = 0;
    int tomate = 0;
};

export struct EstadoJogo {
    Estoque estoque;
    FilaClientes fila;
    Pilha montagem;
    HashCache_Pedidos cache;

    int turnoAtual = 1;
    int pontuacao = 0;
    int clientesAtendidos = 0;
    bool rodando = true;
    string feedback = "Bem-vindo ao Dado-Burguer!";
};

/* FUNÇÕES NÃO EXPORTADAS (auxiliares) */

Receita sortearReceita(Avl_pedidos &avl, HashCache_Pedidos &cache, Metricas &metricas) {
    std::uniform_int_distribution<> dis(avl.menor_codigo, avl.maior_codigo);
    int sorteado = dis(gen);
    string stringret = buscar_hash(&cache, sorteado);

    if(stringret.empty()) return construirReceita(stringret);

    else{
        inserir_hash(&cache, sorteado, stringret);
        Timer t;
        timer_iniciar(&t);
        stringret = buscar_pedido(avl.raiz, sorteado, avl.total_comparacao);
        metricas.avlTempoBuscas += timer_parar(&t);
        return construirReceita(stringret);
    }
}

/* RENDERIZAÇÃO EM LOOP NO TERMINAL: */

void ativarTerminal(){
    #ifdef _WIN32{
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    #else{
        static termios orig;
    }
}

void desativarTerminal(){
    #ifdef _WIN32{
        sexosexo
    }
    #else{
        pornoporno
    }
    
}

char lerTecla(int ms){
    #ifdef _WIN32{
        if(_kbhit()){
            return static_cast<char>(_getch());
        }
        else{
            Sleep(ms);
            return '\0';
        }
    }
    #else{
        mds
    }
}

