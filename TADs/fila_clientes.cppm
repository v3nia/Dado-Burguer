module;
#include <utility>
#include <memory>
export module TAD_cliente;
import TAD_receita;
using std::unique_ptr;
using std::make_unique;

export struct Cliente {
    Receita pedido;
    int paciencia;
};

export struct nodoFila {
    Cliente cliente;
    unique_ptr<nodoFila> prox;
};

export struct FilaClientes {
    unique_ptr<nodoFila> inicio;
    nodoFila* fim = nullptr;
    int tamanho = 0;
};

export void enfileirar(FilaClientes& fila, Receita pedido, int paciencia) {
    auto novoNodo = make_unique<nodoFila>(Cliente{std::move(pedido), paciencia}, nullptr);

    if (!fila.inicio) {
        fila.inicio = std::move(novoNodo);
        fila.fim = fila.inicio.get();
    } else {
        fila.fim->prox = std::move(novoNodo);
        fila.fim = fila.fim->prox.get();
    }
    fila.tamanho++;
}

export void desenfileirar(FilaClientes& fila) {
    if (!fila.inicio) {
        return; 
    }

    fila.inicio = std::move(fila.inicio->prox);
    if (!fila.inicio) {
        fila.fim = nullptr; 
    }
    fila.tamanho--;
}

export Cliente* visualizarInicio(const FilaClientes& fila) {
    if (!fila.inicio) {
        return nullptr; 
    }
    return &fila.inicio->cliente;
}

