module;

export module coleta_metricas;

// =====================================================================
// coleta_metricas: copia os contadores que vivem dentro das estruturas
// (Avl_Pedidos, HashCache_Pedidos, ListaContigua_Historico) para o
// Metricas. Fica separado de "metricas" para que esse módulo não precise
// importar as estruturas (evita dependência circular).
// Ajuste os nomes dos módulos importados aos que você usa.
// =====================================================================

import TAD_avl;
import TAD_hash_cache;
import TAD_lista_contigua;
import metricas;

export {

// Chame logo depois do carregamento (AVL + histórico), antes de gravarLog.
// O tempo de carga (avlTempoCarga) e o flag 'carregou' são preenchidos
// no main, junto do Timer.
void coletarMetricasCarga(const Avl_Pedidos& avl,
                          const ListaContigua_Historico& hist,
                          Metricas& m) {
    m.avlNos       = avl.total_insercao;
    m.avlRotacoes  = avl.total_rotacao;
    m.avlAltura    = avl_altura(avl.raiz);   // SUA função de altura (vazia = 0, raiz = 1)
    m.histEntradas = hist.tamanho;
}

// Chame ao encerrar a partida (vitória, derrota ou saída voluntária),
// antes de gravarLog.
// avlTempoBuscas NÃO é copiado aqui: ele já é acumulado direto em
// g_metricas, com o Timer, no ponto onde a busca na AVL é chamada.
void coletarMetricasPartida(const Avl_Pedidos& avl,
                            const HashCache_Pedidos& hash,
                            Metricas& m) {
    m.avlComparacoes = avl.total_comparacao;
    m.avlBuscas      = hash.miss;   // cada miss gera exatamente uma busca na AVL

    m.hashM          = hash.m;
    m.hashN          = hash.n;
    m.hashHits       = hash.hit;
    m.hashMisses     = hash.miss;
    m.hashColisoes   = hash.colisoes;

    m.partidaTerminou = true;
}

} // export