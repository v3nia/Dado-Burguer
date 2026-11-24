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
static uniform_int_distribution<> dis_paciencia(10, 20);

constexpr int VAL_CARNE        = 6;  // turnos até vencer
constexpr int VAL_OVO          = 5;
constexpr int VAL_BACON        = 8;
constexpr int REPOSICAO_PERENE = 4;  // unidades ganhas ao repor perene
constexpr int MAX_PERDIDOS     = 3;  // limite de clientes perdidos (Game Over)

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

    int turnoAtual = 1;
    int pontuacao = 0;
    int clientesAtendidos = 0;
    int clientesPerdidos = 0;
    static constexpr int pacienciaMax = 20;
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
        metricas.avl_tempo_buscas += timer_parar(&t);
    }while(stringret.empty());

    inserir_hash(&cache, sorteado, stringret);
    return construirReceita(stringret);
}

DadosTela montarDadosTela(const EstadoJogo &jogo) {
    DadosTela dados;
    dados.turnoAtual = jogo.turnoAtual;
    dados.pontuacao = jogo.pontuacao;
    dados.clientesAtendidos = jogo.clientesAtendidos;
    dados.clientesNaFila = jogo.fila.tamanho;
    dados.pacienciaMax = jogo.pacienciaMax;

    Cliente *atual = visualizarInicio(jogo.fila);
    if(atual != nullptr) {
        dados.pacienciaAtual = atual->paciencia;
        dados.textoReceita = stringReceita(atual->pedido);
    }

    dados.textoMontagem = stringPilha(jogo.montagem);

    dados.baconQtd = jogo.estoque.bacon.tamanho;
    dados.carneQtd = jogo.estoque.carne.tamanho;
    dados.ovoQtd = jogo.estoque.ovo.tamanho;

    dados.carneProxVenc = (jogo.estoque.carne.tamanho > 0) ? (jogo.estoque.carne.pereciveis[0] - jogo.turnoAtual) : -1;
    dados.ovoProxVenc = (jogo.estoque.ovo.tamanho > 0) ? (jogo.estoque.ovo.pereciveis[0] - jogo.turnoAtual) : -1;
    dados.baconProxVenc = (jogo.estoque.bacon.tamanho > 0) ? (jogo.estoque.bacon.pereciveis[0] - jogo.turnoAtual) : -1;
    
    dados.queijoPrato = jogo.estoque.queijoPrato;
    dados.queijoCheddar = jogo.estoque.queijoCheddar;
    dados.cebola = jogo.estoque.cebola;
    dados.tomate = jogo.estoque.tomate;

    return dados;
}

void inicializarPartida(EstadoJogo& jogo, Avl_Pedidos& avl, HashCache_Pedidos& cache, Metricas& metricas){
    iniciarMontagem(jogo.montagem);

    jogo.estoque.queijoPrato = 30;
    jogo.estoque.queijoCheddar = 30;
    jogo.estoque.cebola = 30;
    jogo.estoque.tomate = 30;

    int qtdClientes = dis_paciencia(gen);
    for(int i = 0; i < qtdClientes; i++){
        enfileirar(jogo.fila, sortearReceita(avl, cache, metricas), dis_paciencia(gen));
    }
}

bool prepararPerecivel(EstadoJogo& jogo, char tipo) {
    bool ok = false;
    string nome;

    switch (tipo) {
        case 'c':
            ok = inserir(jogo.estoque.carne, jogo.turnoAtual + VAL_CARNE);
            nome = "Carne";
            break;
        case 'o':
            ok = inserir(jogo.estoque.ovo, jogo.turnoAtual + VAL_OVO);
            nome = "Ovo";
            break;
        case 'b':
            ok = inserir(jogo.estoque.bacon, jogo.turnoAtual + VAL_BACON);
            nome = "Bacon";
            break;
        default:
            return false;
    }

    if (!ok) {
        jogo.feedback = "Chapa lotada para " + nome + " (max 8)!";
        return false;
    }

    jogo.feedback = nome + " preparada na chapa!";
    return true;
}

bool reporPerene(EstadoJogo& jogo, char tipo) {
    switch (tipo) {
        case 'q':
            jogo.estoque.queijoPrato += REPOSICAO_PERENE;
            jogo.feedback = "Queijo prato reposto (+" + std::to_string(REPOSICAO_PERENE) + ").";
            return true;
        case 'd':
            jogo.estoque.queijoCheddar += REPOSICAO_PERENE;
            jogo.feedback = "Cheddar reposto (+" + std::to_string(REPOSICAO_PERENE) + ").";
            return true;
        case 'e':
            jogo.estoque.cebola += REPOSICAO_PERENE;
            jogo.feedback = "Cebola reposta (+" + std::to_string(REPOSICAO_PERENE) + ").";
            return true;
        case 't':
            jogo.estoque.tomate += REPOSICAO_PERENE;
            jogo.feedback = "Tomate reposto (+" + std::to_string(REPOSICAO_PERENE) + ").";
            return true;
        default:
            return false;
    }
}

bool montarIngrediente(EstadoJogo& jogo, char ing) {
    // Evita consumir estoque se o pao topo ('z') ja fechou o lanche
    string pilhaAtual = stringPilha(jogo.montagem);
    if (!pilhaAtual.empty() && pilhaAtual[0] == 'z') {
        jogo.feedback = "Lanche ja fechado com pao topo! Entregue [Enter] ou desfaça [Backspace].";
        return false;
    }

    switch (ing) {
        case 'c':
            if (!removerHeap(jogo.estoque.carne)) {
                jogo.feedback = "Sem carne pronta na chapa! Aperte [1] para preparar.";
                return false;
            }
            break;
        case 'o':
            if (!removerHeap(jogo.estoque.ovo)) {
                jogo.feedback = "Sem ovo pronto na chapa! Aperte [2] para preparar.";
                return false;
            }
            break;
        case 'b':
            if (!removerHeap(jogo.estoque.bacon)) {
                jogo.feedback = "Sem bacon pronto na chapa! Aperte [3] para preparar.";
                return false;
            }
            break;
        case 'q':
            if (jogo.estoque.queijoPrato <= 0) {
                jogo.feedback = "Sem queijo prato! Aperte [4] para repor.";
                return false;
            }
            jogo.estoque.queijoPrato--;
            break;
        case 'd':
            if (jogo.estoque.queijoCheddar <= 0) {
                jogo.feedback = "Sem cheddar! Aperte [5] para repor.";
                return false;
            }
            jogo.estoque.queijoCheddar--;
            break;
        case 'e':
            if (jogo.estoque.cebola <= 0) {
                jogo.feedback = "Sem cebola! Aperte [6] para repor.";
                return false;
            }
            jogo.estoque.cebola--;
            break;
        case 't':
            if (jogo.estoque.tomate <= 0) {
                jogo.feedback = "Sem tomate! Aperte [7] para repor.";
                return false;
            }
            jogo.estoque.tomate--;
            break;
        case 'z':
            // Pao topo e livre (nao consome estoque)
            break;
        default:
            return false;
    }

    empilhar(jogo.montagem, ing);
    jogo.feedback = string("Ingrediente '") + ing + "' empilhado!";
    return true;
}

bool desfazerIngrediente(EstadoJogo& jogo) {
    if (stringPilha(jogo.montagem).empty()) {
        jogo.feedback = "A montagem ja esta vazia (apenas pao base).";
        return false;
    }

    desempilhar(jogo.montagem);
    jogo.feedback = "Ultimo ingrediente removido da montagem.";
    return true; // troque para false se nao quiser cobrar turno ao desfazer
}

bool entregarPedido(EstadoJogo& jogo) {
    Cliente* atual = visualizarInicio(jogo.fila);
    if (atual == nullptr) {
        jogo.rodando = false;
        return false;
    }

    if (comparar(atual->pedido, jogo.montagem)) {
        int ganho = 100 + atual->paciencia * 10;
        jogo.pontuacao += ganho;
        jogo.clientesAtendidos++;

        desenfileirar(jogo.fila);
        iniciarMontagem(jogo.montagem);

        if (jogo.fila.tamanho == 0) {
            jogo.feedback = "Expediente concluido! Todos os clientes foram atendidos!";
            jogo.rodando = false;
        } else {
            jogo.feedback = "Pedido entregue com sucesso! +" + std::to_string(ganho) + " pts.";
        }
        return true;
    }

    jogo.feedback = "Pedido incorreto! Use [Backspace] para corrigir e tente novamente.";
    return true; // penalidade de 1 turno por entrega errada
}

void avancarTurno(EstadoJogo& jogo) {
    if (!jogo.rodando) return;

    jogo.turnoAtual++;
    descartarVencidos(jogo.estoque.carne, jogo.turnoAtual);
    descartarVencidos(jogo.estoque.ovo,   jogo.turnoAtual);
    descartarVencidos(jogo.estoque.bacon, jogo.turnoAtual);

    Cliente* atual = visualizarInicio(jogo.fila);
    if (atual != nullptr) {
        atual->paciencia--;
        if (atual->paciencia <= 0) {
            desenfileirar(jogo.fila);
            iniciarMontagem(jogo.montagem);
            jogo.clientesPerdidos++;
            jogo.feedback = "Cliente perdeu a paciencia e foi embora! ("
                          + std::to_string(jogo.clientesPerdidos) + "/"
                          + std::to_string(MAX_PERDIDOS) + ")";
        }
    }

    if (jogo.clientesPerdidos >= MAX_PERDIDOS || jogo.fila.tamanho == 0) {
        jogo.rodando = false;
    }
}

void processarTecla(EstadoJogo& jogo, char tecla) {
    bool gastouTurno = false;

    switch (tecla) {
        // Preparar perecíveis na chapa (Min-Heap)
        case '1': gastouTurno = prepararPerecivel(jogo, 'c'); break;
        case '2': gastouTurno = prepararPerecivel(jogo, 'o'); break;
        case '3': gastouTurno = prepararPerecivel(jogo, 'b'); break;

        // Repor perenes
        case '4': gastouTurno = reporPerene(jogo, 'q'); break;
        case '5': gastouTurno = reporPerene(jogo, 'd'); break;
        case '6': gastouTurno = reporPerene(jogo, 'e'); break;
        case '7': gastouTurno = reporPerene(jogo, 't'); break;

        // Montar hambúrguer (Pilha)
        case 'c': case 'C': gastouTurno = montarIngrediente(jogo, 'c'); break;
        case 'o': case 'O': gastouTurno = montarIngrediente(jogo, 'o'); break;
        case 'b': case 'B': gastouTurno = montarIngrediente(jogo, 'b'); break;
        case 'q': case 'Q': gastouTurno = montarIngrediente(jogo, 'q'); break;
        case 'd': case 'D': gastouTurno = montarIngrediente(jogo, 'd'); break;
        case 'e': case 'E': gastouTurno = montarIngrediente(jogo, 'e'); break;
        case 't': case 'T': gastouTurno = montarIngrediente(jogo, 't'); break;
        case 'z': case 'Z': gastouTurno = montarIngrediente(jogo, 'z'); break;

        // Desfazer (Linux = 127, Windows = '\b')
        case 127:
        case '\b':
            gastouTurno = desfazerIngrediente(jogo);
            break;

        // Entregar (Linux = '\n', Windows = '\r')
        case '\n':
        case '\r':
            gastouTurno = entregarPedido(jogo);
            break;

        // Sair (ESC ou 0)
        case 27:
        case '0':
            jogo.rodando = false;
            return;

        default:
            return;
    }

    if (gastouTurno) {
        avancarTurno(jogo);
    }
}

/* FUNÇÕES EXPORTADAS */
// no fim só foi uma baita unica funcao, mais facil
export int executarGameplay(Avl_Pedidos& avl,
                            HashCache_Pedidos& cache,
                            Metricas& metricas,
                            const PacoteSprites& sprites) {
    EstadoJogo jogo;
    inicializarPartida(jogo, avl, cache, metricas);

    ativarTerminal();

    Timer timerAnim;
    timer_iniciar(&timerAnim);
    int tickAnim = 0;
    bool precisaRedesenhar = true;

    while (jogo.rodando) {
        // Anima sprites ~4 vezes por segundo (independente do teclado)
        if (timer_parar(&timerAnim) >= 0.25) {
            tickAnim++;
            timer_iniciar(&timerAnim);
            precisaRedesenhar = true;
        }

        if (precisaRedesenhar) {
            DadosTela dados = montarDadosTela(jogo);
            renderizarGameplay(dados, sprites, tickAnim);
            precisaRedesenhar = false;
        }

        char tecla = lerTecla(50);
        if (tecla != '\0') {
            processarTecla(jogo, tecla);
            precisaRedesenhar = true;
        }
    }

    desativarTerminal();
    return jogo.pontuacao;
}