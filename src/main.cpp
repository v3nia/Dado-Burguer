// src/main.cpp
#include <iostream>
#include <string>
#include "timer.hpp"

// ---- Modulos do Bloco A -------------------------------------------------
import TAD_avl;
import TAD_hash_cache;
import TAD_lista_contigua;
import arquivos;
import metricas;
import coleta_metricas;
import tela_historico;

// ---- Modulos do Bloco B -------------------------------------------------
import renderizacao;
import gameplay;

enum class OpcaoMenu { NOVO_JOGO, COMO_JOGAR, HISTORICO, SAIR };

struct Argumentos {
    std::string arquivo_pedidos;
    std::string arquivo_historico;
};

static bool tratar_argumentos(int argc, char** argv, Argumentos& args) {
    if (argc < 3) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "dado_burguer")
                  << " <arquivo_pedidos> <arquivo_historico>\n";
        return false;
    }
    args.arquivo_pedidos   = argv[1];
    args.arquivo_historico = argv[2];
    return true;
}

static OpcaoMenu mostrar_menu(const PacoteSprites& sprites) {
    ativarTerminal();

    Timer timerAnim;
    timer_iniciar(&timerAnim);
    int tickAnim = 0;
    bool precisaRedesenhar = true;

    OpcaoMenu escolha = OpcaoMenu::SAIR;
    bool escolheu = false;

    while (!escolheu) {
        if (timer_parar(&timerAnim) >= 0.25) {
            tickAnim++;
            timer_iniciar(&timerAnim);
            precisaRedesenhar = true;
        }

        if (precisaRedesenhar) {
            renderizarLogo(sprites, tickAnim);
            precisaRedesenhar = false;
        }

        char tecla = lerTecla(50);
        switch (tecla) {
            case '1': escolha = OpcaoMenu::NOVO_JOGO;  escolheu = true; break;
            case '2': escolha = OpcaoMenu::COMO_JOGAR; escolheu = true; break;
            case '3': escolha = OpcaoMenu::HISTORICO;  escolheu = true; break;
            case '0':
            case 27:  escolha = OpcaoMenu::SAIR;       escolheu = true; break;
            default:  break;
        }
    }

    desativarTerminal();
    std::cout << "\033[2J\033[H" << std::flush;
    return escolha;
}

static void mostrar_como_jogar() {
    std::cout << "\033[2J\033[H"
              << "=== COMO JOGAR DADO-BURGUER ===\n\n"
              << "Atenda os clientes da fila montando os lanches na ordem exata do pedido\n"
              << "(de baixo para cima, partindo do pao base) antes que a paciencia acabe!\n\n"
              << "-- CHAPA (Pereciveis - vencem com os turnos!) --\n"
              << "  [1] Preparar Carne    [2] Preparar Ovo    [3] Preparar Bacon\n\n"
              << "-- ESTOQUE (Perenes) --\n"
              << "  [4] Repor Queijo Prato   [5] Repor Cheddar\n"
              << "  [6] Repor Cebola         [7] Repor Tomate\n\n"
              << "-- MONTAGEM DO LANCHE --\n"
              << "  [C] Carne  [O] Ovo  [B] Bacon  [Q] Prato  [D] Cheddar\n"
              << "  [E] Cebola [T] Tomate [Z] Pao Topo (fecha o lanche)\n"
              << "  [Backspace] Desfazer ultima camada   [Enter] Entregar pedido\n"
              << "  [0 ou ESC] Encerrar partida\n\n"
              << "Pressione ENTER para voltar ao menu...";
    std::string pausa;
    std::getline(std::cin, pausa);
}

static std::string ler_nome_jogador() {
    std::string nome;
    while (true) {
        std::cout << "\nDigite seu nome para registrar no historico: ";
        if (!std::getline(std::cin, nome)) {
            return "Jogador";
        }
        while (!nome.empty() && (nome.back() == '\r' || nome.back() == ' ' || nome.back() == '\t')) {
            nome.pop_back();
        }
        if (!nome.empty()) {
            return nome;
        }
        std::cout << "Nome invalido. Tente novamente.\n";
    }
}

int main(int argc, char** argv) {
    // 1) Argumentos
    Argumentos args;
    if (!tratar_argumentos(argc, argv, args)) return 1;

    // 2) Estruturas principais
    Avl_Pedidos             avl;
    HashCache_Pedidos       cache;
    ListaContigua_Historico historico;
    inicializar_listacontigua(historico);

    // 3) Carga dos arquivos e sprites
    auto resultado_pedidos = carregar_pedidos(args.arquivo_pedidos, avl);
    if (!resultado_pedidos.sucesso) {
        std::cerr << "Erro ao carregar pedidos.\n";
        return 1;
    }

    auto resultado_historico = carregar_historico(args.arquivo_historico, historico);
    if (resultado_historico.ignoradas > 0) {
        std::cerr << "Aviso: " << resultado_historico.ignoradas
                  << " linha(s) do historico foram ignoradas por estarem malformadas.\n";
    }

    PacoteSprites sprites;
    if (!carregarTodosSprites(sprites)) {
        std::cerr << "Aviso: um ou mais sprites em assets/ nao foram encontrados.\n";
    }

    // 4) Metricas da carga + gravacao inicial do log
    g_metricas.carregou = true;
    g_metricas.avl_tempo_carga = resultado_pedidos.tempo_carregamento;
    coletar_metricas_carga(avl, historico, g_metricas);
    if (!gravar_log(g_metricas)) {
        std::cerr << "Aviso: nao foi possivel gravar medicoes.log\n";
    }

    // 5) Loop do menu inicial
    bool rodando = true;
    while (rodando) {
        OpcaoMenu opcao = mostrar_menu(sprites);

        switch (opcao) {
            case OpcaoMenu::COMO_JOGAR:
                mostrar_como_jogar();
                break;

            case OpcaoMenu::HISTORICO:
                telaHistorico(historico, g_metricas);
                break;

            case OpcaoMenu::NOVO_JOGO: {
                resetar_metricas_partida(g_metricas);
                avl.total_comparacao = 0;
                limpar_hash(&cache);

                int pontuacao = executarGameplay(avl, cache, g_metricas, sprites);

                coletar_metricas_partida(avl, cache, g_metricas);

                std::cout << "\033[2J\033[H"
                          << "=== FIM DE PARTIDA ===\n"
                          << "Pontuacao final: " << pontuacao << " pts\n\n";
                mostrar_relatorio(g_metricas);
                gravar_log(g_metricas);

                std::string nome = ler_nome_jogador();
                if (anexar_historico(args.arquivo_historico, nome, pontuacao)) {
                    NodoListaContigua novo{nome, pontuacao};
                    inserir_listacontigua(historico, novo, tamanho_listacontigua(historico));
                    std::cout << "Pontuacao salva no historico!\n";
                }

                std::cout << "\nPressione ENTER para voltar ao menu...";
                std::string pausa;
                std::getline(std::cin, pausa);
                break;
            }

            case OpcaoMenu::SAIR:
                rodando = false;
                break;
        }
    }

    return 0;
}