module;

#include <memory>
#include <utility>
#include <string>
#include <random>
#include "timer.hpp"
#include <iostream>

export module gameplay;

import TAD_minheap;
import TAD_cliente;
import TAD_montagem;
import TAD_receita;
import TAD_avl;
import TAD_hash_cache;
import metricas;
import renderizacao;

using std::string;
using std::uniform_int_distribution;

static std::mt19937 gen(std::random_device{}());

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

Receita sortearReceita(Avl_Pedidos &avl, HashCache_Pedidos &cache, Metricas &metricas) {
    uniform_int_distribution<> dis(avl.menor_cod, avl.maior_cod);
    int sorteado;
    string stringret;
    Timer t;
    
    do{
        sorteado = dis(gen);
        stringret = buscar_hash(&cache, sorteado);
        if(!stringret.empty()) return construirReceita(stringret);

        timer_iniciar(&t);
        stringret = buscar_pedido(avl.raiz, sorteado, avl.total_comparacao);
        metricas.avlTempoBuscas += timer_parar(&t);
    }while(stringret.empty());

    inserir_hash(&cache, sorteado, stringret);
    return construirReceita(stringret);
}

