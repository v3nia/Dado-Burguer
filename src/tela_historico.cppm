module;
#include <iostream>
#include <string>
#include <cstdlib>
#include "timer.hpp"

export module tela_historico;

//uso de IA (Claude) para gerar a funções da tela de histórico - só conferi e mudei os parametros dos nomes 'padrões' para os nossos

import TAD_lista_contigua;
import ordenacao_merge;     
import metricas;    // ajuste: struct Metricas e gravarLog()

// Lê uma opção inteira; devolve -1 se a entrada for inválida.
static int lerOpcao() {
    std::string linha;
    if (!std::getline(std::cin, linha)) return 0;   // EOF: sai da tela
    char* fim = nullptr;
    long v = std::strtol(linha.c_str(), &fim, 10);
    if (linha.empty() || *fim != '\0') return -1;
    return static_cast<int>(v);
}

// Cronometra SOMENTE a ordenação; a cópia fica fora do intervalo.
static double ordenarEMedir(const ListaContigua_Historico& original, CriterioOrdenacao criterio, ListaContigua_Historico& copia) {
    // SEU CÓDIGO: preencher 'copia' como cópia independente de 'original'
    // (o 'elementos' é unique_ptr, então a cópia precisa ser feita à mão)

    copia = copiar_listacontigua(original);

    //timer só inicia depois da lista ja ser copiada - só conta o tempo da ordenação
    Timer t;
    timer_iniciar(&t);

    merge_sort(copia, criterio);

    return timer_parar(&t);
}

export void telaHistorico(const ListaContigua_Historico& hist, Metricas& m) {
    int opcao;
    do {
        std::cout << "\n=== HISTORICO ===\n"
                  << "Entradas: " << hist.tamanho << "\n"
                  << "1) Ordenar por nome (A-Z)\n"
                  << "2) Ordenar por pontuacao (decrescente)\n"
                  << "0) Voltar ao menu\n"
                  << "Escolha: ";
        opcao = lerOpcao();

if (opcao == 1 || opcao == 2) {
    CriterioOrdenacao criterio;
    if (opcao == 1) {
        criterio = CriterioOrdenacao::POR_NOME;
    } else {
        criterio = CriterioOrdenacao::POR_PONTOS;
    }

    ListaContigua_Historico copia;
    double tempo = ordenarEMedir(hist, criterio, copia);

    // métricas: a medição é repetida a cada acesso
    m.ord_tamanho = hist.tamanho;
    m.ord_acessos++;
    if (opcao == 1) {
        m.ord_tempo_nome = tempo;
    } else {
        m.ord_tempo_pontos = tempo;
    }

    // exibição (fora do cronômetro): só as primeiras N entradas
    const int N = 20;
    for (int i = 0; i < copia.tamanho && i < N; ++i) {
        std::cout << (i + 1) << ". " << copia.elementos[i].nome
                  << " - " << copia.elementos[i].pontuacao << "\n";
    }
    if (copia.tamanho > N) {
        std::cout << "... (" << (copia.tamanho - N) << " entradas omitidas)\n";
    }
    std::cout << "Tempo de ordenacao (Merge Sort): " << tempo << " s\n";

    gravar_log(m);   // regrava o medicoes.log com o bloco atualizado
} else if (opcao != 0) {
    std::cout << "Opcao invalida.\n";
}
    } while (opcao != 0);
}