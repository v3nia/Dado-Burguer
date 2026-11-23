// src/main.cpp  --  ESQUELETO (nao compilado). Serve para juntar os blocos A e B.
//
// Legenda dos comentarios:
//   [IZA]    = ja existe no Bloco A (confira nomes/assinaturas reais)
//   [COLEGA] = Bloco B (jogo, telas, loop); nomes de modulos/funcoes a combinar
//   [COMBINAR] = decisao pendente entre os dois
//   TODO     = falta escrever

#include <iostream>
#include <string>

// ---- Modulos do Bloco A -------------------------------------------------
import TAD_avl;               // [IZA] Avl_Pedidos
import TAD_hash_cache;        // [IZA] HashCache_Pedidos (cache m = 16)
import TAD_lista_contigua;    // [IZA] ListaContigua_Historico
import arquivos;              // [IZA] carregar_pedidos, carregar_historico, anexar_historico
import metricas;              // [IZA] Metricas, g_metricas, mostrarRelatorio, gravarLog
import coleta_metricas;       // [IZA] coletarMetricasCarga, coletarMetricasPartida
// import ordenacao_merge;    // so a tela do Historico usa (nao precisa aqui)

// ---- Modulos do Bloco B (nomes a combinar) ------------------------------
// import terminal;           // [COLEGA] limpar tela, ler tecla/opcao, desenhar caixas
// import tela_inicial;       // [COLEGA] menu principal e tela "Como jogar"
import tela_historico;     // [IZA]    tela do Historico (20 entradas, ASCII art)
// import gameplay;           // [COLEGA] loop do jogo (receita sorteada, pilha, fila, heap...)

// ---- Opcoes do menu inicial (so a main as conhece) ----------------------
enum class OpcaoMenu { NOVO_JOGO, COMO_JOGAR, HISTORICO, SAIR };

// =========================================================================
// 1) Argumentos de linha de comando
//    O enunciado define os argumentos (confira a ordem exata la):
//    ex.: ./dado_burger <pedidos.txt> <historico.txt>
//    Permitido pela secao 10 (E/S e argumentos).
// =========================================================================
struct Argumentos {
    std::string arquivo_pedidos;
    std::string arquivo_historico;
};

// Devolve false se faltar argumento (a main imprime o "Uso:" e sai com erro).
static bool tratar_argumentos(int argc, char** argv, Argumentos& args) {
    // TODO: conferir quantos argumentos o enunciado exige.
    if (argc < 3) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "dado_burger")
                  << " <arquivo_pedidos> <arquivo_historico>\n";
        return false;
    }
    args.arquivo_pedidos   = argv[1];
    args.arquivo_historico = argv[2];
    return true;
}

int main(int argc, char** argv) {

    // =====================================================================
    // 2) Argumentos
    // =====================================================================
    Argumentos args;
    if (!tratar_argumentos(argc, argv, args)) return 1;

    // =====================================================================
    // 3) Objetos principais (donos de tudo: vivem durante o programa inteiro)
    //    Sao passados por referencia para as telas e para o gameplay.
    // =====================================================================
    Avl_Pedidos            avl;       // [IZA] receitas por codigo
    HashCache_Pedidos      cache;     // [IZA] cache (m = 16); [COMBINAR] zerar por partida?
    ListaContigua_Historico historico;// [IZA] historico em memoria

    // TODO: inicializar_listacontigua(historico);  (confira a assinatura real)
    inicializar_listacontigua(historico);
    
    // Lista encadeada de receitas, pilha, fila, heap: [COLEGA] criam dentro do gameplay.

    // =====================================================================
    // 4) Carga dos arquivos (ANTES do menu, para o jogo ja ter dados)
    //    4.1 pedidos -> AVL, com Timer (tempo de carga, secao 5.3)
    // =====================================================================
    // TODO: usar o Timer para guardar g_metricas.avl_tempo_carga.
    //       (a carregar_pedidos ja faz cronometro interno? confira no seu arquivos.cppm)
    auto resultado_pedidos = carregar_pedidos(args.arquivo_pedidos, avl);
    // TODO: se a carga falhou (arquivo inexistente/vazio), mostrar erro e return 1.
    //       Sem receitas o jogo nao tem o que sortear.
    if(!resultado_pedidos.sucesso){
        std::cerr << "Erro ao carregar pedidos.\n";
        return 1;
    }

    //    4.2 historico -> lista contigua
    //        Arquivo inexistente = historico vazio (nao e erro fatal).
    auto resultado_historico = carregar_historico(args.arquivo_historico, historico);
    // TODO: se resultado_historico.ignoradas > 0, avisar no terminal (linhas malformadas).
    if(resultado_historico.ignoradas>0){
        std::cerr << "Aviso: " << resultado_historico.ignoradas
                  << " linha(s) do historico foram ignoradas por estarem malformadas.\n";
    }

    //aqui não preciso atribuir diretamente as metricas, porque coletar_metricas_carga() já faz isso

    // =====================================================================
    // 5) Metricas da carga + primeira gravacao do medicoes.log
    // =====================================================================
    g_metricas.carregou = true;
    // TODO: g_metricas.avlTempoCarga = <tempo medido no passo 4.1>;
    g_metricas.avl_tempo_carga = resultado_pedidos.tempo_carregamento;

    coletar_metricas_carga(avl, historico, g_metricas);
    if (!gravar_log(g_metricas)) {
        std::cerr << "Aviso: nao foi possivel gravar medicoes.log\n";
    }

    // =====================================================================
    // 6) LOOP DA TELA INICIAL  (renderizacao e leitura de opcao: [COLEGA])
    // =====================================================================
    bool rodando = true;
    while (rodando) {

        // TODO [COLEGA]: desenhar o menu (ASCII art) e ler a opcao.
        OpcaoMenu opcao = OpcaoMenu::HISTORICO;   // placeholder ate existir a tela
        // opcao = tela_inicial::mostrar_menu();

        switch (opcao) {

        case OpcaoMenu::COMO_JOGAR:
            // [COLEGA] tela estatica com as regras; volta ao menu ao apertar uma tecla.
            // tela_inicial::mostrar_como_jogar();
            break;

        case OpcaoMenu::HISTORICO:
            
            // [IZA] tela do Historico: ordena uma COPIA da lista (nome ou pontuacao),
            // mede o tempo, preenche g_metricas.ord* e chama gravarLog.
            // tela_historico::mostrar(historico, g_metricas);
            telaHistorico(historico, g_metricas);

            
            break;

        case OpcaoMenu::NOVO_JOGO:
            // -----------------------------------------------------------------
            // LOOP DE GAMEPLAY (modulo gameplay [COLEGA])
            //
            // ANTES de entrar: [COMBINAR]
            //   - resetarMetricasPartida(g_metricas);
            resetar_metricas_partida(g_metricas);
            //   - zerar contadores da AVL e da hash para a partida comecar do zero
            avl.total_comparacao = 0;
            limpar_hash(&cache);
            //     (decisao de politica: cache zerado por partida?)
            //   - pedir o nome do jogador (vai para o historico)
            //
            // Dentro do gameplay, a cada pedido sorteado:
            //   1. sortear codigo no intervalo [menor_cod, maior_cod] da AVL
            //   2. buscar na cache (hash): hit -> usa a receita
            //   3. miss -> buscar na AVL (com Timer somando em g_metricas.avlTempoBuscas),
            //      inserir na cache, devolver a receita
            //   4. receita vai para a lista encadeada de preparo do [COLEGA]
            //
            // gameplay::jogar(avl, cache, historico, g_metricas) devolve a pontuacao.
            //
            // DEPOIS de sair: [COMBINAR]
            //   - coletarMetricasPartida(avl, cache, g_metricas);
            coletar_metricas_partida(avl, cache, g_metricas);
            //   - mostrarRelatorio(g_metricas);   // na tela (secao 5.2)
            mostrar_relatorio(g_metricas);
            //   - gravarLog(g_metricas);          // e no medicoes.log
            gravar_log(g_metricas);
            //   - pedir/confirmar o nome e gravar no historico:
            //       anexar_historico(args.arquivo_historico, nome, pontuacao);
            //       e inserir tambem na lista em memoria, para o Historico
            //       ja mostrar a partida nova sem reabrir o programa.
            // -----------------------------------------------------------------
            break;

        case OpcaoMenu::SAIR:
            rodando = false;
            break;
        }
    }

    // =====================================================================
    // 7) Encerramento
    //    Os unique_ptr liberam AVL, cache e lista automaticamente ao sair do main.
    //    Se o usuario sair sem jogar, o medicoes.log ja tem o bloco de carga (passo 5).
    // =====================================================================
    return 0;
}