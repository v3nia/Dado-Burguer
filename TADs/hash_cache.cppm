module;
#include <memory>
#include <string>

export module TAD_hash_cache;

using std::unique_ptr;
using std::make_unique;
using std::string;

const int M = 32; //tamanho da tabela hash

export{
    struct NodoHashCache {
        int cod_pedido;
        string receita;
        unique_ptr<NodoHashCache> prox;
    };

    struct HashCache_Pedidos{
        unique_ptr<NodoHashCache> tabela[M];
        int n = 0; //número de elementos na tabela
        int m = M; //tamanho da tabela

        int hit = 0; //número de acertos na busca
        int miss = 0; //número de falhas na busca

        int colisoes = 0; //número de colisões na inserção
    };

    //PROTÓTIPOS DAS FUNÇÕES INTERNAS
    int hash_multiplicativo(int chave, int m);
    void inserir_hash(HashCache_Pedidos *t, int cod_pedido, const string& receita);

    //retorna a receita se o código do pedido estiver na tabela hash, se não estiver, busca na avl, insere na tabela hash e retorna uma string vazia
    string buscar_hash(HashCache_Pedidos *t, int cod_pedido) {
        int i = hash_multiplicativo(cod_pedido, t->m);

        const NodoHashCache *p = t->tabela[i].get();
        while (p != nullptr) {
            if (p->cod_pedido == cod_pedido) {
                t->hit++;
                return p->receita;
            }
            p = p->prox.get();
        }

        t->miss++;
        return "";
    }

    void inserir_hash(HashCache_Pedidos *t, int cod_novo, const string& receita_nova) {
        int i = hash_multiplicativo(cod_novo, t->m);
        
        //cria um novo nodo e insere no início da lista encadeada na posição i da tabela hash
        auto novo = make_unique<NodoHashCache>();
        novo->cod_pedido = cod_novo;
        novo->receita = receita_nova;

        //verifica se já existe algum elemento na posição i da tabela hash, se sim, incrementa o número de colisões
        if (t->tabela[i] !=nullptr) {
            t->colisoes++;
        }

        novo->prox = std::move(t->tabela[i]); 
        t->tabela[i] = std::move(novo);
        t->n++;
    }

    float taxa_acerto(int hit, int miss) {
        if (hit + miss == 0) return 0.0f; //trata o caso de divisão por zero quando não houve buscas
        float resultado = (float)hit / (hit + miss);
        return resultado; //NA EXIBIÇÃO, MULTIPLICAR POR 100 PARA OBTER A TAXA EM PORCENTAGEM
    }

    void limpar_hash(HashCache_Pedidos *t) {
        if (t == nullptr) {
            return;
        }

        for (int i = 0; i < t->m; i++) {
            t->tabela[i] = nullptr;
        }

        t->n = 0;
        t->hit = 0;
        t->miss = 0;
        t->colisoes = 0;
    }

}

int hash_multiplicativo(int chave, int m) {
    double A  = 0.6180339887;
    double frac = chave * A - (int)(chave * A);
    return (int)(m * frac);
}