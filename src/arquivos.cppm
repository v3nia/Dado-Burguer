module;
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "timer.hpp"

export module arquivos;
import TAD_avl;

export{
    struct Resultado_Carregamento{
        bool sucesso = false;
        double tempo_carregamento = 0.0;
        int pares_ignorados = 0;
    };

    //PROTÓTIPOS DAS FUNÇÕES INTERNAS
    void remover_fim(std::string& str);
    bool converter_codigo(const std::string& str, int& codigo);
    bool receita_valida(const std::string& receita);

    //uso de IA (Claude) para gerar a função de carregamento de pedidos e conversão de código, com base na função de carregamento de pedidos fornecida.
    Resultado_Carregamento carregar_pedidos(const std::string& nome_arquivo, Avl_Pedidos& avl){
        Resultado_Carregamento resultado;

        std::ifstream arquivo(nome_arquivo);
        if (!arquivo.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << nome_arquivo << std::endl;
            return resultado;
        }

        Timer t;
        timer_iniciar(&t);

        std::string linha_cod, linha_receita;

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
}

void remover_fim(std::string& str) {
    while (!str.empty() && (str.back() == '\r' || str.back() == ' ' || str.back() == '\t' || str.back() == '\n')) {
        str.pop_back();
    }
}

bool converter_codigo(const std::string& str, int& codigo) {
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

bool receita_valida(const std::string& receita) {
    return receita.size() >= 5 && receita.size() <= 7;
}