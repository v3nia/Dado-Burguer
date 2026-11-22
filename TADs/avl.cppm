module;
#include <string>
#include <memory>  

export module TAD_avl;

using std::unique_ptr;
using std::make_unique;
using std::string;

export{
    struct NodoAvl {
        int cod_pedido;
        string receita;
        unique_ptr<NodoAvl> esq;
        unique_ptr<NodoAvl> dir;
        int altura;   /* arvoreVazia = 0 e só raiz = 1 */
    };
    
    struct Avl_Pedidos{
        unique_ptr<NodoAvl> raiz;
        int maior_cod = 0;
        int menor_cod = 0;
        int total_insercao = 0; //PASSO COMO PONTEIRO PARA A FUNÇÃO DE INSERÇÃO
        int total_rotacao = 0; //PASSO COMO PONTEIRO PARA AS FUNCÕES DE ROTACAO
        int total_comparacao = 0; //PASSO COMO PONTEIRO PARA A FUNCÃO DE BUSCA
    };

    //PROTÓTIPOS DAS FUNÇÕES INTERNAS
    int avl_altura(const unique_ptr<NodoAvl>& no);
    void avl_atualizar_altura(const unique_ptr<NodoAvl>& no);
    int avl_fb(const unique_ptr<NodoAvl>& raiz);
    unique_ptr<NodoAvl> avl_rotacao_direita(unique_ptr<NodoAvl>& raiz, int &total_rotacao);
    unique_ptr<NodoAvl> avl_rotacao_esquerda(unique_ptr<NodoAvl>& raiz, int &total_rotacao);
    unique_ptr<NodoAvl> avl_reequilibrar(unique_ptr<NodoAvl>& raiz, int &total_rotacao);

    unique_ptr<NodoAvl> inserir_pedido(unique_ptr<NodoAvl> &raiz, const NodoAvl& pedido, int &total_insercao, int &total_rotacao, int &total_comparacao, int& menor_cod, int& maior_cod){
        if (total_insercao == 0) {
            menor_cod = pedido.cod_pedido;
            maior_cod = pedido.cod_pedido;
        }

        if (raiz == nullptr) {
            unique_ptr<NodoAvl> novo = make_unique<NodoAvl>();
            novo->cod_pedido = pedido.cod_pedido;
            novo->receita = pedido.receita;
            novo->esq = nullptr;
            novo->dir = nullptr;
            novo->altura = 1;

            if (pedido.cod_pedido < menor_cod) {
                menor_cod = pedido.cod_pedido;
            }
            if (pedido.cod_pedido > maior_cod) {
                maior_cod = pedido.cod_pedido;
            }

            total_insercao++;
            return novo;
        }

        if (pedido.cod_pedido < raiz->cod_pedido)
            raiz->esq = inserir_pedido(raiz->esq, pedido, total_insercao, total_rotacao, total_comparacao, menor_cod, maior_cod);
            
        else if (pedido.cod_pedido > raiz->cod_pedido)
            raiz->dir = inserir_pedido(raiz->dir, pedido, total_insercao, total_rotacao, total_comparacao, menor_cod, maior_cod);
        else
            return std::move(raiz); //por que não pode ser só return raiz;?

        
        return avl_reequilibrar(raiz, total_rotacao);
    }

    string buscar_pedido(const unique_ptr<NodoAvl>& raiz, int cod_busca, int &total_comparacao){
        if (raiz == nullptr) return "";
        if (cod_busca == raiz->cod_pedido){
            total_comparacao++;
            return raiz->receita;
        }
        if (cod_busca < raiz->cod_pedido){
            total_comparacao++;
            return buscar_pedido(raiz->esq, cod_busca, total_comparacao);
        } else {
            total_comparacao++;
            return buscar_pedido(raiz->dir, cod_busca, total_comparacao);
        }
    }

}

int avl_altura(const unique_ptr<NodoAvl>& no){
    if (no == nullptr) return 0;
    return no->altura;
}

void avl_atualizar_altura(const unique_ptr<NodoAvl>& no) {
    if (no == nullptr) return;
    int ae = avl_altura(no->esq);
    int ad = avl_altura(no->dir);
    if (ae > ad){
        no->altura = 1 + ae;
    } else{
        no->altura = 1 + ad;
    }
}

int avl_fb(const unique_ptr<NodoAvl>& raiz) {
    if (raiz == nullptr) return 0;
    return avl_altura(raiz->esq) - avl_altura(raiz->dir);
}

unique_ptr<NodoAvl> avl_reequilibrar(unique_ptr<NodoAvl>& raiz, int &total_rotacao) {
    avl_atualizar_altura(raiz);
    int balanceamento = avl_fb(raiz);

    if (balanceamento == +2 && avl_fb(raiz ->esq) >= 0) {
        return avl_rotacao_direita(raiz, total_rotacao);
    } else if (balanceamento == -2 && avl_fb(raiz->dir) <= 0) {
        return avl_rotacao_esquerda(raiz, total_rotacao);
    } else if (balanceamento == +2 && avl_fb(raiz->esq) < 0) {
        raiz->esq = avl_rotacao_esquerda(raiz->esq, total_rotacao);
        return avl_rotacao_direita(raiz, total_rotacao  );
    } else if (balanceamento == -2 && avl_fb(raiz->dir) > 0) {
        raiz->dir = avl_rotacao_direita(raiz->dir, total_rotacao);
        return avl_rotacao_esquerda(raiz, total_rotacao);
    }

    return std::move(raiz);
}

unique_ptr<NodoAvl> avl_rotacao_direita(unique_ptr<NodoAvl>& raiz, int &total_rotacao) {
    unique_ptr<NodoAvl> nova_raiz = std::move(raiz->esq);
    unique_ptr<NodoAvl> temp = std::move(nova_raiz->dir);

    nova_raiz->dir = std::move(raiz);
    nova_raiz->dir->esq = std::move(temp);
    
    avl_atualizar_altura(nova_raiz->dir);
    avl_atualizar_altura(nova_raiz);
    total_rotacao++;

    return std::move(nova_raiz);
}

unique_ptr<NodoAvl> avl_rotacao_esquerda(unique_ptr<NodoAvl>& raiz, int &total_rotacao) {
    unique_ptr<NodoAvl> nova_raiz = std::move(raiz->dir);
    unique_ptr<NodoAvl> temp = std::move(nova_raiz->esq);

    nova_raiz->esq = std::move(raiz);
    nova_raiz->esq->dir = std::move(temp);
    
    avl_atualizar_altura(nova_raiz->esq);
    avl_atualizar_altura(nova_raiz);
    total_rotacao++;

    return std::move(nova_raiz);
}

int maior_codigo(Avl_Pedidos& avl){
    while(avl.raiz->dir != nullptr){
        avl.raiz = std::move(avl.raiz->dir);
    }
    return avl.raiz->cod_pedido;
}

int menor_codigo(Avl_Pedidos& avl){
    while(avl.raiz->esq != nullptr){
        avl.raiz = std::move(avl.raiz->esq);
    }
    return avl.raiz->cod_pedido; 
}