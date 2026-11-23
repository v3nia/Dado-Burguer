module;
#include <memory>
#include <string>
export module TAD_montagem;
import TAD_receita;
using std::unique_ptr; 
using std::make_unique;
using std::string;

struct nodoPilha{
    char ingrediente;
    unique_ptr<nodoPilha> prox;
};

export struct Pilha{
    unique_ptr<nodoPilha> topo;
};

export void iniciarMontagem(Pilha &p){
    p.topo = make_unique<nodoPilha>('i', nullptr);
};

export void empilhar(Pilha &p, char ingrediente){
    if (p.topo->ingrediente == 'z') return;

    auto novo = make_unique<nodoPilha>(ingrediente, std::move(p.topo));
    p.topo = std::move(novo);
};

export void desempilhar(Pilha &p){
    if (p.topo->ingrediente == 'i') return;

    p.topo = std::move(p.topo->prox);
};

export string stringPilha(const Pilha &p){
    string resultado;
    auto aux = p.topo.get();

    while(aux->prox != nullptr && aux->ingrediente != 'i'){
        resultado += aux->ingrediente;
        aux = aux->prox.get();
    }

    return resultado;
};

export bool comparar(const Receita& r, const Pilha& p){
    return (stringPilha(p) == stringReceita(r));
};
