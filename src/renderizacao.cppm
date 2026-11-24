/* Modulo de interface TUI, leitura de sprites e controle de terminal */

module;

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <poll.h>
#endif

#include <fstream>
#include <iostream>
#include <string>

export module renderizacao;

using std::string;

/* ==================== STRUCTS EXPORTADAS ==================== */

export struct Frame {
    static constexpr int MAX_LINHAS = 30;
    string linhas[MAX_LINHAS];
    int altura = 0;
    int largura = 0;
};

export struct Animacao {
    static constexpr int MAX_FRAMES = 20;
    Frame frames[MAX_FRAMES];
    int totalFrames = 0;
};

export struct PacoteSprites {
    Animacao chapa;
    Animacao clienteCalmo;
    Animacao clienteBravo;
    Animacao logo;
};

export struct DadosTela {
    int turnoAtual        = 1;
    int pontuacao         = 0;
    int clientesAtendidos = 0;
    int clientesNaFila    = 0;

    int pacienciaAtual    = 0;
    int pacienciaMax      = 22;

    string textoReceita;   // ordem topo -> base
    string textoMontagem;  // ordem topo -> base

    // Perecíveis: proxVenc = turnos restantes para o próximo estragar (-1 se vazio)
    int carneQtd      = 0;
    int carneProxVenc = -1;
    int ovoQtd        = 0;
    int ovoProxVenc   = -1;
    int baconQtd      = 0;
    int baconProxVenc = -1;

    // Perenes
    int queijoSuico   = 0;
    int queijoCheddar = 0;
    int cebola        = 0;
    int tomate        = 0;

    string feedback;
};

/* ==================== CONTROLE DO TERMINAL ==================== */

#ifndef _WIN32
static termios g_termiosOrig;
static bool g_rawAtivo = false;
#endif

export void ativarTerminal() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (GetConsoleMode(hOut, &modo)) {
        SetConsoleMode(hOut, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    if (!g_rawAtivo) {
        tcgetattr(STDIN_FILENO, &g_termiosOrig);
        termios raw = g_termiosOrig;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        tcflush(STDIN_FILENO, TCIFLUSH);
        g_rawAtivo = true;
    }
#endif
    std::cout << "\033[?25l\033[2J\033[H" << std::flush;
}

export void desativarTerminal() {
#ifndef _WIN32
    if (g_rawAtivo) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_termiosOrig);
        g_rawAtivo = false;
    }
#endif
    std::cout << "\033[?25h\033[0m" << std::flush;
}

export char lerTecla(int ms) {
#ifdef _WIN32
    if (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 224) {
            if (_kbhit()) (void)_getch(); // ignora teclas de seta/funcao no Windows
            return '\0';
        }
        return static_cast<char>(c);
    }
    Sleep(ms);
    return '\0';
#else
    pollfd pfd{STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, ms) > 0) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == 1) {
            // Se vier ESC seguido de outros bytes (setas/Home/Delete), descarta a sequência
            if (c == 27) {
                if (poll(&pfd, 1, 5) > 0) {
                    char lixo = '\0';
                    while (poll(&pfd, 1, 2) > 0) {
                        if (read(STDIN_FILENO, &lixo, 1) != 1) break;
                    }
                    return '\0';
                }
            }
            return c;
        }
    }
    return '\0';
#endif
}

/* ==================== FUNÇÕES AUXILIARES INTERNAS ==================== */

int larguraVisual(const string& s) {
    int colunas = 0;
    bool emAnsi = false;

    for (unsigned char c : s) {
        if (c == '\033') {
            emAnsi = true;
            continue;
        }
        if (emAnsi) {
            if (c == 'm') emAnsi = false;
            continue;
        }
        if ((c & 0xC0) != 0x80) {
            colunas++;
        }
    }
    return colunas;
}

string ajustarColuna(const string& linha, int larguraAlvo) {
    int faltam = larguraAlvo - larguraVisual(linha);
    string out = linha + "\033[0m";
    if (faltam > 0) {
        out.append(static_cast<size_t>(faltam), ' ');
    }
    return out;
}

string decodificarAnsi(const string& linha) {
    string out;
    out.reserve(linha.size());
    for (size_t i = 0; i < linha.size(); ++i) {
        if (i + 3 < linha.size() && linha.compare(i, 4, "\\033") == 0) {
            out += '\033';
            i += 3;
        } else {
            out += linha[i];
        }
    }
    return out;
}

Frame juntarLadoALado(const Frame& esq, const Frame& dir, int espacoEntre = 3) {
    Frame resultado;
    resultado.altura = (esq.altura > dir.altura) ? esq.altura : dir.altura;
    if (resultado.altura > Frame::MAX_LINHAS) {
        resultado.altura = Frame::MAX_LINHAS;
    }
    resultado.largura = esq.largura + espacoEntre + dir.largura;

    string separador(static_cast<size_t>(espacoEntre), ' ');

    for (int i = 0; i < resultado.altura; ++i) {
        string lEsq = (i < esq.altura) ? esq.linhas[i] : "";
        string lDir = (i < dir.altura) ? dir.linhas[i] : "";

        resultado.linhas[i] = ajustarColuna(lEsq, esq.largura)
                            + separador
                            + ajustarColuna(lDir, dir.largura);
    }
    return resultado;
}

const Frame& pegarFrame(const Animacao& anim, int tickAnim) {
    static const Frame vazio;
    if (anim.totalFrames <= 0) return vazio;
    return anim.frames[tickAnim % anim.totalFrames];
}

// Todos os 12 ingredientes da Tabela 1 + pao base ('i'), sempre com 18 colunas visuais
string camadaIngrediente(char ing) {
    switch (ing) {
        case 'z': return "\033[38;5;215m  ╭────────────╮  \033[0m"; // Pao topo (z)
        case 'i': return "\033[38;5;215m  ╰────────────╯  \033[0m"; // Pao base (i)
        case 'k': return "\033[38;5;196m ~~~~~~~~~~~~~~~~ \033[0m";  // Ketchup (k)
        case 'm': return "\033[38;5;226m ~~~~~~~~~~~~~~~~ \033[0m";  // Mostarda (m)
        case 'p': return "\033[38;5;71m  ▄▄  ▄▄  ▄▄  ▄▄  \033[0m";  // Picles (p)
        case 'a': return "\033[38;5;46m ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ \033[0m";  // Alface (a)
        case 's': return "\033[38;5;229m ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ \033[0m"; // Queijo suico (s)
        case 'q': return "\033[38;5;208m ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ \033[0m"; // Queijo cheddar (q)
        case 'c': return "\033[38;5;183m ()()()()()()()() \033[0m"; // Cebola (c)
        case 't': return "\033[38;5;160m ──────────────── \033[0m"; // Tomate (t)
        case 'n': return "\033[38;5;94m ████████████████ \033[0m";  // Carne (n)
        case 'o': return "\033[38;5;255m ▄▄▄▄▄\033[38;5;226m██████\033[38;5;255m▄▄▄▄▄ \033[0m"; // Ovo (o)
        case 'b': return "\033[38;5;167m ≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈ \033[0m"; // Bacon (b)
        default:  return "      [ " + string(1, ing) + " ]       ";
    }
}

Frame montarFrameBurguer(const string& titulo, const string& ingredientes) {
    Frame f;
    f.altura = 16;
    f.largura = 18;

    f.linhas[0] = titulo;
    f.linhas[1] = "\033[38;5;240m──────────────────\033[0m";

    int qtd = static_cast<int>(ingredientes.size());
    if (qtd > 12) qtd = 12;

    int linhaInicio = 14 - qtd;
    for (int i = 0; i < qtd; ++i) {
        f.linhas[linhaInicio + i] = camadaIngrediente(ingredientes[static_cast<size_t>(i)]);
    }

    f.linhas[14] = camadaIngrediente('i');
    f.linhas[15] = "\033[38;5;245m ════════════════ \033[0m";
    return f;
}

string linhaItemChapa(const string& rotulo, int qtd, int proxVenc, const string& icone, const string& corNormal) {
    string cor = (qtd > 0 && proxVenc >= 0 && proxVenc <= 2) ? "\033[38;5;196m" : corNormal;
    string barra;
    int mostrar = (qtd > 8) ? 8 : qtd;

    for (int i = 0; i < 8; ++i) {
        if (i < mostrar) {
            barra += cor + icone + "\033[0m ";
        } else {
            barra += "\033[38;5;238m..\033[0m ";
        }
    }

    string info = (qtd > 0) ? (std::to_string(proxVenc) + "t") : "--";
    return "\033[38;5;240m│\033[0m " + rotulo + " " + barra + "\033[38;5;245m" + info + "\033[0m";
}

Frame montarFrameChapaDinamica(const DadosTela& dados, int tickAnim) {
    Frame f;
    f.altura = 16;
    f.largura = 36;

    bool temAlgo = (dados.carneQtd + dados.ovoQtd + dados.baconQtd) > 0;
    int fase = tickAnim % 4;

    string vapor[4] = {
        "      \033[38;5;250m(  )       (  )       (  )\033[0m    ",
        "       \033[38;5;246m)  (       )  (       )  (\033[0m   ",
        "      \033[38;5;242m(  )       (  )       (  )\033[0m    ",
        "     \033[38;5;246m)  (       )  (       )  (\033[0m     "
    };

    f.linhas[0] = "\033[1;33m         [ CHAPA AO VIVO ]          \033[0m";
    f.linhas[1] = temAlgo ? vapor[fase] : "";
    f.linhas[2] = temAlgo ? vapor[(fase + 2) % 4] : "";
    f.linhas[3] = "\033[38;5;240m╭──────────────────────────────────╮\033[0m";
    f.linhas[4] = "\033[38;5;240m│\033[38;5;245m  Grelha (ate 8 por ingrediente)  \033[38;5;240m│\033[0m";
    f.linhas[5] = "\033[38;5;240m├──────────────────────────────────┤\033[0m";

    f.linhas[6]  = ajustarColuna(linhaItemChapa("\033[1mN\033[0m", dados.carneQtd, dados.carneProxVenc, "██", "\033[38;5;94m"),  35) + "\033[38;5;240m│\033[0m";
    f.linhas[7]  = "\033[38;5;238m│ ──────────────────────────────── │\033[0m";
    f.linhas[8]  = ajustarColuna(linhaItemChapa("\033[1mO\033[0m", dados.ovoQtd,   dados.ovoProxVenc,   "()", "\033[38;5;226m"), 35) + "\033[38;5;240m│\033[0m";
    f.linhas[9]  = "\033[38;5;238m│ ──────────────────────────────── │\033[0m";
    f.linhas[10] = ajustarColuna(linhaItemChapa("\033[1mB\033[0m", dados.baconQtd, dados.baconProxVenc, "≈≈", "\033[38;5;167m"), 35) + "\033[38;5;240m│\033[0m";

    f.linhas[11] = "\033[38;5;240m╰──────────────────────────────────╯\033[0m";

    string brasa = (tickAnim % 2 == 0)
        ? "\033[38;5;196m  ^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^\033[0m"
        : "\033[38;5;208m  ^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^ \033[38;5;220m^ \033[38;5;196m^ \033[38;5;208m^\033[0m";
    f.linhas[12] = temAlgo ? brasa : "\033[38;5;52m  ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^\033[0m";
    f.linhas[13] = "\033[38;5;240m ══════════════════════════════════ \033[0m";
    f.linhas[14] = "\033[38;5;245m  [1] Carne   [2] Ovo   [3] Bacon   \033[0m";
    f.linhas[15] = "";

    return f;
}

string formatarPerecivel(const string& nome, int qtd, int proxVenc) {
    string s = nome + ": " + std::to_string(qtd) + "/8";
    if (qtd > 0 && proxVenc >= 0) {
        string cor = (proxVenc <= 2) ? "\033[38;5;196m" : "\033[38;5;220m";
        s += " (" + cor + "vence em " + std::to_string(proxVenc) + "t\033[0m)";
    } else {
        s += " (\033[38;5;240mvazio\033[0m)";
    }
    return s;
}

bool carregarAnimacao(const string& caminho, Animacao& anim) {
    std::ifstream arq(caminho);
    if (!arq.is_open()) return false;

    anim.totalFrames = 0;
    bool lendoFrame = false;
    string linha;

    while (std::getline(arq, linha)) {
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();

        if (linha == "@FRAME") {
            if (anim.totalFrames >= Animacao::MAX_FRAMES) break;
            lendoFrame = true;
            anim.frames[anim.totalFrames] = Frame{};
            continue;
        }
        if (linha == "@END") {
            if (lendoFrame) {
                anim.totalFrames++;
                lendoFrame = false;
            }
            continue;
        }

        if (lendoFrame) {
            Frame& f = anim.frames[anim.totalFrames];
            if (f.altura < Frame::MAX_LINHAS) {
                string dec = decodificarAnsi(linha);
                int w = larguraVisual(dec);
                if (w > f.largura) f.largura = w;
                f.linhas[f.altura++] = dec;
            }
        }
    }
    return anim.totalFrames > 0;
}

bool carregarComFallback(const string& nomeArquivo, Animacao& anim) {
    if (carregarAnimacao("assets/" + nomeArquivo, anim)) return true;
    return carregarAnimacao("../assets/" + nomeArquivo, anim);
}

/* ==================== CARREGAMENTO E RENDERIZAÇÃO EXPORTADOS ==================== */

export bool carregarTodosSprites(PacoteSprites& pacote) {
    bool ok1 = carregarComFallback("chapa.txt", pacote.chapa);
    bool ok2 = carregarComFallback("cliente_calmo.txt", pacote.clienteCalmo);
    bool ok3 = carregarComFallback("cliente_bravo.txt", pacote.clienteBravo);
    bool ok4 = carregarComFallback("logo.txt", pacote.logo);
    return ok1 && ok2 && ok3 && ok4;
}

export void renderizarLogo(const PacoteSprites& sprites, int tickAnim) {
    const Frame& f = pegarFrame(sprites.logo, tickAnim);
    string tela = "\033[H";

    for (int i = 0; i < f.altura; ++i) {
        tela += f.linhas[i] + "\033[0m\033[K\n";
    }
    tela += "\033[K\n";
    tela += "  \033[1;33m[1]\033[0m Novo Jogo   \033[1;33m[2]\033[0m Como Jogar   \033[1;33m[3]\033[0m Historico   \033[1;33m[0]\033[0m Sair\033[K\n";
    tela += "\033[J";

    std::cout << tela << std::flush;
}

export void renderizarGameplay(const DadosTela& dados,
                               const PacoteSprites& sprites,
                               int tickAnim) {
    bool estaBravo = (dados.pacienciaMax > 0) &&
                     (dados.pacienciaAtual * 10 <= dados.pacienciaMax * 4);

    const Frame& clienteFrame = estaBravo
        ? pegarFrame(sprites.clienteBravo, tickAnim)
        : pegarFrame(sprites.clienteCalmo, tickAnim);
    Frame chapaFrame = montarFrameChapaDinamica(dados, tickAnim);

    Frame pedidoFrame   = montarFrameBurguer("\033[1;36m    [ PEDIDO ]    \033[0m", dados.textoReceita);
    Frame montagemFrame = montarFrameBurguer("\033[1;32m   [ MONTAGEM ]   \033[0m", dados.textoMontagem);

    Frame centro = juntarLadoALado(pedidoFrame, montagemFrame, 2);
    Frame painelPrincipal = juntarLadoALado(juntarLadoALado(clienteFrame, centro, 3), chapaFrame, 3);

    string corPaciencia = estaBravo ? "\033[1;31m" : "\033[1;32m";

    string tela = "\033[H";

    tela += "\033[1;33m=== DADO-BURGUER ===\033[0m   "
            "Turno: \033[1m" + std::to_string(dados.turnoAtual) + "\033[0m"
            "  |  Atendidos (Pts): \033[1;32m" + std::to_string(dados.pontuacao) + "\033[0m"
            "  |  Restantes na Fila: " + std::to_string(dados.clientesNaFila) +
            "  |  Paciencia: " + corPaciencia + std::to_string(dados.pacienciaAtual) + "\033[0m\033[K\n";
    tela += "\033[38;5;240m" + string(116, '=') + "\033[0m\033[K\n";

    for (int i = 0; i < painelPrincipal.altura; ++i) {
        tela += painelPrincipal.linhas[i] + "\033[0m\033[K\n";
    }

    tela += "\033[38;5;240m" + string(116, '=') + "\033[0m\033[K\n";
    tela += " \033[1mPereciveis:\033[0m  "
            + formatarPerecivel("[1/n] Carne", dados.carneQtd, dados.carneProxVenc) + "  |  "
            + formatarPerecivel("[2/o] Ovo", dados.ovoQtd, dados.ovoProxVenc) + "  |  "
            + formatarPerecivel("[3/b] Bacon", dados.baconQtd, dados.baconProxVenc) + "\033[K\n";
    tela += " \033[1mPerenes:   \033[0m  "
            "[4/s] Suico: " + std::to_string(dados.queijoSuico) +
            "  |  [5/q] Cheddar: " + std::to_string(dados.queijoCheddar) +
            "  |  [6/c] Cebola: " + std::to_string(dados.cebola) +
            "  |  [7/t] Tomate: " + std::to_string(dados.tomate) + "\033[K\n";
    tela += " \033[1mInfinitos: \033[0m  "
            "[k] Ketchup  |  [m] Mostarda  |  [p] Picles  |  [a] Alface  |  [z] Pao Topo (fecha e entrega)\033[K\n";
    tela += "\033[38;5;240m" + string(116, '-') + "\033[0m\033[K\n";
    tela += " \033[1;33mStatus:\033[0m " + dados.feedback + "\033[K\n";
    tela += " \033[38;5;245m[1-7] Preparar | [Letra] Empilhar | [Backspace] Desfazer topo | [0/ESC] Sair\033[0m\033[K\n";
    tela += "\033[J";

    std::cout << tela << std::flush;
}