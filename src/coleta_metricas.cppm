module;

export module coleta_metricas;

// =====================================================================
// coleta_metricas: copia os contadores que vivem dentro das estruturas
// (Avl_Pedidos, HashCache_Pedidos, ListaContigua_Historico) para o
// Metricas. Fica separado de "metricas" para que esse módulo não precise
// importar as estruturas (evita dependência circular).
// =====================================================================

import TAD_avl;
import TAD_hash_cache;
import TAD_lista_contigua;
import metricas;

export {

// Chame logo depois do carregamento (AVL + histórico), antes de gravar_log.
// O tempo de carga (avl_tempo_carga) e o flag 'carregou' são preenchidos
// no main, junto do Timer.
void coletar_metricas_carga(const Avl_Pedidos& avl,
                            const ListaContigua_Historico& hist,
                            Metricas& m) {
    m.avl_nos       = avl.total_insercao;
    m.avl_rotacoes  = avl.total_rotacao;
    m.avl_altura    = avl_altura(avl.raiz);   // função de altura (vazia = 0, raiz = 1)
    m.hist_entradas = hist.tamanho;
}

// Chame ao encerrar a partida (vitória, derrota ou saída voluntária),
// antes de gravar_log.
// avl_tempo_buscas NÃO é copiado aqui: ele já é acumulado direto em
// g_metricas, com o Timer, no ponto onde a busca na AVL é chamada.
void coletar_metricas_partida(const Avl_Pedidos& avl,
                              const HashCache_Pedidos& hash,
                              Metricas& m) {
    m.avl_comparacoes = avl.total_comparacao;
    m.avl_buscas      = hash.miss;   // cada miss gera exatamente uma busca na AVL

    m.hash_m          = hash.m;
    m.hash_n          = hash.n;
    m.hash_hits       = hash.hit;
    m.hash_miss       = hash.miss;
    m.hash_colisoes   = hash.colisoes;

    int hits_int = static_cast<int>(hash.hit);
    int misses_int = static_cast<int>(hash.miss);
    m.hash_taxa_acerto = static_cast<double>(taxa_acerto(hits_int, misses_int)) * 100.0;

    m.partida_terminou = true;
}

} // export