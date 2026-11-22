module;
#include <fstream>
#include <iomanip>

export module metricas;

// =====================================================================
// metricas: guarda todas as medições exigidas (seção 5 da especificação)
// e grava o medicoes.log. Este módulo NÃO importa AVL, hash nem histórico
// (evita dependência circular). A cópia dos contadores das estruturas
// para cá é feita no módulo coleta_metricas.
// =====================================================================

export {

struct Metricas {
    // --- AVL: carregamento (§5.1) ---
    long long avlNos          = 0;
    int       avlAltura       = 0;      // árvore vazia = 0, só raiz = 1
    long long avlRotacoes     = 0;      // rotação dupla = 2 simples
    double    avlTempoCarga   = 0.0;    // segundos

    // --- Histórico: contexto (§5.1) ---
    int       histEntradas    = 0;

    // --- AVL: buscas durante a partida (§5.2) ---
    long long avlBuscas       = 0;
    long long avlComparacoes  = 0;
    double    avlTempoBuscas  = 0.0;    // acumulado, em segundos

    // --- Tabela hash (§5.2) ---
    int       hashM           = 0;      // tamanho da tabela
    long long hashN           = 0;      // entradas ocupadas
    long long hashHits        = 0;
    long long hashMisses      = 0;
    long long hashColisoes    = 0;

    // --- Ordenação do histórico (atualizada a cada acesso à tela) ---
    int       ordTamanho      = 0;
    int       ordAcessos      = 0;
    double    ordTempoNome    = -1.0;   // -1 = ainda não medido
    double    ordTempoPontos  = -1.0;

    // --- controle de quais blocos já podem aparecer no log ---
    bool      carregou        = false;  // true ao fim do carregamento
    bool      partidaTerminou = false;  // true ao encerrar a partida
};

// Instância global compartilhada por todos os módulos.
Metricas g_metricas;

// Zera as métricas de partida. Chame ao INICIAR cada partida.
// Não mexe nas métricas de carregamento nem nas de ordenação.
void resetarMetricasPartida(Metricas& m) {
    m.avlBuscas       = 0;
    m.avlComparacoes  = 0;
    m.avlTempoBuscas  = 0.0;
    m.hashN           = 0;
    m.hashHits        = 0;
    m.hashMisses      = 0;
    m.hashColisoes    = 0;
    m.partidaTerminou = false;
}

// Reescreve o medicoes.log inteiro (cria ou sobrescreve). Chame:
//  1) após o carregamento, 2) ao fim de cada partida,
//  3) a cada acesso à tela de Histórico.
// Devolve false se não conseguiu abrir o arquivo.
bool gravarLog(const Metricas& m, const char* algoritmo = "Merge Sort") {
    std::ofstream out("medicoes.log");
    if (!out.is_open()) return false;

    out << std::fixed;
    out << "=== RELATORIO DE DESEMPENHO ===\n";

    if (m.carregou) {
        out << "-- AVL: carregamento --\n"
            << "Nos inseridos: " << m.avlNos << "\n"
            << "Altura resultante: " << m.avlAltura << "\n"
            << "Rotacoes (insercao): " << m.avlRotacoes << "\n"
            << "Tempo de carregamento: " << std::setprecision(6)
            << m.avlTempoCarga << " s\n"
            << "-- Historico: carregamento --\n"
            << "Entradas carregadas: " << m.histEntradas << "\n";
    }

    if (m.partidaTerminou) {
        // proteções contra divisão por zero (média = 0 se não houve busca)
        double media = (m.avlBuscas > 0)
            ? static_cast<double>(m.avlComparacoes) / m.avlBuscas : 0.0;
        long long acessos = m.hashHits + m.hashMisses;
        double taxa = (acessos > 0) ? 100.0 * m.hashHits / acessos : 0.0;
        double lambda = (m.hashM > 0)
            ? static_cast<double>(m.hashN) / m.hashM : 0.0;

        out << "-- AVL: buscas durante a partida --\n"
            << "Total de buscas: " << m.avlBuscas << "\n"
            << "Total de comparacoes: " << m.avlComparacoes << "\n"
            << "Media de comparacoes por busca: " << std::setprecision(2)
            << media << "\n"
            << "Tempo total acumulado: " << std::setprecision(6)
            << m.avlTempoBuscas << " s\n"
            << "-- Tabela hash --\n"
            << "Tamanho (m): " << m.hashM << "\n"
            << "Entradas ocupadas (n): " << m.hashN << "\n"
            << "Fator de carga (lambda): " << std::setprecision(2)
            << lambda << "\n"
            << "Total de hits: " << m.hashHits << "\n"
            << "Total de misses: " << m.hashMisses << "\n"
            << "Taxa de acerto: " << std::setprecision(1) << taxa << "%\n"
            << "Total de colisoes: " << m.hashColisoes << "\n";
    }

    if (m.ordAcessos > 0) {
        out << "-- Ordenacao do historico (ao acessar Historico) --\n"
            << "Tamanho: " << m.ordTamanho << " entradas\n"
            << "Acessos ao Historico: " << m.ordAcessos << "\n";
        if (m.ordTempoNome >= 0)
            out << algoritmo << " (por nome): " << std::setprecision(6)
                << m.ordTempoNome << " s\n";
        if (m.ordTempoPontos >= 0)
            out << algoritmo << " (por pontuacao): " << std::setprecision(6)
                << m.ordTempoPontos << " s\n";
    }

    out << "================================\n";
    return true;
}

} // export
