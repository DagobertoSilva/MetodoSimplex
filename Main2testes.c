#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#define TAM_ENTRADA 1024
#define TAM_SAIDA 4096
#define TAM_SUBSTITUTO 50

/*
    Substitui todas as ocorrencias de um operador por um texto numerado.
    Exemplo: "<=" -> " + F1 = " e ">=" -> " - E1 = ".
    Retorna 1 em caso de sucesso e 0 se ocorrer erro/faltar espaco.
*/
int substituirTodas(const char entrada[],const char procurar[],const char formatoSubstituto[],char saida[], size_t capacidade, int *contador) {
    if (procurar[0] == '\0' || capacidade == 0) {
        return 0;
    }

    const char *atual = entrada;
    size_t usados = 0;
    size_t tamanhoProcurar = strlen(procurar);
    saida[0] = '\0';

    while (*atual != '\0') {
        const char *posicao = strstr(atual, procurar);

        if (posicao == NULL) {
            size_t restante = strlen(atual);
            if (restante >= capacidade - usados) {
                return 0;
            }
            memcpy(saida + usados, atual, restante);
            usados += restante;
            saida[usados] = '\0';
            return 1;
        }

        size_t trecho = (size_t)(posicao - atual);
        if (trecho >= capacidade - usados) {
            return 0;
        }
        memcpy(saida + usados, atual, trecho);
        usados += trecho;
        saida[usados] = '\0';

        char substituto[TAM_SUBSTITUTO];
        int tamanhoSubstituto = snprintf(
            substituto,
            sizeof(substituto),
            formatoSubstituto,
            *contador
        );

        if (tamanhoSubstituto < 0 ||
            (size_t)tamanhoSubstituto >= sizeof(substituto)) {
            return 0;
        }

        if ((size_t)tamanhoSubstituto >= capacidade - usados) {
            return 0;
        }
        memcpy(saida + usados, substituto, (size_t)tamanhoSubstituto);
        usados += (size_t)tamanhoSubstituto;
        saida[usados] = '\0';

        (*contador)++;
        atual = posicao + tamanhoProcurar;
    }

    return 1;
}

/*
    Identifica linhas de nao negatividade como "x1 >= 0".
    Essas linhas devem permanecer inalteradas; caso contrario, seriam
    incorretamente transformadas em "x1 - E1 = 0".
*/
int ehRestricaoNaoNegatividade(const char linha[]) {
    const char *inicio = linha;

    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }

    return inicio[0] == 'x' &&
           inicio[1] >= '0' && inicio[1] <= '9';
}

/*
    Identifica a linha da funcao objetivo (comeca com "z").
*/
int ehFuncaoObjetivo(const char linha[]) {
    const char *inicio = linha;

    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }

    return inicio[0] == 'z';
}

/*
    Escreve a funcao objetivo na forma padrao:
    z = 3x1 + 2x2   ->   z - 3.00x1 - 2.00x2 - 0.00F1 - 0.00E1 = 0
    Ou seja, as variaveis vao para o lado do z com o sinal trocado
    e o lado direito fica igual a zero.
    As variaveis de folga (F) e de excesso (E) nao aparecem na funcao
    objetivo original, entao entram com valor 0 (e sinal trocado).
    Retorna 1 em caso de sucesso e 0 em caso de erro de escrita.
*/
int escreverFuncaoObjetivoPadrao(
    FILE *arquivo,
    const double coeficientes[],
    int quantidade,
    int quantidadeF,
    int quantidadeE
) {
    int k;

    if (fprintf(arquivo, "z") < 0) {
        return 0;
    }

    for (k = 0; k < quantidade; k++) {
        double invertido = -coeficientes[k];
        int resultado;

        if (invertido >= 0) {
            resultado = fprintf(arquivo, " + %.2lfx%d", invertido, k + 1);
        } else {
            resultado = fprintf(arquivo, " - %.2lfx%d", -invertido, k + 1);
        }

        if (resultado < 0) {
            return 0;
        }
    }

    // Variaveis de folga: - 0.00F1 - 0.00F2 ...
    for (k = 1; k <= quantidadeF; k++) {
        if (fprintf(arquivo, " - %.2lfF%d", 0.0, k) < 0) {
            return 0;
        }
    }

    // Variaveis de excesso: - 0.00E1 - 0.00E2 ...
    for (k = 1; k <= quantidadeE; k++) {
        if (fprintf(arquivo, " - %.2lfE%d", 0.0, k) < 0) {
            return 0;
        }
    }

    if (fprintf(arquivo, " = 0\n") < 0) {
        return 0;
    }

    return 1;
}

/*
    Le uma linha da forma padrao e preenche 'destino' (colunas + 1 posicoes).
    Colunas: x1..xn | F1..Fnf | E1..Ene | termo independente (ultima).
    Exemplos de linhas aceitas:
      z - 3.00x1 - 2.00x2 - 0.00F1 = 0
      1.00x1 + 2.00x2 + F1 = 10.00
      3.00x1 + 1.00x2 - E1 = 5.00
*/
void lerLinhaForma(
    const char linha[],
    double destino[],
    int n,
    int nf,
    int ne
) {
    const char *p = linha;
    int colunas = n + nf + ne;
    double sinal = 1.0;

    while (*p != '\0') {

        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            p++;
        } else if (*p == '+') {
            sinal = 1.0;
            p++;
        } else if (*p == '-') {
            sinal = -1.0;
            p++;
        } else if (*p == '=') {
            // Termo independente (lado direito)
            destino[colunas] = strtod(p + 1, NULL);
            return;
        } else {
            double valor = 1.0;
            char numero[64];
            int tamanho = 0;
            char tipo;
            int indice = 0;
            int coluna = -1;

            // Coeficiente numerico (se existir). Lido manualmente para
            // nao confundir "0.00E1" com notacao cientifica.
            if ((*p >= '0' && *p <= '9') || *p == '.') {
                while (((*p >= '0' && *p <= '9') || *p == '.') &&
                       tamanho < (int)sizeof(numero) - 1) {
                    numero[tamanho++] = *p;
                    p++;
                }
                numero[tamanho] = '\0';
                valor = atof(numero);
            }

            tipo = *p;
            if (tipo == '\0') {
                break;
            }
            p++;

            // Indice da variavel (x1, F2, E3...)
            while (*p >= '0' && *p <= '9') {
                indice = indice * 10 + (*p - '0');
                p++;
            }

            if (tipo == 'x' && indice >= 1 && indice <= n) {
                coluna = indice - 1;
            } else if (tipo == 'F' && indice >= 1 && indice <= nf) {
                coluna = n + indice - 1;
            } else if (tipo == 'E' && indice >= 1 && indice <= ne) {
                coluna = n + nf + indice - 1;
            }

            if (coluna >= 0) {
                destino[coluna] += sinal * valor;
            }

            sinal = 1.0;
        }
    }
}

/*
    Escreve o nome da coluna (x1, F1, E1...) em 'nome'.
*/
void nomeColuna(char nome[], size_t tamanho, int coluna, int n, int nf) {
    if (coluna < n) {
        snprintf(nome, tamanho, "x%d", coluna + 1);
    } else if (coluna < n + nf) {
        snprintf(nome, tamanho, "F%d", coluna - n + 1);
    } else {
        snprintf(nome, tamanho, "E%d", coluna - n - nf + 1);
    }
}

/*
    Imprime a tabela (em arquivo ou na tela) no formato:

    Base   |         z |        x1 |        x2 |        F1 | ... |         b
    -------+-----------+-----------+-----------+-----------+-----+----------
    z      |      1.00 |     -4.00 |     -5.00 |      0.00 | ... |      0.00
    F1     |      0.00 |      1.00 |      1.00 |      1.00 | ... |     15.00
*/
void imprimirTabela(
    FILE *saida,
    const double tabela[],
    int linhasTabela,
    int n,
    int nf,
    int ne
) {
    int colunas = n + nf + ne;
    int largura = colunas + 1;
    int i, j, k;
    char nome[32];

    fprintf(saida, "%-6s", "Base");
    fprintf(saida, " | %9s", "z");   // coluna do coeficiente de z
    for (j = 0; j < colunas; j++) {
        nomeColuna(nome, sizeof(nome), j, n, nf);
        fprintf(saida, " | %9s", nome);
    }
    fprintf(saida, " | %9s", "b");   // b e a ultima coluna
    fprintf(saida, "\n");

    fprintf(saida, "-------");
    for (j = 0; j <= colunas + 1; j++) {
        fprintf(saida, "+-----------");
    }
    fprintf(saida, "\n");

    for (i = 0; i < linhasTabela; i++) {
        double valor;

        // Nome da variavel basica da linha
        if (i == 0) {
            snprintf(nome, sizeof(nome), "z");
        } else {
            snprintf(nome, sizeof(nome), "-");
            for (k = 0; k < nf; k++) {
                valor = tabela[i * largura + n + k];
                if (valor > 0.999999 && valor < 1.000001) {
                    snprintf(nome, sizeof(nome), "F%d", k + 1);
                    break;
                }
            }
        }

        fprintf(saida, "%-6s", nome);

        // Coluna z: coeficiente 1 na linha da funcao objetivo e 0 nas
        // linhas das restricoes (z nao aparece nas restricoes).
        fprintf(saida, " | %9.2lf", (i == 0) ? 1.0 : 0.0);

        for (j = 0; j < colunas; j++) {
            valor = tabela[i * largura + j];
            if (valor == 0.0) valor = 0.0;   // evita "-0.00"
            fprintf(saida, " | %9.2lf", valor);
        }

        // Termo independente (b) como ultima coluna
        valor = tabela[i * largura + colunas];
        if (valor == 0.0) valor = 0.0;
        fprintf(saida, " | %9.2lf", valor);
        fprintf(saida, "\n");
    }
}

/* ============================================================
   METODO SIMPLEX EM DUAS FASES
   ============================================================
   O problema e tratado como MAXIMIZACAO (Z = c1x1 + ... + cnxn),
   com a linha do tableau na forma  z - c1x1 - ... - cnxn = 0.
   A variavel que entra e a de coeficiente mais negativo nessa linha
   (empate: menor indice) e o criterio de parada e "nenhum negativo".
   Na Fase I maximiza-se -w (w = soma das artificiais), o que equivale
   a minimizar w, e a linha do tableau mostra os custos reduzidos c-z.
   ============================================================ */

#define EPS 1e-9
#define TOL_INVIAVEL 1e-6
#define MAX_ITERACOES 10000

typedef struct {
    int n, nf, ne, na;   /* decisao, folga, excesso, artificiais        */
    int m;               /* numero de restricoes (linhas 1..m)          */
    int ncols;           /* colunas de variaveis (n + nf + ne + na)     */
    double *t;           /* tableau (m+1) x (ncols+1); ultima col. = b  */
    int *base;           /* base[i] = coluna basica da linha i          */
    int *ativa;          /* 0 = coluna removida (artificiais na Fase II)*/
    int iteracoes[3];    /* [1] = Fase I, [2] = Fase II                 */
} Simplex;

#define CEL(s, i, j) ((s)->t[(size_t)(i) * ((size_t)(s)->ncols + 1) + (size_t)(j)])

/* Escreve na tela e, se existir, no arquivo de solucao. */
void emitir(FILE *log, const char *formato, ...) {
    va_list args;

    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);

    if (log != NULL) {
        va_start(args, formato);
        vfprintf(log, formato, args);
        va_end(args);
    }
}

double limpar(double v) {
    return (fabs(v) < 1e-12) ? 0.0 : v;   /* evita "-0.000" */
}

/* Nome da coluna: x, F (folga), E (excesso) ou A (artificial). */
void nomeVariavel(const Simplex *s, int col, char nome[], size_t tam) {
    if (col < s->n) {
        snprintf(nome, tam, "x%d", col + 1);
    } else if (col < s->n + s->nf) {
        snprintf(nome, tam, "F%d", col - s->n + 1);
    } else if (col < s->n + s->nf + s->ne) {
        snprintf(nome, tam, "E%d", col - s->n - s->nf + 1);
    } else {
        snprintf(nome, tam, "A%d", col - s->n - s->nf - s->ne + 1);
    }
}

/*
    Imprime o tableau no formato:
    Base | z (ou w) | colunas das variaveis | b | b/col (se houver razao)
*/
void imprimirTableau(FILE *f, const Simplex *s, int fase, const double razao[]) {
    char nome[32];
    const char *rotulo = (fase == 1) ? "w" : "z";
    int i, j, k, mostradas = 0, total;

    fprintf(f, "%-6s | %9s", "Base", rotulo);
    for (j = 0; j < s->ncols; j++) {
        if (!s->ativa[j]) continue;
        nomeVariavel(s, j, nome, sizeof(nome));
        fprintf(f, " | %9s", nome);
        mostradas++;
    }
    fprintf(f, " | %9s", "b");
    if (razao != NULL) fprintf(f, " | %9s", "b/col");
    fprintf(f, "\n");

    total = 1 + mostradas + 1 + ((razao != NULL) ? 1 : 0);
    fprintf(f, "-------");
    for (k = 0; k < total; k++) fprintf(f, "+-----------");
    fprintf(f, "\n");

    for (i = 0; i <= s->m; i++) {
        double b;

        if (i == 0) {
            snprintf(nome, sizeof(nome), "%s", rotulo);
        } else {
            nomeVariavel(s, s->base[i], nome, sizeof(nome));
        }

        fprintf(f, "%-6s | %9.3lf", nome, (i == 0) ? 1.0 : 0.0);

        for (j = 0; j < s->ncols; j++) {
            if (!s->ativa[j]) continue;
            fprintf(f, " | %9.3lf", limpar(CEL(s, i, j)));
        }

        b = CEL(s, i, s->ncols);
        if (i == 0 && fase == 1) b = -b;      /* mostra w, nao -w */
        fprintf(f, " | %9.3lf", limpar(b));

        if (razao != NULL) {
            if (i > 0 && razao[i] >= 0) fprintf(f, " | %9.3lf", razao[i]);
            else                        fprintf(f, " | %9s", "-");
        }
        fprintf(f, "\n");
    }
}

void emitirTableau(FILE *log, const Simplex *s, int fase, const double razao[]) {
    imprimirTableau(stdout, s, fase, razao);
    if (log != NULL) imprimirTableau(log, s, fase, razao);
}

/* Pivoteamento: normaliza a linha pivo e zera a coluna nas demais. */
void pivotear(Simplex *s, int lin, int col) {
    int W = s->ncols + 1;
    int i, j;
    double pivo = CEL(s, lin, col);

    for (j = 0; j < W; j++) {
        CEL(s, lin, j) /= pivo;
    }

    for (i = 0; i <= s->m; i++) {
        double fator;

        if (i == lin) continue;
        fator = CEL(s, i, col);
        if (fator == 0.0) continue;

        for (j = 0; j < W; j++) {
            CEL(s, i, j) -= fator * CEL(s, lin, j);
        }
        CEL(s, i, col) = 0.0;
    }

    CEL(s, lin, col) = 1.0;
    s->base[lin] = col;
}

/*
    Executa as iteracoes de uma fase.
    Retorna: 0 = otimo da fase, 1 = ilimitado, 2 = limite de iteracoes.
*/
int executarFase(Simplex *s, int fase, FILE *log) {
    int rhs = s->ncols;
    double *razao = malloc((size_t)(s->m + 1) * sizeof(double));
    const char *nomeFase = (fase == 1) ? "I" : "II";
    const char *rotulo = (fase == 1) ? "w" : "z";

    if (razao == NULL) {
        printf("Erro ao alocar memoria.\n");
        return 2;
    }

    while (1) {
        int i, j, entra = -1, sai = -1;
        double menor = -EPS, melhor = 0.0;
        char nomeEntra[32], nomeSai[32];

        /* Variavel que entra: coeficiente mais negativo (empate: menor indice) */
        for (j = 0; j < s->ncols; j++) {
            if (!s->ativa[j]) continue;
            if (CEL(s, 0, j) < menor) {
                menor = CEL(s, 0, j);
                entra = j;
            }
        }

        emitir(log, "\n=== FASE %s - Tableau %d ===\n", nomeFase, s->iteracoes[fase]);

        if (entra < 0) {
            emitirTableau(log, s, fase, NULL);
            emitir(log, "Todos os coeficientes da linha %s sao >= 0: criterio de parada atendido.\n", rotulo);
            free(razao);
            return 0;
        }

        /* Teste da razao: so linhas com elemento positivo */
        for (i = 1; i <= s->m; i++) {
            if (CEL(s, i, entra) > EPS) {
                double r = CEL(s, i, rhs) / CEL(s, i, entra);
                razao[i] = r;
                if (sai < 0 || r < melhor - EPS ||
                    (fabs(r - melhor) <= EPS && s->base[i] < s->base[sai])) {
                    sai = i;
                    melhor = r;
                }
            } else {
                razao[i] = -1.0;
            }
        }

        emitirTableau(log, s, fase, razao);
        nomeVariavel(s, entra, nomeEntra, sizeof(nomeEntra));

        if (sai < 0) {
            emitir(log, "Entra: %s | nenhuma linha com elemento positivo na coluna.\n", nomeEntra);
            emitir(log, "A variavel %s pode crescer indefinidamente: PROBLEMA ILIMITADO.\n", nomeEntra);
            free(razao);
            return 1;
        }

        nomeVariavel(s, s->base[sai], nomeSai, sizeof(nomeSai));
        emitir(log, "Entra: %s | Sai: %s | Pivo = %.3lf (linha %d)\n",
               nomeEntra, nomeSai, CEL(s, sai, entra), sai);

        pivotear(s, sai, entra);
        s->iteracoes[fase]++;

        if (s->iteracoes[1] + s->iteracoes[2] >= MAX_ITERACOES) {
            emitir(log, "Limite de %d iteracoes atingido.\n", MAX_ITERACOES);
            free(razao);
            return 2;
        }
    }
}

/*
    Resolve o problema a partir do tableau inicial lido do arquivo da forma
    padrao (colunas x | F | E | b; linha 0 = z - cx = 0).
    Passos: normaliza b < 0, cria artificiais onde nao ha base evidente,
    Fase I, teste de inviabilidade, remocao das artificiais, Fase II,
    deteccao de ilimitabilidade e apresentacao da solucao.
*/
int resolverSimplex(
    const double tabela[],
    int m0,
    int n,
    int nf,
    int ne,
    const char *caminhoSolucao
) {
    int nc = n + nf + ne;
    int W0 = nc + 1;
    int i, j, k;
    int na = 0;
    int resultado = 0;   /* 0 otimo, 1 ilimitado, 2 inviavel, 3 limite de iteracoes */
    int status;
    double w;
    FILE *log = fopen(caminhoSolucao, "w");
    Simplex s;
    double *ini = malloc((size_t)(m0 + 1) * (size_t)W0 * sizeof(double));
    int *baseIni = malloc((size_t)(m0 + 1) * sizeof(int));
    double *zorig;

    if (log == NULL) {
        printf("Aviso: nao foi possivel criar %s (resultado apenas na tela).\n", caminhoSolucao);
    }

    if (ini == NULL || baseIni == NULL) {
        printf("Erro ao alocar memoria para o simplex.\n");
        free(ini);
        free(baseIni);
        if (log != NULL) fclose(log);
        return 0;
    }

    memcpy(ini, tabela, (size_t)(m0 + 1) * (size_t)W0 * sizeof(double));

    emitir(log, "\n============================================\n");
    emitir(log, "        METODO SIMPLEX - DUAS FASES\n");
    emitir(log, "============================================\n");

    /* Passo 1: normalizacao - o simplex exige b >= 0 */
    for (i = 1; i <= m0; i++) {
        if (ini[i * W0 + nc] < -EPS) {
            for (j = 0; j < W0; j++) {
                ini[i * W0 + j] = -ini[i * W0 + j];
            }
            emitir(log, "Restricao %d tinha b < 0: linha multiplicada por -1.\n", i);
        }
    }

    /* Passo 2: base inicial. Coluna de folga/excesso que seja unitaria
       (um 1 na linha e zeros nas demais) serve de base; senao, artificial. */
    for (i = 1; i <= m0; i++) {
        baseIni[i] = -1;
        for (j = n; j < nc && baseIni[i] < 0; j++) {
            if (fabs(ini[i * W0 + j] - 1.0) < EPS) {
                int unitaria = 1;
                for (k = 1; k <= m0; k++) {
                    if (k != i && fabs(ini[k * W0 + j]) > EPS) {
                        unitaria = 0;
                        break;
                    }
                }
                if (unitaria) baseIni[i] = j;
            }
        }
        if (baseIni[i] < 0) na++;
    }

    s.n = n; s.nf = nf; s.ne = ne; s.na = na;
    s.m = m0;
    s.ncols = nc + na;
    s.iteracoes[0] = s.iteracoes[1] = s.iteracoes[2] = 0;
    s.t = calloc((size_t)(m0 + 1) * (size_t)(s.ncols + 1), sizeof(double));
    s.base = calloc((size_t)(m0 + 1), sizeof(int));
    s.ativa = calloc((size_t)s.ncols + 1, sizeof(int));
    zorig = calloc((size_t)s.ncols + 1, sizeof(double));

    if (s.t == NULL || s.base == NULL || s.ativa == NULL || zorig == NULL) {
        printf("Erro ao alocar memoria para o tableau.\n");
        free(s.t); free(s.base); free(s.ativa); free(zorig);
        free(ini); free(baseIni);
        if (log != NULL) fclose(log);
        return 0;
    }

    for (i = 0; i <= m0; i++) {
        for (j = 0; j < nc; j++) CEL(&s, i, j) = ini[i * W0 + j];
        CEL(&s, i, s.ncols) = ini[i * W0 + nc];
    }

    k = 0;
    for (i = 1; i <= m0; i++) {
        if (baseIni[i] >= 0) {
            s.base[i] = baseIni[i];
        } else {
            CEL(&s, i, nc + k) = 1.0;        /* variavel artificial */
            s.base[i] = nc + k;
            k++;
        }
    }

    for (j = 0; j < s.ncols; j++) s.ativa[j] = 1;
    for (j = 0; j <= s.ncols; j++) zorig[j] = CEL(&s, 0, j);   /* linha z original */

    emitir(log, "Variaveis: %d de decisao, %d de folga, %d de excesso, %d artificiais.\n",
           n, nf, ne, na);

    /* Passos 3 e 4: Fase I e teste de inviabilidade */
    if (na > 0) {
        emitir(log, "\n############################################\n");
        emitir(log, "#                 FASE I                   #\n");
        emitir(log, "#   Minimizar w = soma das artificiais     #\n");
        emitir(log, "############################################\n");

        for (j = 0; j < s.ncols; j++) CEL(&s, 0, j) = (j >= nc) ? 1.0 : 0.0;
        CEL(&s, 0, s.ncols) = 0.0;

        for (i = 1; i <= s.m; i++) {
            if (s.base[i] >= nc) {
                for (j = 0; j <= s.ncols; j++) CEL(&s, 0, j) -= CEL(&s, i, j);
            }
        }

        status = executarFase(&s, 1, log);

        if (status == 2) {
            resultado = 3;
        } else {
            w = -CEL(&s, 0, s.ncols);
            emitir(log, "\nFim da Fase I: w = %.6lf\n", limpar(w));

            if (w > TOL_INVIAVEL) {
                emitir(log, "w > 0: nao e possivel zerar as artificiais.\n");
                resultado = 2;
            } else {
                emitir(log, "w = 0: existe solucao basica viavel.\n");
            }
        }
    } else {
        emitir(log, "\nTodas as restricoes tem base evidente: Fase I dispensada.\n");
    }

    /* Passo 5: remover artificiais */
    if (resultado == 0 && na > 0) {
        for (i = 1; i <= s.m; ) {
            if (s.base[i] >= nc) {
                int col = -1;
                for (j = 0; j < nc; j++) {
                    if (fabs(CEL(&s, i, j)) > EPS) { col = j; break; }
                }

                if (col >= 0) {
                    char nomeA[32], nomeC[32];
                    nomeVariavel(&s, s.base[i], nomeA, sizeof(nomeA));
                    nomeVariavel(&s, col, nomeC, sizeof(nomeC));
                    emitir(log, "Artificial %s basica com valor 0: pivoteando na coluna %s.\n", nomeA, nomeC);
                    pivotear(&s, i, col);
                    i++;
                } else {
                    emitir(log, "Linha %d redundante (so zeros nas colunas nao artificiais): removida.\n", i);
                    for (k = i; k < s.m; k++) {
                        for (j = 0; j <= s.ncols; j++) CEL(&s, k, j) = CEL(&s, k + 1, j);
                        s.base[k] = s.base[k + 1];
                    }
                    s.m--;
                }
            } else {
                i++;
            }
        }

        for (j = nc; j < s.ncols; j++) s.ativa[j] = 0;
        emitir(log, "Colunas artificiais removidas.\n");
    }

    /* Passos 6, 7 e 8: Fase II */
    if (resultado == 0) {
        emitir(log, "\n############################################\n");
        emitir(log, "#                FASE II                   #\n");
        emitir(log, "#   Maximizar Z (funcao objetivo original) #\n");
        emitir(log, "############################################\n");

        /* Recalcula a linha z com a funcao original e a base atual */
        for (j = 0; j <= s.ncols; j++) CEL(&s, 0, j) = zorig[j];
        for (i = 1; i <= s.m; i++) {
            double f = CEL(&s, 0, s.base[i]);
            if (f != 0.0) {
                for (j = 0; j <= s.ncols; j++) CEL(&s, 0, j) -= f * CEL(&s, i, j);
            }
        }

        status = executarFase(&s, 2, log);
        if (status == 1) resultado = 1;
        else if (status == 2) resultado = 3;
    }

    /* Passo 9: apresentacao */
    emitir(log, "\n============================================\n");
    emitir(log, "              RESULTADO FINAL\n");
    emitir(log, "============================================\n");

    if (resultado == 0) {
        double *valor = calloc((size_t)nc + 1, sizeof(double));
        int *basica = calloc((size_t)nc + 1, sizeof(int));
        int alternativo = 0;

        if (valor != NULL && basica != NULL) {
            for (i = 1; i <= s.m; i++) {
                if (s.base[i] < nc) {
                    valor[s.base[i]] = CEL(&s, i, s.ncols);
                    basica[s.base[i]] = 1;
                }
            }

            emitir(log, "Status: SOLUCAO OTIMA\n\n");
            for (j = 0; j < n; j++) {
                emitir(log, "x%d = %.3lf\n", j + 1, limpar(valor[j]));
            }
            for (j = n; j < nc; j++) {
                char nome[32];
                nomeVariavel(&s, j, nome, sizeof(nome));
                emitir(log, "%s = %.3lf\n", nome, limpar(valor[j]));
            }
            emitir(log, "\nZ = %.3lf\n", limpar(CEL(&s, 0, s.ncols)));

            for (j = 0; j < nc; j++) {
                if (!basica[j] && fabs(CEL(&s, 0, j)) < EPS) alternativo = 1;
            }
            if (alternativo) {
                emitir(log, "\nObs.: ha variavel nao basica com custo reduzido 0 (possivel otimo alternativo).\n");
            }
        }

        free(valor);
        free(basica);
    } else if (resultado == 1) {
        emitir(log, "Status: PROBLEMA ILIMITADO\n");
        emitir(log, "Detectado na Fase II, iteracao %d.\n", s.iteracoes[2]);
    } else if (resultado == 2) {
        emitir(log, "Status: PROBLEMA INVIAVEL\n");
        emitir(log, "Ao final da Fase I, w = %.6lf > 0.\n", limpar(-CEL(&s, 0, s.ncols)));
    } else {
        emitir(log, "Status: LIMITE DE ITERACOES ATINGIDO\n");
    }

    emitir(log, "\nIteracoes na Fase I : %d\n", s.iteracoes[1]);
    emitir(log, "Iteracoes na Fase II: %d\n", s.iteracoes[2]);
    emitir(log, "Total de iteracoes  : %d\n", s.iteracoes[1] + s.iteracoes[2]);

    free(s.t); free(s.base); free(s.ativa); free(zorig);
    free(ini); free(baseIni);
    if (log != NULL) fclose(log);

    return 1;
}

/*
    Le problemaSimplexNaFormaPadrao.txt, extrai os coeficientes, grava
    a tabela inicial do simplex em 'caminhoSaida' (e mostra na tela) e
    resolve o problema (resultado e tableaux em 'caminhoSolucao').
    Retorna 1 em caso de sucesso e 0 em caso de erro.
*/
int gerarTabelaSimplex(
    const char *caminhoEntrada,
    const char *caminhoSaida,
    int n,
    int nf,
    int ne,
    int numeroRestricoes,
    const char *caminhoSolucao
) {
    int colunas = n + nf + ne;
    int largura = colunas + 1;
    int linhasTabela = numeroRestricoes + 1;
    int linhaAtual = 1;
    char linha[TAM_ENTRADA];
    FILE *entrada;
    FILE *saida;
    double *tabela;

    tabela = calloc((size_t)linhasTabela * (size_t)largura, sizeof(double));
    if (tabela == NULL) {
        printf("Erro ao alocar memoria para a tabela simplex.\n");
        return 0;
    }

    entrada = fopen(caminhoEntrada, "r");
    if (entrada == NULL) {
        printf("Erro ao abrir o arquivo da forma padrao: %s\n", caminhoEntrada);
        free(tabela);
        return 0;
    }

    while (fgets(linha, sizeof(linha), entrada) != NULL) {

        // Ignora linhas em branco
        if (linha[strspn(linha, " \t\r\n")] == '\0') {
            continue;
        }

        if (ehFuncaoObjetivo(linha)) {
            lerLinhaForma(linha, &tabela[0], n, nf, ne);
        } else if (ehRestricaoNaoNegatividade(linha)) {
            continue;
        } else if (linhaAtual < linhasTabela) {
            lerLinhaForma(linha, &tabela[linhaAtual * largura], n, nf, ne);
            linhaAtual++;
        }
    }

    fclose(entrada);

    saida = fopen(caminhoSaida, "w");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo da tabela: %s\n", caminhoSaida);
        free(tabela);
        return 0;
    }

    imprimirTabela(saida, tabela, linhasTabela, n, nf, ne);

    if (fclose(saida) == EOF) {
        printf("Erro ao finalizar o arquivo da tabela.\n");
        free(tabela);
        return 0;
    }

    printf("\n============================================\n");
    printf("            TABELA SIMPLEX INICIAL\n");
    printf("============================================\n");
    imprimirTabela(stdout, tabela, linhasTabela, n, nf, ne);

    // Resolve o problema com o simplex de duas fases, mostrando cada tableau
    resolverSimplex(tabela, numeroRestricoes, n, nf, ne, caminhoSolucao);

    free(tabela);
    return 1;
}

/*
    Leitura segura: encerra o programa se a entrada (teclado ou arquivo)
    estiver incompleta ou invalida.
*/
void lerEntrada(const char *formato, void *destino) {
    if (scanf(formato, destino) != 1) {
        printf("\nErro: entrada invalida ou incompleta.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    // ============================================================
    // ENTRADA POR ARQUIVO (opcional)
    // ============================================================
    // Uso: simplex entrada.txt
    // Formato: N / coeficientes de Z / B e B linhas (<=) /
    //          C e C linhas (=) / D e D linhas (>=)
    // Sem argumento, os dados sao digitados no teclado.

    if (argc > 1) {
        if (freopen(argv[1], "r", stdin) == NULL) {
            printf("Erro ao abrir o arquivo de entrada: %s\n", argv[1]);
            return 1;
        }
        printf("Lendo o problema do arquivo: %s\n", argv[1]);
    }


    // ============================================================
    // VARIÁVEIS PRINCIPAIS
    // ============================================================

    int numeroVariaveisDecisao;
    int numeroRestricoesMenorIgual;
    int numeroRestricoesIgual;
    int numeroRestricoesMaiorIgual;

    int i, j;

    FILE *arquivoRestricoes;
    FILE *problemaSimplexCompleto;


    // ============================================================
    // CAMINHOS DOS ARQUIVOS
    // ============================================================

    const char *caminhoProblema ="C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplex.txt";

    const char *caminhoRestricoes ="C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\gerarGraficos\\restricoes.txt";


    // ============================================================
    // ABERTURA DOS ARQUIVOS
    // ============================================================

    problemaSimplexCompleto = fopen(caminhoProblema, "w");

    if (problemaSimplexCompleto == NULL) {

        printf("\n============================================\n");
        printf("ERRO AO ABRIR problemaSimplex.txt\n");
        printf("============================================\n");

        printf("\nCaminho utilizado:\n");
        printf("%s\n", caminhoProblema);

        return 1;
    }


    arquivoRestricoes = fopen(caminhoRestricoes, "w");

    if (arquivoRestricoes == NULL) {

        printf("\n============================================\n");
        printf("ERRO AO ABRIR restricoes.txt\n");
        printf("============================================\n");

        printf("\nCaminho utilizado:\n");
        printf("%s\n", caminhoRestricoes);

        printf("\nVerifique se a pasta existe:\n");
        printf("C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\gerarGraficos\n");

        fclose(problemaSimplexCompleto);

        return 1;
    }


    printf("\n============================================\n");
    printf("ARQUIVOS ABERTOS COM SUCESSO!\n");
    printf("============================================\n");


    // ============================================================
    // RECEBENDO A QUANTIDADE DE VARIÁVEIS DE DECISÃO
    // ============================================================

    printf("\nDigite a quantidade de variaveis de decisao: ");
    lerEntrada("%d", &numeroVariaveisDecisao);


    if (numeroVariaveisDecisao <= 0) {

        printf("\nErro: a quantidade de variaveis deve ser maior que zero.\n");

        fclose(arquivoRestricoes);
        fclose(problemaSimplexCompleto);

        return 1;
    }


    // ============================================================
    // ALOCAÇÃO DA FUNÇÃO OBJETIVO
    // ============================================================

    double *coeficienteFuncaoObjetivo;

    coeficienteFuncaoObjetivo = malloc(numeroVariaveisDecisao * sizeof(double));


    if (coeficienteFuncaoObjetivo == NULL) {

        printf("\nErro ao alocar memoria para a funcao objetivo.\n");

        fclose(arquivoRestricoes);
        fclose(problemaSimplexCompleto);

        return 1;
    }


    // ============================================================
    // RECEBENDO A FUNÇÃO OBJETIVO
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("           FUNCAO OBJETIVO\n");
    printf("============================================\n");


    for (i = 0; i < numeroVariaveisDecisao; i++) {

        printf("Valor da variavel de decisao x%d: ",i + 1);

        lerEntrada("%lf",&coeficienteFuncaoObjetivo[i]);
    }


    // ============================================================
    // ESCREVENDO A FUNÇÃO OBJETIVO
    // ============================================================

    fprintf(problemaSimplexCompleto,"z = ");


    for (i = 0; i < numeroVariaveisDecisao; i++) {

        double coeficiente = coeficienteFuncaoObjetivo[i];


        if (i == 0) {
            if (coeficiente < 0) {
                fprintf(problemaSimplexCompleto,"-%.2lfx%d",-coeficiente,i + 1);

            } else {
                fprintf(problemaSimplexCompleto,"%.2lfx%d",coeficiente,i + 1);
            }

        } else {

            if (coeficiente >= 0) {
                fprintf(problemaSimplexCompleto," + %.2lfx%d",coeficiente,i + 1);

            } else {

                fprintf(problemaSimplexCompleto," -%.2lfx%d",-coeficiente,i + 1);
            }
        }
    }


    fprintf(problemaSimplexCompleto,"\n");


    // ============================================================
    // RESTRIÇÕES DO TIPO <=
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("          RESTRICOES DO TIPO <=\n");
    printf("============================================\n");


    printf(
        "Digite a quantidade de restricoes do tipo <=: "
    );

    lerEntrada("%d",&numeroRestricoesMenorIgual);


    if (numeroRestricoesMenorIgual < 0) {

        printf("\nErro: quantidade invalida de restricoes.\n");

        free(coeficienteFuncaoObjetivo);

        fclose(arquivoRestricoes);
        fclose(problemaSimplexCompleto);

        return 1;
    }


    // ============================================================
    // MATRIZ DAS RESTRIÇÕES <=
    //
    // Cada linha possui:
    //
    // x1 | x2 | x3 | ... | xn | termo independente
    //
    // ============================================================

    double **matrizMenorIgual = NULL;

    if (numeroRestricoesMenorIgual > 0) {

        matrizMenorIgual = malloc(numeroRestricoesMenorIgual * sizeof(double *));


        if (matrizMenorIgual == NULL) {

            printf("\nErro ao alocar matriz das restricoes <=.\n");

            free(coeficienteFuncaoObjetivo);

            fclose(arquivoRestricoes);
            fclose(problemaSimplexCompleto);

            return 1;
        }


        // --------------------------------------------------------
        // Aloca cada linha da matriz
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesMenorIgual; i++) {

            matrizMenorIgual[i] = malloc((numeroVariaveisDecisao + 1) * sizeof(double));

            if (matrizMenorIgual[i] == NULL) {

                printf("\nErro ao alocar memoria para a matriz <=.\n");


                for (j = 0; j < i; j++) {
                    free(matrizMenorIgual[j]);
                }

                free(matrizMenorIgual);

                free(coeficienteFuncaoObjetivo);

                fclose(arquivoRestricoes);
                fclose(problemaSimplexCompleto);

                return 1;
            }
        }


        // --------------------------------------------------------
        // Preenche a matriz
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesMenorIgual; i++) {

            printf("\n");
            printf("Restricao <= %d\n", i + 1);

            for (j = 0; j < numeroVariaveisDecisao + 1; j++) {

                if (j < numeroVariaveisDecisao) {
                    printf("Coeficiente de x%d: ",j + 1);
                } else {
                    printf("Termo independente: ");
                }

                lerEntrada("%lf",&matrizMenorIgual[i][j]);
            }
        }


        // --------------------------------------------------------
        // Salva as restrições <=
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesMenorIgual; i++) {

            for (j = 0; j < numeroVariaveisDecisao; j++) {

                double coeficiente = matrizMenorIgual[i][j];

                if (j == 0) {

                    if (coeficiente < 0) {

                        fprintf(problemaSimplexCompleto,"-%.2lfx%d",-coeficiente,j + 1);

                        fprintf(arquivoRestricoes,"-%.2lfx%d",-coeficiente,j + 1);

                    } else {

                        fprintf(problemaSimplexCompleto,"%.2lfx%d",coeficiente,j + 1);

                        fprintf(arquivoRestricoes,"%.2lfx%d",coeficiente,j + 1);
                    }

                } else {

                    if (coeficiente >= 0) {

                        fprintf(
                            problemaSimplexCompleto," + %.2lfx%d",coeficiente,j + 1);

                        fprintf(arquivoRestricoes," + %.2lfx%d",coeficiente,j + 1);

                    } else {
                        fprintf(problemaSimplexCompleto," - %.2lfx%d",-coeficiente,j + 1);

                        fprintf(arquivoRestricoes," - %.2lfx%d",-coeficiente,j + 1);
                    }
                }
            }


            fprintf(problemaSimplexCompleto," <= %.2lf\n",matrizMenorIgual[i][numeroVariaveisDecisao]);


            fprintf(arquivoRestricoes," <= %.2lf\n",matrizMenorIgual[i][numeroVariaveisDecisao]);
        }
    }


    // ============================================================
    // RESTRIÇÕES DO TIPO =
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("           RESTRICOES DO TIPO =\n");
    printf("============================================\n");


    printf("Digite a quantidade de restricoes do tipo =: ");

    lerEntrada("%d",&numeroRestricoesIgual);

    if (numeroRestricoesIgual < 0) {

        printf("\nErro: quantidade invalida de restricoes.\n");

        if (matrizMenorIgual != NULL) {

            for (i = 0; i < numeroRestricoesMenorIgual; i++) {

                free(matrizMenorIgual[i]);
            }

            free(matrizMenorIgual);
        }

        free(coeficienteFuncaoObjetivo);

        fclose(arquivoRestricoes);
        fclose(problemaSimplexCompleto);

        return 1;
    }


    // ============================================================
    // MATRIZ DAS RESTRIÇÕES =
    // ============================================================

    double **matrizIgual = NULL;


    if (numeroRestricoesIgual > 0) {

        matrizIgual = malloc(numeroRestricoesIgual * sizeof(double *));


        if (matrizIgual == NULL) {

            printf("\nErro ao alocar matriz das restricoes =.\n");

            if (matrizMenorIgual != NULL) {

                for (i = 0; i < numeroRestricoesMenorIgual; i++) {

                    free(matrizMenorIgual[i]);
                }

                free(matrizMenorIgual);
            }

            free(coeficienteFuncaoObjetivo);

            fclose(arquivoRestricoes);
            fclose(problemaSimplexCompleto);

            return 1;
        }


        // --------------------------------------------------------
        // Aloca cada linha
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesIgual; i++) {

            matrizIgual[i] = malloc((numeroVariaveisDecisao + 1) * sizeof(double));


            if (matrizIgual[i] == NULL) {

                printf("\nErro ao alocar memoria para matriz =.\n");


                for (j = 0; j < i; j++) {

                    free(matrizIgual[j]);
                }

                free(matrizIgual);


                if (matrizMenorIgual != NULL) {

                    for (j = 0; j < numeroRestricoesMenorIgual; j++) {

                        free(matrizMenorIgual[j]);
                    }

                    free(matrizMenorIgual);
                }


                free(coeficienteFuncaoObjetivo);

                fclose(arquivoRestricoes);
                fclose(problemaSimplexCompleto);

                return 1;
            }
        }


        // --------------------------------------------------------
        // Preenche a matriz
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesIgual; i++) {

            printf("\n");
            printf("Restricao = %d\n", i + 1);


            for (j = 0; j < numeroVariaveisDecisao + 1; j++) {

                if (j < numeroVariaveisDecisao) {

                    printf(
                        "Coeficiente de x%d: ",
                        j + 1
                    );

                } else {

                    printf(
                        "Termo independente: "
                    );
                }


                lerEntrada("%lf",&matrizIgual[i][j]);
            }
        }


        // --------------------------------------------------------
        // Salva as restrições =
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesIgual; i++) {

            for (j = 0; j < numeroVariaveisDecisao; j++) {

                double coeficiente =
                    matrizIgual[i][j];


                if (j == 0) {

                    if (coeficiente < 0) {

                        fprintf(problemaSimplexCompleto,"-%.2lfx%d",-coeficiente,j + 1);

                        fprintf(arquivoRestricoes,"-%.2lfx%d",-coeficiente,j + 1);

                    } else {

                        fprintf(problemaSimplexCompleto,"%.2lfx%d",coeficiente,j + 1);
                        fprintf(arquivoRestricoes,"%.2lfx%d", coeficiente,j + 1);
                    }

                } else {

                    if (coeficiente >= 0) {

                        fprintf(problemaSimplexCompleto," + %.2lfx%d",coeficiente, j + 1);

                        fprintf(arquivoRestricoes," + %.2lfx%d", coeficiente,j + 1);

                    } else {
                        fprintf(problemaSimplexCompleto," - %.2lfx%d",-coeficiente,j + 1);

                        fprintf(arquivoRestricoes," - %.2lfx%d",-coeficiente,j + 1);
                    }
                }
            }


            fprintf(problemaSimplexCompleto," = %.2lf\n",matrizIgual[i][numeroVariaveisDecisao]);


            fprintf(arquivoRestricoes," = %.2lf\n",matrizIgual[i][numeroVariaveisDecisao]);
        }
    }


    // ============================================================
    // RESTRIÇÕES DO TIPO >=
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("          RESTRICOES DO TIPO >=\n");
    printf("============================================\n");


    printf(
        "Digite a quantidade de restricoes do tipo >=: "
    );

    lerEntrada("%d",&numeroRestricoesMaiorIgual);


    // ============================================================
    // VALIDAÇÃO DA QUANTIDADE
    // ============================================================

    if (numeroRestricoesMaiorIgual < 0) {

        printf(
            "\nErro: quantidade invalida de restricoes.\n"
        );

        free(coeficienteFuncaoObjetivo);

        fclose(arquivoRestricoes);
        fclose(problemaSimplexCompleto);

        return 1;
    }


    // ============================================================
    // MATRIZ DAS RESTRIÇÕES >=
    //
    // Cada linha possui:
    //
    // x1 | x2 | x3 | ... | xn | termo independente
    //
    // Exemplo:
    //
    // 2 | 3 | 20
    //
    // representa:
    //
    // 2x1 + 3x2 >= 20
    // ============================================================

    double **matrizMaiorIgual = NULL;


    if (numeroRestricoesMaiorIgual > 0) {

        // --------------------------------------------------------
        // Aloca o vetor de ponteiros
        // --------------------------------------------------------

        matrizMaiorIgual = malloc(numeroRestricoesMaiorIgual * sizeof(double *));


        if (matrizMaiorIgual == NULL) {

            printf(
                "\nErro ao alocar matriz das restricoes >=.\n"
            );

            free(coeficienteFuncaoObjetivo);

            fclose(arquivoRestricoes);
            fclose(problemaSimplexCompleto);

            return 1;
        }


        // --------------------------------------------------------
        // Aloca cada linha da matriz
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesMaiorIgual; i++) {

            matrizMaiorIgual[i] =
                malloc((numeroVariaveisDecisao + 1) * sizeof(double));


            if (matrizMaiorIgual[i] == NULL) {

                printf("\nErro ao alocar memoria para a matriz >=.\n");


                // Libera as linhas que já foram alocadas
                for (j = 0; j < i; j++) {

                    free(matrizMaiorIgual[j]);
                }


                free(matrizMaiorIgual);

                free(coeficienteFuncaoObjetivo);

                fclose(arquivoRestricoes);
                fclose(problemaSimplexCompleto);

                return 1;
            }
        }


        // ========================================================
        // PREENCHE A MATRIZ
        // ========================================================

        for (i = 0; i < numeroRestricoesMaiorIgual; i++) {

            printf("\n");
            printf("Restricao >= %d\n", i + 1);


            for (j = 0; j < numeroVariaveisDecisao + 1; j++) {

                if (j < numeroVariaveisDecisao) {

                    printf("Coeficiente de x%d: ",j + 1);

                } else {

                    printf("Termo independente: ");
                }


                lerEntrada("%lf",&matrizMaiorIgual[i][j]);
            }
        }


        // ========================================================
        // SALVA AS RESTRIÇÕES >=
        // ========================================================

        for (i = 0; i < numeroRestricoesMaiorIgual; i++) {

            for (j = 0; j < numeroVariaveisDecisao; j++) {

                double coeficiente = matrizMaiorIgual[i][j];


                // ------------------------------------------------
                // PRIMEIRO COEFICIENTE
                // ------------------------------------------------

                if (j == 0) {

                    if (coeficiente < 0) {

                        fprintf(problemaSimplexCompleto,"- %.2lfx%d",-coeficiente, j + 1);

                        fprintf(arquivoRestricoes,"- %.2lfx%d",-coeficiente, j + 1);

                    } else {

                        fprintf(problemaSimplexCompleto,"%.2lfx%d",coeficiente,j + 1);

                        fprintf(arquivoRestricoes,"%.2lfx%d",coeficiente,j + 1);
                    }

                }

                // ------------------------------------------------
                // DEMAIS COEFICIENTES
                // ------------------------------------------------

                else {

                    if (coeficiente >= 0) {

                        fprintf(problemaSimplexCompleto," + %.2lfx%d",coeficiente,j + 1);

                        fprintf(arquivoRestricoes," + %.2lfx%d",coeficiente,j + 1);

                    } else {

                        fprintf(problemaSimplexCompleto," - %.2lfx%d",-coeficiente,j + 1);

                        fprintf(arquivoRestricoes," - %.2lfx%d",-coeficiente,j + 1);
                    }
                }
            }


            // ----------------------------------------------------
            // TERMO INDEPENDENTE
            // ----------------------------------------------------

            fprintf(problemaSimplexCompleto," >= %.2lf\n",matrizMaiorIgual[i][numeroVariaveisDecisao]);


            fprintf(arquivoRestricoes," >= %.2lf\n",matrizMaiorIgual[i][numeroVariaveisDecisao]);
        }
    }


    // ============================================================
    // NÃO NEGATIVIDADE DAS VARIÁVEIS
    // ============================================================

    // Gera x1 >= 0, x2 >= 0, ..., xn >= 0
    for (i = 0; i < numeroVariaveisDecisao; i++){
        // Grava a não negatividade no arquivo completo
        fprintf(problemaSimplexCompleto, "x%d >= 0\n", i + 1);

        // Grava a não negatividade no arquivo de restrições
        fprintf(arquivoRestricoes, "x%d >= 0\n", i + 1);
    }


    // ============================================================
    // IMPRESSÃO FINAL DO PROBLEMA
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("          PROBLEMA DE PROGRAMACAO LINEAR\n");
    printf("============================================\n\n");

    printf("z = ");

    for (i = 0; i < numeroVariaveisDecisao; i++)
    {
        if (i > 0)
        {
            printf(" + ");
        }

        printf("%.2lfx%d", coeficienteFuncaoObjetivo[i], i + 1);
    }

    printf("\n");

    // Imprime as restrições do tipo <=
    for (i = 0; i < numeroRestricoesMenorIgual; i++)
    {
        for (j = 0; j < numeroVariaveisDecisao; j++)
        {
            if (j > 0)
            {
                printf(" + ");
            }

            printf("%.2lfx%d",matrizMenorIgual[i][j],j + 1);
        }

        printf(" <= %.2lf\n",matrizMenorIgual[i][numeroVariaveisDecisao]);
    }


    // Imprime as restrições do tipo >=
    for (i = 0; i < numeroRestricoesMaiorIgual; i++)
    {
        for (j = 0; j < numeroVariaveisDecisao; j++)
        {
            if (j > 0)
            {
                printf(" + ");
            }

            printf("%.2lfx%d",matrizMaiorIgual[i][j],j + 1);
        }

        printf(" >= %.2lf\n",matrizMaiorIgual[i][numeroVariaveisDecisao]);
    }


    // Imprime as restrições do tipo =
    for (i = 0; i < numeroRestricoesIgual; i++)
    {
        for (j = 0; j < numeroVariaveisDecisao; j++)
        {
            if (j > 0)
            {
                printf(" + ");
            }

            printf("%.2lfx%d",matrizIgual[i][j],j + 1);
        }

        printf(" = %.2lf\n", matrizIgual[i][numeroVariaveisDecisao]);
    }


    // Imprime as condições de não negatividade
    for (i = 0; i < numeroVariaveisDecisao; i++)
    {
        printf("x%d >= 0\n", i + 1);
    }


    // ============================================================
    // FECHANDO OS ARQUIVOS DO PROBLEMA ORIGINAL
    // ============================================================
    // IMPORTANTE: problemaSimplex.txt precisa ser totalmente gravado
    // e fechado antes de ser aberto para leitura na etapa seguinte.

    if (fclose(arquivoRestricoes) == EOF) {
        printf("Erro ao fechar restricoes.txt.\n");
        fclose(problemaSimplexCompleto);
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    if (fclose(problemaSimplexCompleto) == EOF) {
        printf("Erro ao fechar problemaSimplex.txt.\n");
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    // ============================================================
    // TRANSFORMANDO AS RESTRICOES PARA A FORMA PADRAO
    // ============================================================
    // Funcao objetivo: z = c1x1 + c2x2 + ...  vira
    // z - c1x1 - c2x2 - ... - 0F1 - ... - 0E1 - ... = 0
    // <= vira + F1 =, + F2 =, ... (variaveis de folga)
    // >= vira - E1 =, - E2 =, ... (variaveis de excesso)
    // = permanece inalterado.
    // As linhas x1 >= 0, x2 >= 0, ... sao mantidas sem alteracao.

    FILE *arquivoEntrada;
    FILE *arquivoSaida;

    char linha[TAM_ENTRADA];
    char etapa[TAM_SAIDA];
    char modificada[TAM_SAIDA];

    int quantidadeVariaveisF = 1;
    int quantidadeVariaveisE = 1;

    const char *caminhoEntrada ="C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplex.txt";

    const char *caminhoSaida ="C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplexNaFormaPadrao.txt";

    arquivoEntrada = fopen(caminhoEntrada, "r");
    if (arquivoEntrada == NULL) {
        printf("Erro ao abrir o arquivo de entrada: %s\n", caminhoEntrada);
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    arquivoSaida = fopen(caminhoSaida, "w");
    if (arquivoSaida == NULL) {
        printf("Erro ao criar o arquivo de saida: %s\n", caminhoSaida);
        fclose(arquivoEntrada);
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL) {

        // Funcao objetivo: z = 3x1 + 2x2  ->  z - 3.00x1 - 2.00x2 - 0.00F1 - 0.00E1 = 0
        if (ehFuncaoObjetivo(linha)) {
            if (!escreverFuncaoObjetivoPadrao(
                    arquivoSaida,
                    coeficienteFuncaoObjetivo,
                    numeroVariaveisDecisao,
                    numeroRestricoesMenorIgual,
                    numeroRestricoesMaiorIgual)) {
                printf("Erro ao escrever a funcao objetivo no arquivo de saida.\n");
                fclose(arquivoEntrada);
                fclose(arquivoSaida);
                free(coeficienteFuncaoObjetivo);
                return 1;
            }
            continue;
        }

        // Mantem as condicoes x1 >= 0, x2 >= 0, ... sem substituicao.
        if (ehRestricaoNaoNegatividade(linha)) {
            if (fputs(linha, arquivoSaida) == EOF) {
                printf("Erro ao escrever no arquivo de saida.\n");
                fclose(arquivoEntrada);
                fclose(arquivoSaida);
                free(coeficienteFuncaoObjetivo);
                return 1;
            }
            continue;
        }

        // Primeira etapa: substitui <= por variaveis de folga F numeradas.
        if (!substituirTodas(linha, "<=", " + F%d = ", etapa, sizeof(etapa),&quantidadeVariaveisF)){
            printf("Erro ao substituir o operador <=. Linha muito grande?\n");
            fclose(arquivoEntrada);
            fclose(arquivoSaida);
            free(coeficienteFuncaoObjetivo);
            return 1;
        }

        // Segunda etapa: substitui >= por variaveis de excesso E numeradas.
        if (!substituirTodas(
                etapa, ">=", " - E%d = ", modificada, sizeof(modificada),
                &quantidadeVariaveisE)) {
            printf("Erro ao substituir o operador >=. Linha muito grande?\n");
            fclose(arquivoEntrada);
            fclose(arquivoSaida);
            free(coeficienteFuncaoObjetivo);
            return 1;
        }

        if (fputs(modificada, arquivoSaida) == EOF) {
            printf("Erro ao escrever no arquivo de saida.\n");
            fclose(arquivoEntrada);
            fclose(arquivoSaida);
            free(coeficienteFuncaoObjetivo);
            return 1;
        }
    }

    if (ferror(arquivoEntrada)) {
        printf("Erro durante a leitura do arquivo de entrada.\n");
        fclose(arquivoEntrada);
        fclose(arquivoSaida);
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    fclose(arquivoEntrada);
    if (fclose(arquivoSaida) == EOF) {
        printf("Erro ao finalizar o arquivo de saida.\n");
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    printf("\nTransformacao para a forma padrao concluida!\n");
    printf("Variaveis de folga criadas (F): %d\n", quantidadeVariaveisF - 1);
    printf("Variaveis de excesso criadas (E): %d\n", quantidadeVariaveisE - 1);
    printf("Arquivo gerado: %s\n", caminhoSaida);

    // ============================================================
    // GERANDO A TABELA DO SIMPLEX A PARTIR DA FORMA PADRAO
    // ============================================================

    const char *caminhoTabela ="C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tabelaSimplex.txt";

    const char *caminhoSolucao = "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\solucaoSimplex.txt";

    if (!gerarTabelaSimplex(
            caminhoSaida,
            caminhoTabela,
            numeroVariaveisDecisao,
            quantidadeVariaveisF - 1,
            quantidadeVariaveisE - 1,
            numeroRestricoesMenorIgual +
            numeroRestricoesIgual +
            numeroRestricoesMaiorIgual,
            caminhoSolucao)) {
        printf("Erro ao gerar a tabela do simplex.\n");
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    printf("\nArquivo da tabela gerado: %s\n", caminhoTabela);
    printf("Arquivo da solucao e dos tableaux: %s\n", caminhoSolucao);

    printf("\n");
    printf("============================================\n");

    // Os arquivos problemaSimplex.txt e restricoes.txt já foram
    // fechados antes da transformação para garantir a gravação.


    // ============================================================
    // LIBERANDO A MEMÓRIA DA FUNÇÃO OBJETIVO
    // ============================================================

    free(coeficienteFuncaoObjetivo);


    // ============================================================
    // LIBERANDO A MEMÓRIA DA MATRIZ <=
    // ============================================================

    if (matrizMenorIgual != NULL) {

        for (i = 0; i < numeroRestricoesMenorIgual; i++) {

            free(matrizMenorIgual[i]);
        }

        free(matrizMenorIgual);
    }


    // ============================================================
    // LIBERANDO A MEMÓRIA DA MATRIZ =
    // ============================================================

    if (matrizIgual != NULL) {

        for (i = 0; i < numeroRestricoesIgual; i++) {

            free(matrizIgual[i]);
        }

        free(matrizIgual);
    }


    // ============================================================
    // LIBERANDO A MEMÓRIA DA MATRIZ >=
    // ============================================================

    if (matrizMaiorIgual != NULL) {

        for (i = 0; i < numeroRestricoesMaiorIgual; i++) {

            free(matrizMaiorIgual[i]);
        }

        free(matrizMaiorIgual);
    }

    // ============================================================
    // FINALIZAÇÃO
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("     PROBLEMA SIMPLEX GERADO COM SUCESSO\n");
    printf("============================================\n");

    printf("\nArquivo do problema:\n");
    printf("%s\n", caminhoProblema);

    printf("\nArquivo das restricoes:\n");
    printf("%s\n", caminhoRestricoes);

    printf("\n");


    return 0;
}