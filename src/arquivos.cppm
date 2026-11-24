module;
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>
#include "timer.hpp"

export module arquivos;
import TAD_avl;
import TAD_lista_contigua;

using namespace std;
using std::string;

//uso de IA (Claude) para gerar a função de carregamento de pedidos e conversão de código, com base na função de carregamento de pedidos fornecida.

export{
    /*-------- struct e funções da AVL -----------*/
    struct Resultado_Carregamento{
        bool sucesso = false;
        double tempo_carregamento = 0.0;
        int pares_ignorados = 0;
    };

    //PROTÓTIPOS DAS FUNÇÕES INTERNAS
    void remover_fim(string& str);
    bool converter_codigo(const string& str, int& codigo);
    bool receita_valida(const string& receita);

    Resultado_Carregamento carregar_pedidos(const string& nome_arquivo, Avl_Pedidos& avl){
        Resultado_Carregamento resultado;

        std::ifstream arquivo(nome_arquivo);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << nome_arquivo << endl;
            return resultado;
        }

        Timer t;
        timer_iniciar(&t);

        string linha_cod, linha_receita;

        while (std::getline(arquivo, linha_cod)){
            remover_fim(linha_cod);
            
            if(linha_cod.empty()) continue;

            if (!std::getline(arquivo, linha_receita)) {
                resultado.pares_ignorados++;
                break;
            }
            remover_fim(linha_receita);

            int codigo;
            
            if (!converter_codigo(linha_cod, codigo) || !receita_valida(linha_receita)) {
                resultado.pares_ignorados++;
                continue;
            }
            
            avl.raiz = inserir_pedido(avl.raiz, {codigo, linha_receita, nullptr, nullptr, 0}, avl.total_insercao, avl.total_rotacao, avl.total_comparacao, avl.menor_cod, avl.maior_cod);
        }

        resultado.tempo_carregamento = timer_parar(&t);
        resultado.sucesso = true;
        return resultado;

    }


    /*------------ struct e funções da Lista Contígua -----------*/
    struct ResultadoHistorico {
        bool ok = false;        // false se o arquivo existe mas nao pode ser aberto
        long entradas = 0;      // entradas carregadas na lista
        long ignoradas = 0;     // pares invalidos que foram pulados
    } ;

    ResultadoHistorico carregar_historico(const string& caminho, ListaContigua_Historico& lista) {
        ResultadoHistorico res;

        std::error_code ec;
        if (!std::filesystem::exists(caminho, ec)) {
            res.ok = true;      // primeira execucao: historico comeca vazio
            return res;
        }

        std::ifstream arq(caminho);
        if (!arq.is_open()) {
            std::cerr << "Erro: nao foi possivel abrir o historico '" << caminho << "'\n";
            return res;
        }

        string linha_nome, linha_pontos;
        while (std::getline(arq, linha_nome)) {
            remover_fim(linha_nome);
            if (linha_nome.empty()) continue;            // linha em branco

            if (!std::getline(arq, linha_pontos)) {      // nome sem pontuacao no fim
                res.ignoradas++;
                break;
            }
            remover_fim(linha_pontos);

            int pontuacao = 0;
            if (!converter_codigo(linha_pontos, pontuacao) || pontuacao < 0) {
                res.ignoradas++;
                continue;
            }

            NodoListaContigua novo{linha_nome, pontuacao};
            if (!inserir_listacontigua(lista, novo, tamanho_listacontigua(lista))) {
                res.ignoradas++;
                continue;
            }
            res.entradas++;
        }

        res.ok = true;
        return res;
    }

    bool anexar_historico(const std::string& caminho, const std::string& nome, int pontuacao) {
        std::string limpo;
        for (char c : nome) {
            if (c != '\r' && c != '\n') limpo += c;
        }
        remover_fim(limpo);
        if (limpo.empty() || pontuacao < 0) return false;

        bool precisa_quebra = false;
        {
            std::ifstream leitura(caminho, std::ios::binary | std::ios::ate);
            if (leitura.is_open() && leitura.tellg() > 0) {
                leitura.seekg(-1, std::ios::end);
                char ultimo = 0;
                leitura.get(ultimo);
                precisa_quebra = (ultimo != '\n');
            }
        }

        std::ofstream saida(caminho, std::ios::app);
        if (!saida.is_open()) {
            std::cerr << "Erro: nao foi possivel gravar no historico '" << caminho << "'\n";
            return false;
        }
        if (precisa_quebra) saida << '\n';
        saida << limpo << '\n' << pontuacao << '\n';
        return saida.good();
    }
}

void remover_fim(string& str) {
    while (!str.empty() && (str.back() == '\r' || str.back() == ' ' || str.back() == '\t' || str.back() == '\n')) {
        str.pop_back();
    }
}

bool converter_codigo(const string& str, int& codigo) {
    try {
        size_t pos;
        codigo = std::stoi(str, &pos);

        if (pos != str.length()) {
            return false; //caracteres não numéricos na string
        }

    } catch (const std::invalid_argument&) {
        return false; //a string não é um número válido

    } catch (const std::out_of_range&) {
        return false; //número fora do intervalo representável por um int

    }
    return true;
}

bool receita_valida(const string& receita) {
    return receita.size() >= 5 && receita.size() <= 7;
}