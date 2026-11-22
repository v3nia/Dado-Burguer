module;

#include <memory>
#include <string>

export module TAD_lista_contigua;

using std::string;
using std::unique_ptr;
using std::make_unique;

export{
    struct NodoListaContigua {
        string nome;
        int pontuacao;
    };

    struct ListaContigua_Historico {
        int tamanho;
        int capacidade;
        unique_ptr<NodoListaContigua[]> elementos;
    };

    //PROTÓTIPOS DAS FUNÇÕES INTERNAS
    void redimensionar(ListaContigua_Historico& lista);

    void inicializar_listacontigua(ListaContigua_Historico& lista) {
        lista.tamanho = 0;
        lista.capacidade = 1000; 
        lista.elementos = make_unique<NodoListaContigua[]>(lista.capacidade);
    }

    bool inserir_listacontigua(ListaContigua_Historico& lista, NodoListaContigua novo, int posicao) {
        
        if (posicao < 0 || posicao > lista.tamanho) { //posição inválida
            return false;  
        }

        if (lista.tamanho >= lista.capacidade) { //verifico se ainda cabe, se não, redimensiono
            redimensionar(lista);
        }

        for (int i = lista.tamanho; i > posicao; i--) {
            lista.elementos[i] = std::move(lista.elementos[i - 1]);
        }

        lista.elementos[posicao] = novo;
        lista.tamanho++;
          
        return true;
    }

    NodoListaContigua* acessar_listacontigua(ListaContigua_Historico& lista, int posicao) {
        NodoListaContigua* resultado;

        if (posicao >= lista.tamanho || posicao < 0) {
            return nullptr;
        }

        resultado = &lista.elementos[posicao];

        return resultado;
    }

    int tamanho_listacontigua(const ListaContigua_Historico& l) {
        return l.tamanho;
    }

    ListaContigua_Historico copiar_listacontigua(const ListaContigua_Historico& lista) {
        ListaContigua_Historico copia;
        copia.tamanho = lista.tamanho;
        copia.capacidade = lista.capacidade;
        //preciso alocar a memoria para a cópia, se não, dará erro e não vai ter como armazenar
        copia.elementos = make_unique<NodoListaContigua[]>(copia.capacidade);
        for (int i = 0; i < lista.tamanho; ++i) {
            copia.elementos[i] = lista.elementos[i];
        }
        return copia;
    }
}

void redimensionar(ListaContigua_Historico& lista) {
    int nova_capacidade = lista.capacidade * 2;
        
    //aloco e movo os elementos antigos para a nova lista
    auto novos_elementos = make_unique<NodoListaContigua[]>(nova_capacidade);
    for (int i = 0; i < lista.tamanho; ++i) {
        novos_elementos[i] = std::move(lista.elementos[i]);
    }

    lista.elementos = std::move(novos_elementos);
    lista.capacidade = nova_capacidade;
}