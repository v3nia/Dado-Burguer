module;

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include "timer.hpp"

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
static uniform_int_distribution<> dis_fila(10, 20);
static uniform_int_distribution<> dis_paciencia(15, 22);

constexpr int VAL_CARNE = 7; // turnos até vencer
constexpr int VAL_OVO   = 6;
constexpr int VAL_BACON = 8;

export struct Estoque {
    minHeap carne;
    minHeap ovo;
    minHeap bacon;

    int queijoSuico   = 0;
    int queijoCheddar = 0;
    int cebola        = 0;
    int tomate        = 0;
};

export struct EstadoJogo {
    Estoque estoque;
    FilaClientes fila;
    Pilha montagem;

    int turnoAtual        = 1;
    int pontuacao         = 0;
    int clientesAtendidos = 0;
    static constexpr int pacienciaMax = 22;
    bool rodando          = true;
    bool vitoria          = false;
    string feedback       = "Bem-vindo ao Dado-Burguer! Prepare os ingredientes e monte o pedido.";
};

/* ==================== FUNÇÕES AUXILIARES ==================== */

Receita sortearReceita(Avl_Pedidos& avl, HashCache_Pedidos& cache, Metricas& metricas) {
    uniform_int_distribution<> dis(avl.menor_cod, avl.maior_cod);
    int sorteado = 0;
    string stringret;
    Timer t;

    do {
        sorteado = dis(gen);
        stringret = buscar_hash(&cache, sorteado);
        if (!stringret.empty()) {
            return construirReceita(stringret);
        }

        timer_iniciar(&t);
        stringret = buscar_pedido(avl.raiz, sorteado, avl.total_comparacao);
        metricas.avl_tempo_buscas += timer_parar(&t);
    } while (stringret.empty());

    inserir_hash(&cache, sorteado, stringret);
    return construirReceita(stringret);
}

DadosTela montarDadosTela(const EstadoJogo& jogo) {
    DadosTela dados;
    dados.turnoAtual        = jogo.turnoAtual;
    dados.pontuacao         = jogo.pontuacao;
    dados.clientesAtendidos = jogo.clientesAtendidos;
    dados.clientesNaFila    = jogo.fila.tamanho;
    dados.pacienciaMax      = jogo.pacienciaMax;
    dados.feedback          = jogo.feedback;

    Cliente* atual = visualizarInicio(jogo.fila);
    if (atual != nullptr) {
        dados.pacienciaAtual = atual->paciencia;
        dados.textoReceita   = stringReceita(atual->pedido);
    }

    dados.textoMontagem = stringPilha(jogo.montagem);

    dados.carneQtd = jogo.estoque.carne.tamanho;
    dados.ovoQtd   = jogo.estoque.ovo.tamanho;
    dados.baconQtd = jogo.estoque.bacon.tamanho;

    dados.carneProxVenc = (jogo.estoque.carne.tamanho > 0)
                        ? (jogo.estoque.carne.pereciveis[0] - jogo.turnoAtual) : -1;
    dados.ovoProxVenc   = (jogo.estoque.ovo.tamanho > 0)
                        ? (jogo.estoque.ovo.pereciveis[0] - jogo.turnoAtual) : -1;
    dados.baconProxVenc = (jogo.estoque.bacon.tamanho > 0)
                        ? (jogo.estoque.bacon.pereciveis[0] - jogo.turnoAtual) : -1;

    dados.queijoSuico   = jogo.estoque.queijoSuico;
    dados.queijoCheddar = jogo.estoque.queijoCheddar;
    dados.cebola        = jogo.estoque.cebola;
    dados.tomate        = jogo.estoque.tomate;

    return dados;
}

void inicializarPartida(EstadoJogo& jogo, Avl_Pedidos& avl, HashCache_Pedidos& cache, Metricas& metricas) {
    iniciarMontagem(jogo.montagem);

    int qtdClientes = dis_fila(gen);
    for (int i = 0; i < qtdClientes; ++i) {
        enfileirar(jogo.fila, sortearReceita(avl, cache, metricas), dis_paciencia(gen));
    }
}

void avancarTurno(EstadoJogo& jogo, bool descontarPaciencia) {
    if (!jogo.rodando) return;

    jogo.turnoAtual++;
    descartarVencidos(jogo.estoque.carne, jogo.turnoAtual);
    descartarVencidos(jogo.estoque.ovo,   jogo.turnoAtual);
    descartarVencidos(jogo.estoque.bacon, jogo.turnoAtual);

    if (!descontarPaciencia) return;

    Cliente* atual = visualizarInicio(jogo.fila);
    if (atual != nullptr) {
        atual->paciencia--;
        if (atual->paciencia <= 0) {
            jogo.feedback = "DERROTA! O cliente perdeu a paciencia e foi embora!";
            jogo.vitoria  = false;
            jogo.rodando  = false;
        }
    }
}

bool prepararPerecivel(EstadoJogo& jogo, char tipo) {
    bool ok = false;
    string nome;

    switch (tipo) {
        case 'n':
            ok = inserir(jogo.estoque.carne, jogo.turnoAtual + VAL_CARNE);
            nome = "Carne";
            break;
        case 'o':
            ok = inserir(jogo.estoque.ovo, jogo.turnoAtual + VAL_OVO);
            nome = "Ovo frito";
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

    jogo.feedback = nome + " preparado na chapa!";
    return true;
}

bool prepararPerene(EstadoJogo& jogo, char tipo) {
    switch (tipo) {
        case 's':
            jogo.estoque.queijoSuico++;
            jogo.feedback = "Queijo suico preparado (+1).";
            return true;
        case 'q':
            jogo.estoque.queijoCheddar++;
            jogo.feedback = "Queijo cheddar preparado (+1).";
            return true;
        case 'c':
            jogo.estoque.cebola++;
            jogo.feedback = "Cebola preparada (+1).";
            return true;
        case 't':
            jogo.estoque.tomate++;
            jogo.feedback = "Tomate preparado (+1).";
            return true;
        default:
            return false;
    }
}

// Retorna: 0 = nao gastou turno, 1 = gastou turno normal, 2 = pedido entregue com sucesso
int montarIngrediente(EstadoJogo& jogo, char ing) {
    string pilhaAtual = stringPilha(jogo.montagem);
    if (!pilhaAtual.empty() && pilhaAtual[0] == 'z') {
        jogo.feedback = "Lanche fechado com pao topo! Use [Backspace] para corrigir.";
        return 0;
    }

    switch (ing) {
        // Perecíveis (consomem topo do Min-Heap)
        case 'n':
            if (!removerHeap(jogo.estoque.carne)) {
                jogo.feedback = "Sem carne na chapa! Aperte [1] para preparar.";
                return 0;
            }
            break;
        case 'o':
            if (!removerHeap(jogo.estoque.ovo)) {
                jogo.feedback = "Sem ovo na chapa! Aperte [2] para preparar.";
                return 0;
            }
            break;
        case 'b':
            if (!removerHeap(jogo.estoque.bacon)) {
                jogo.feedback = "Sem bacon na chapa! Aperte [3] para preparar.";
                return 0;
            }
            break;

        // Perenes (consomem 1 unidade preparada)
        case 's':
            if (jogo.estoque.queijoSuico <= 0) {
                jogo.feedback = "Sem queijo suico! Aperte [4] para preparar.";
                return 0;
            }
            jogo.estoque.queijoSuico--;
            break;
        case 'q':
            if (jogo.estoque.queijoCheddar <= 0) {
                jogo.feedback = "Sem queijo cheddar! Aperte [5] para preparar.";
                return 0;
            }
            jogo.estoque.queijoCheddar--;
            break;
        case 'c':
            if (jogo.estoque.cebola <= 0) {
                jogo.feedback = "Sem cebola! Aperte [6] para preparar.";
                return 0;
            }
            jogo.estoque.cebola--;
            break;
        case 't':
            if (jogo.estoque.tomate <= 0) {
                jogo.feedback = "Sem tomate! Aperte [7] para preparar.";
                return 0;
            }
            jogo.estoque.tomate--;
            break;

        // Infinitos (sem preparo)
        case 'z':
        case 'k':
        case 'm':
        case 'p':
        case 'a':
            break;

        default:
            return 0;
    }

    empilhar(jogo.montagem, ing);

    // Ao colocar o pão de cima ('z'), verifica automaticamente o pedido
    if (ing == 'z') {
        Cliente* atual = visualizarInicio(jogo.fila);
        if (atual != nullptr && comparar(atual->pedido, jogo.montagem)) {
            jogo.clientesAtendidos++;
            jogo.pontuacao = jogo.clientesAtendidos;

            desenfileirar(jogo.fila);
            iniciarMontagem(jogo.montagem);

            if (jogo.fila.tamanho == 0) {
                jogo.feedback = "VITORIA! Todos os clientes foram atendidos!";
                jogo.vitoria  = true;
                jogo.rodando  = false;
            } else {
                jogo.feedback = "Pedido entregue com sucesso! (" + std::to_string(jogo.clientesAtendidos) + " atendidos)";
            }
            return 2;
        }

        jogo.feedback = "Pedido incorreto! Use [Backspace] para remover o pao topo e corrigir.";
        return 1;
    }

    jogo.feedback = string("Ingrediente '") + ing + "' adicionado ao hamburguer.";
    return 1;
}

bool desfazerIngrediente(EstadoJogo& jogo) {
    if (stringPilha(jogo.montagem).empty()) {
        jogo.feedback = "A montagem ja esta vazia (apenas pao de baixo).";
        return false;
    }

    desempilhar(jogo.montagem);
    jogo.feedback = "Ingrediente do topo descartado.";
    return true;
}

void processarTecla(EstadoJogo& jogo, char tecla) {
    int resultadoTurno = 0; // 0 = nenhum, 1 = turno normal, 2 = turno de entrega bem-sucedida

    switch (tecla) {
        // Preparar perecíveis na chapa (Min-Heap)
        case '1': resultadoTurno = prepararPerecivel(jogo, 'n') ? 1 : 0; break;
        case '2': resultadoTurno = prepararPerecivel(jogo, 'o') ? 1 : 0; break;
        case '3': resultadoTurno = prepararPerecivel(jogo, 'b') ? 1 : 0; break;

        // Preparar perenes (+1 unidade cada)
        case '4': resultadoTurno = prepararPerene(jogo, 's') ? 1 : 0; break;
        case '5': resultadoTurno = prepararPerene(jogo, 'q') ? 1 : 0; break;
        case '6': resultadoTurno = prepararPerene(jogo, 'c') ? 1 : 0; break;
        case '7': resultadoTurno = prepararPerene(jogo, 't') ? 1 : 0; break;

        // Montar hambúrguer (todos os 12 símbolos da Tabela 1)
        case 'z': case 'Z': resultadoTurno = montarIngrediente(jogo, 'z'); break;
        case 'k': case 'K': resultadoTurno = montarIngrediente(jogo, 'k'); break;
        case 'm': case 'M': resultadoTurno = montarIngrediente(jogo, 'm'); break;
        case 'p': case 'P': resultadoTurno = montarIngrediente(jogo, 'p'); break;
        case 'a': case 'A': resultadoTurno = montarIngrediente(jogo, 'a'); break;
        case 's': case 'S': resultadoTurno = montarIngrediente(jogo, 's'); break;
        case 'q': case 'Q': resultadoTurno = montarIngrediente(jogo, 'q'); break;
        case 'c': case 'C': resultadoTurno = montarIngrediente(jogo, 'c'); break;
        case 't': case 'T': resultadoTurno = montarIngrediente(jogo, 't'); break;
        case 'n': case 'N': resultadoTurno = montarIngrediente(jogo, 'n'); break;
        case 'o': case 'O': resultadoTurno = montarIngrediente(jogo, 'o'); break;
        case 'b': case 'B': resultadoTurno = montarIngrediente(jogo, 'b'); break;

        // Remover ingrediente do topo (Linux = 127, Windows = '\b')
        case 127:
        case '\b':
            resultadoTurno = desfazerIngrediente(jogo) ? 1 : 0;
            break;

        // Sair voluntariamente (ESC isolado ou 0)
        case 27:
        case '0':
            jogo.feedback = "Partida encerrada pelo jogador.";
            jogo.rodando  = false;
            return;

        default:
            return;
    }

    if (resultadoTurno == 1) {
        avancarTurno(jogo, true);
    } else if (resultadoTurno == 2) {
        avancarTurno(jogo, false); // novo cliente entra com paciência intacta
    }
}

/* ==================== FUNÇÃO EXPORTADA ==================== */

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

    std::cout << "\033[2J\033[H";
    if (jogo.vitoria) {
        std::cout << "\033[1;32m=== VITORIA! TODOS OS CLIENTES ATENDIDOS! ===\033[0m\n";
    } else {
        std::cout << "\033[1;31m=== FIM DE PARTIDA ===\033[0m\n"
                  << jogo.feedback << "\n";
    }

    return jogo.pontuacao;
}