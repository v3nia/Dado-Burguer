module;
#include <fstream>
#include <iomanip>
#include <iostream>

export module metricas;

//uso de IA (Claude) para gerar a struct de metricas e fazer as funcoes de relatorio

export {

    struct Metricas {
        //metricas da avl -- ao carregar pedidos
        long long avl_nos          = 0;
        int       avl_altura       = 0;      //árvore vazia = 0, só raiz = 1
        long long avl_rotacoes     = 0;      
        double    avl_tempo_carga   = 0.0;    

        //metricas do historico
        int       hist_entradas    = 0;

        //metricas da avl -- depois da partida
        long long avl_buscas       = 0;
        long long avl_comparacoes  = 0;
        double    avl_tempo_buscas  = 0.0;    //acumulado, em segundos

        //metricas da tabela hash
        int       hash_m           = 0;      //tamanho da tabela
        long long hash_n           = 0;      //entradas ocupadas
        long long hash_hits        = 0;
        long long hash_miss        = 0;
        long long hash_colisoes    = 0;
        double    hash_taxa_acerto = 0.0;    //em porcentagem (0.0 a 100.0)

        //ordena o historico
        int       ord_tamanho      = 0;
        int       ord_acessos      = 0;
        double    ord_tempo_nome    = -1.0;   //-1 = ainda não medido
        double    ord_tempo_pontos  = -1.0;

        //controle das metricas pra partida e log
        bool      carregou        = false;  //true ao fim do carregamento
        bool      partida_terminou = false;  //true ao encerrar a partida
    };

//instância global compartilhada por todos os módulos.
Metricas g_metricas;

//zera as metricas da partida
void resetar_metricas_partida(Metricas& m) {
    m.avl_buscas       = 0;
    m.avl_comparacoes  = 0;
    m.avl_tempo_buscas  = 0.0;
    m.hash_n           = 0;
    m.hash_hits        = 0;
    m.hash_miss        = 0;
    m.hash_colisoes    = 0;
    m.hash_taxa_acerto = 0.0;
    m.partida_terminou = false;
}

void escrever_relatorio(std::ostream& out, const Metricas& m, const char* algoritmo){

    auto flags = out.flags();
    auto prec  = out.precision();

    out << std::fixed;
    out << "=== RELATORIO DE DESEMPENHO ===\n";

    if (m.carregou) {
        out << "-- AVL: carregamento --\n"
            << "Nos inseridos: " << m.avl_nos << "\n"
            << "Altura resultante: " << m.avl_altura << "\n"
            << "Rotacoes (insercao): " << m.avl_rotacoes << "\n"
            << "Tempo de carregamento: " << std::setprecision(6)
            << m.avl_tempo_carga << " s\n"
            << "-- Historico: carregamento --\n"
            << "Entradas carregadas: " << m.hist_entradas << "\n";
    }

    if (m.partida_terminou) {
        double media = 0.0;


        if (m.avl_buscas > 0) {
            media = static_cast<double>(m.avl_comparacoes) / m.avl_buscas;
        } else {
            media = 0.0;
        }

        double tempo_medio_busca = 0.0;
        if (m.avl_buscas > 0) {
            tempo_medio_busca = m.avl_tempo_buscas / static_cast<double>(m.avl_buscas);
        } else {
            tempo_medio_busca = 0.0;
        }

        double lambda = 0.0;
        if (m.hash_m > 0) {
            lambda = static_cast<double>(m.hash_n) / m.hash_m;
        } else {
            lambda = 0.0;
        }

        out << "-- AVL: buscas durante a partida --\n"
            << "Total de buscas: " << m.avl_buscas << "\n"
            << "Total de comparacoes: " << m.avl_comparacoes << "\n"
            << "Media de comparacoes por busca: " << std::setprecision(2)
            << media << "\n"
            << "Tempo medio por busca: " << std::setprecision(6)
            << tempo_medio_busca << " s\n"
            << "Tempo total acumulado: " << std::setprecision(6)
            << m.avl_tempo_buscas << " s\n"
            << "-- Tabela hash --\n"
            << "Tamanho (m): " << m.hash_m << "\n"
            << "Entradas ocupadas (n): " << m.hash_n << "\n"
            << "Fator de carga (lambda): " << std::setprecision(4)
            << lambda << "\n"
            << "Total de hits: " << m.hash_hits << "\n"
            << "Total de misses: " << m.hash_miss << "\n"
            << "Taxa de acerto: " << std::setprecision(1)
            << m.hash_taxa_acerto << "%\n"
            << "Total de colisoes: " << m.hash_colisoes << "\n";
    }

    if (m.ord_acessos > 0) {
        out << "-- Ordenacao do historico (ao acessar Historico) --\n"
            << "Tamanho: " << m.ord_tamanho << " entradas\n"
            << "Acessos ao Historico: " << m.ord_acessos << "\n";
        if (m.ord_tempo_nome >= 0) {
            out << algoritmo << " (por nome): " << std::setprecision(6)
                << m.ord_tempo_nome << " s\n";
        }
        if (m.ord_tempo_pontos >= 0) {
            out << algoritmo << " (por pontuacao): " << std::setprecision(6)
                << m.ord_tempo_pontos << " s\n";
        }
    }

    out.flags(flags);
    out.precision(prec);

    out << "================================\n";
}

//cria/sobreescreve o medicoes.log
bool gravar_log(const Metricas& m, const char* algoritmo = "Merge Sort") {
    std::ofstream out("medicoes.log");
    if (!out.is_open()) {
        return false;
    }

    escrever_relatorio(out, m, algoritmo);    
    return true;
}

//mostra o relatorio na tela depois de cada partida
void mostrar_relatorio(const Metricas& m, const char* algoritmo = "Merge Sort") {
    escrever_relatorio(std::cout, m, algoritmo);
}

} // export