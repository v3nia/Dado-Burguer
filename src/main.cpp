// Arquivo principal do projeto
#include <iostream>

//TEMPORARIO
import TAD_avl;
import arquivos;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <pedidos>\n";
        return 1;
    }

    Avl_Pedidos avl;
    Resultado_Carregamento res = carregar_pedidos(argv[1], avl);

    if (!res.sucesso) return 1;

    std::cout << "Leitura OK\n";
    std::cout << "Tempo de carga: " << res.tempo_carregamento << " s\n";
    std::cout << "Pares ignorados: " << res.pares_ignorados << "\n";

    std::cout << "Total de insercoes: " << avl.total_insercao << "\n";
    std::cout << "Altura da arvore: " << avl_altura(avl.raiz) << "\n";
    std::cout << "Total de rotacoes: " << avl.total_rotacao << "\n";
    std::cout << "Total de comparacoes: " << avl.total_comparacao << "\n";
    std::cout << "Menor codigo: " << avl.menor_cod << "\n";
    std::cout << "Maior codigo: " << avl.maior_cod << "\n";

    return 0;
}