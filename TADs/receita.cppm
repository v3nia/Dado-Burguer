module;
#include<memory>

export module TAD_receita;

using std::unique_ptr;
using std::make_unique;
using std::move;

export{
    typedef struct Nodo {
        unique_ptr<Nodo> prox;
        char Ingrediente;
    } Nodo;

    typedef struct Receita{
        unique_ptr<Nodo> Ini;
    } Receita;

    Receita inicializar(){
        Receita receita;
        receita->Ini = make_unique<Nodo>();
        return receita;
    }

    /* FUNÇÕES */

    //void imprimir(const Receita &receita){};
    
    void inserirIninio(Receita &receita, Nodo &novo){
        novo->prox = move(receita->Ini);
        receita->Ini = move(novo);

        return;
    };

    void inserirFim(Receita &receita, Nodo &novo){
        Nodo* aux = receita->Ini.get();
        if(aux == nullptr) return;

        while(aux->prox != nullptr){
            aux = aux->prox.get();
        }

        aux->prox = move(novo);

        return;
    };
    
    int tamanho(const Receita &receita){
        Nodo* aux = receita->Ini.get();
        if(aux == nullptr) return;
        int contador = 0;

        while(aux != nullptr){
            contador++;
            aux = aux->prox.get();
        }

        return contador;
    };

    void removerFim(Receita &receita){
        Nodo* aux = receita->Ini.get();
        if(aux == nullptr || aux->prox == nullptr) return;

        while(aux->prox->prox != nullptr){
            aux = aux->prox.get();
        }

        aux->prox = move(aux->prox->prox);

        return;
    }
}