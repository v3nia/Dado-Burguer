module;
#include <memory>
#include <string>
export module TAD_receita;
using std::string;
using std::unique_ptr;
using std::make_unique;

struct nodoReceita {
    char Ingrediente;
    unique_ptr<nodoReceita> prox;
};

export struct Receita {
    unique_ptr<nodoReceita> Ini;
};

export void inserirInicio(Receita &receita, char ingrediente) {
    auto novo = make_unique<nodoReceita>(ingrediente, std::move(receita.Ini));
    receita.Ini = std::move(novo);
}

export void inserirFim(Receita &receita, char ingrediente) {
    auto novo = make_unique<nodoReceita>(ingrediente, nullptr);
    nodoReceita* aux = receita.Ini.get();
    if (aux == nullptr) {
        receita.Ini = std::move(novo);
        return;
    }

    while (aux->prox != nullptr) {
        aux = aux->prox.get();
    }

    aux->prox = std::move(novo);
}

export void removerFim(Receita &receita) {
    nodoReceita* aux = receita.Ini.get();

    if (aux == nullptr) return;

    if (aux->prox == nullptr) {
        receita.Ini = nullptr;
        return;
    }

    while (aux->prox->prox != nullptr) {
        aux = aux->prox.get();
    }

    aux->prox = nullptr;
}

/* FUNÇÕES EXPORTADAS */

export Receita construirReceita(const string &resultadoBusca) {
    Receita receita;
    for (char ingrediente : resultadoBusca) {
        inserirFim(receita, ingrediente);
    }
    return receita;
}

export int tamanho(const Receita &receita) {
    const nodoReceita* aux = receita.Ini.get();
    if (aux == nullptr) return 0;
    int contador = 0;

    while (aux != nullptr) {
        contador++;
        aux = aux->prox.get();
    }

    return contador;
}

string stringInversaReceita(const unique_ptr<nodoReceita> &nodo) {
    string resultado = "";
    if(nodo->prox == nullptr) return resultado += nodo->Ingrediente;
    resultado = stringInversaReceita(nodo->prox);

    return resultado += nodo->Ingrediente;
}

export string stringReceita(const Receita &receita) {
    if (receita.Ini == nullptr) return "";
    return stringInversaReceita(receita.Ini);
}