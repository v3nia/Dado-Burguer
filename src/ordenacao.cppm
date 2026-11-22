module;
#include <memory>

export module ordenacao_merge;

import TAD_lista_contigua;

export{
    enum class CriterioOrdenacao {
        POR_PONTOS,
        POR_NOME
    };

    //criterio me diz se a ordenação é por pontuação ou por nome
    void merge(ListaContigua_Historico& lista, ListaContigua_Historico& aux, int inicio, int meio, int fim, CriterioOrdenacao criterio) {
        for (int t = inicio; t <= fim; ++t) {
            aux.elementos[t] = lista.elementos[t];
        }

        int i = inicio;
        int j = meio + 1;
        int k = inicio;

        if (criterio == CriterioOrdenacao::POR_PONTOS) { //ordenar por pontuação
            while (i <= meio && j <= fim) {
                //se as pontuacoes forem iguais, o que chegou na pontuacao primeiro, vai primeiro
                if (aux.elementos[i].pontuacao <= aux.elementos[j].pontuacao) {
                    lista.elementos[k++] = aux.elementos[i++];
                } else {
                    lista.elementos[k++] = aux.elementos[j++];
                }
            }
        } else { //ordenar por nome
            while (i <= meio && j <= fim) {
                if (aux.elementos[i].nome <= aux.elementos[j].nome) {
                    lista.elementos[k++] = aux.elementos[i++];
                } else {
                    lista.elementos[k++] = aux.elementos[j++];
                }
            }
        }
        
        while (i <= meio) {
            lista.elementos[k++] = aux.elementos[i++];
        }
        while (j <= fim) {
            lista.elementos[k++] = aux.elementos[j++];
        }
    }

    void merge_rec(ListaContigua_Historico& lista, ListaContigua_Historico& aux, int inicio, int fim, CriterioOrdenacao criterio) {
        if(inicio >= fim) {
            return;
        }
        int meio = inicio + (fim - inicio) / 2;
        merge_rec(lista, aux, inicio, meio, criterio);
        merge_rec(lista, aux, meio + 1, fim, criterio);
        merge(lista, aux, inicio, meio, fim, criterio);
    }

    void merge_sort(ListaContigua_Historico& lista, CriterioOrdenacao criterio) {
        if (lista.tamanho <= 1) {
            return;
        }

        ListaContigua_Historico aux;
        aux.tamanho = lista.tamanho;
        aux.capacidade = lista.capacidade;
        aux.elementos = std::make_unique<NodoListaContigua[]>(aux.capacidade);

        merge_rec(lista, aux, 0, lista.tamanho - 1, criterio);
    }
}