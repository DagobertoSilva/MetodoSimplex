#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_ENTRADA 1024
#define TAM_SAIDA 4096
#define TAM_SUBSTITUTO 50

/*
    Substitui todas as ocorrencias de um operador por um texto numerado.
    Exemplo: "<=" -> " + F1 = " e ">=" -> " - E1 = ".
    Retorna 1 em caso de sucesso e 0 se ocorrer erro/faltar espaco.
*/
int substituirTodas(
    const char entrada[],
    const char procurar[],
    const char formatoSubstituto[],
    char saida[],
    size_t capacidade,
    int *contador
) {
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

    Base   |        x1 |        x2 |        F1 | ... |         b
    -------+-----------+-----------+-----------+-----+----------
    z      |     -4.00 |     -5.00 |      0.00 | ... |      0.00
    F1     |      1.00 |      1.00 |      1.00 | ... |     15.00
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
    for (j = 0; j < colunas; j++) {
        nomeColuna(nome, sizeof(nome), j, n, nf);
        fprintf(saida, " | %9s", nome);
    }
    fprintf(saida, " | %9s", "b");   // b e a ultima coluna
    fprintf(saida, "\n");

    fprintf(saida, "-------");
    for (j = 0; j <= colunas; j++) {
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

/*
    Le problemaSimplexNaFormaPadrao.txt, extrai os coeficientes e grava
    a tabela do simplex em 'caminhoSaida' (e mostra na tela).
    Retorna 1 em caso de sucesso e 0 em caso de erro.
*/
int gerarTabelaSimplex(
    const char *caminhoEntrada,
    const char *caminhoSaida,
    int n,
    int nf,
    int ne,
    int numeroRestricoes
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

    free(tabela);
    return 1;
}

int main() {

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
    scanf("%d", &numeroVariaveisDecisao);


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

    coeficienteFuncaoObjetivo =
        malloc(numeroVariaveisDecisao * sizeof(double));


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

        printf(
            "Valor da variavel de decisao x%d: ",
            i + 1
        );

        scanf(
            "%lf",
            &coeficienteFuncaoObjetivo[i]
        );
    }


    // ============================================================
    // ESCREVENDO A FUNÇÃO OBJETIVO
    // ============================================================

    fprintf(
        problemaSimplexCompleto,
        "z = "
    );


    for (i = 0; i < numeroVariaveisDecisao; i++) {

        double coeficiente =
            coeficienteFuncaoObjetivo[i];


        if (i == 0) {

            if (coeficiente < 0) {

                fprintf(
                    problemaSimplexCompleto,
                    "- %.2lfx%d",
                    -coeficiente,
                    i + 1
                );

            } else {

                fprintf(
                    problemaSimplexCompleto,
                    "%.2lfx%d",
                    coeficiente,
                    i + 1
                );
            }

        } else {

            if (coeficiente >= 0) {

                fprintf(
                    problemaSimplexCompleto,
                    " + %.2lfx%d",
                    coeficiente,
                    i + 1
                );

            } else {

                fprintf(
                    problemaSimplexCompleto,
                    " - %.2lfx%d",
                    -coeficiente,
                    i + 1
                );
            }
        }
    }


    fprintf(
        problemaSimplexCompleto,
        "\n"
    );


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

    scanf(
        "%d",
        &numeroRestricoesMenorIgual
    );


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

        matrizMenorIgual =
            malloc(
                numeroRestricoesMenorIgual *
                sizeof(double *)
            );


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

            matrizMenorIgual[i] =
                malloc(
                    (numeroVariaveisDecisao + 1) *
                    sizeof(double)
                );


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

                    printf(
                        "Coeficiente de x%d: ",
                        j + 1
                    );

                } else {

                    printf(
                        "Termo independente: "
                    );
                }


                scanf(
                    "%lf",
                    &matrizMenorIgual[i][j]
                );
            }
        }


        // --------------------------------------------------------
        // Salva as restrições <=
        // --------------------------------------------------------

        for (i = 0; i < numeroRestricoesMenorIgual; i++) {

            for (j = 0; j < numeroVariaveisDecisao; j++) {

                double coeficiente =
                    matrizMenorIgual[i][j];


                if (j == 0) {

                    if (coeficiente < 0) {

                        fprintf(
                            problemaSimplexCompleto,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );
                    }

                } else {

                    if (coeficiente >= 0) {

                        fprintf(
                            problemaSimplexCompleto,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );
                    }
                }
            }


            fprintf(
                problemaSimplexCompleto,
                " <= %.2lf\n",
                matrizMenorIgual[i][numeroVariaveisDecisao]
            );


            fprintf(
                arquivoRestricoes,
                " <= %.2lf\n",
                matrizMenorIgual[i][numeroVariaveisDecisao]
            );
        }
    }


    // ============================================================
    // RESTRIÇÕES DO TIPO =
    // ============================================================

    printf("\n");
    printf("============================================\n");
    printf("           RESTRICOES DO TIPO =\n");
    printf("============================================\n");


    printf(
        "Digite a quantidade de restricoes do tipo =: "
    );

    scanf(
        "%d",
        &numeroRestricoesIgual
    );


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

        matrizIgual =
            malloc(
                numeroRestricoesIgual *
                sizeof(double *)
            );


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

            matrizIgual[i] =
                malloc(
                    (numeroVariaveisDecisao + 1) *
                    sizeof(double)
                );


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


                scanf(
                    "%lf",
                    &matrizIgual[i][j]
                );
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

                        fprintf(
                            problemaSimplexCompleto,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );
                    }

                } else {

                    if (coeficiente >= 0) {

                        fprintf(
                            problemaSimplexCompleto,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );
                    }
                }
            }


            fprintf(
                problemaSimplexCompleto,
                " = %.2lf\n",
                matrizIgual[i][numeroVariaveisDecisao]
            );


            fprintf(
                arquivoRestricoes,
                " = %.2lf\n",
                matrizIgual[i][numeroVariaveisDecisao]
            );
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

    scanf(
        "%d",
        &numeroRestricoesMaiorIgual
    );


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

        matrizMaiorIgual =
            malloc(
                numeroRestricoesMaiorIgual *
                sizeof(double *)
            );


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
                malloc(
                    (numeroVariaveisDecisao + 1) *
                    sizeof(double)
                );


            if (matrizMaiorIgual[i] == NULL) {

                printf(
                    "\nErro ao alocar memoria para a matriz >=.\n"
                );


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
            printf(
                "Restricao >= %d\n",
                i + 1
            );


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


                scanf(
                    "%lf",
                    &matrizMaiorIgual[i][j]
                );
            }
        }


        // ========================================================
        // SALVA AS RESTRIÇÕES >=
        // ========================================================

        for (i = 0; i < numeroRestricoesMaiorIgual; i++) {

            for (j = 0; j < numeroVariaveisDecisao; j++) {

                double coeficiente =
                    matrizMaiorIgual[i][j];


                // ------------------------------------------------
                // PRIMEIRO COEFICIENTE
                // ------------------------------------------------

                if (j == 0) {

                    if (coeficiente < 0) {

                        fprintf(
                            problemaSimplexCompleto,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "- %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            "%.2lfx%d",
                            coeficiente,
                            j + 1
                        );
                    }

                }

                // ------------------------------------------------
                // DEMAIS COEFICIENTES
                // ------------------------------------------------

                else {

                    if (coeficiente >= 0) {

                        fprintf(
                            problemaSimplexCompleto,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " + %.2lfx%d",
                            coeficiente,
                            j + 1
                        );

                    } else {

                        fprintf(
                            problemaSimplexCompleto,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );

                        fprintf(
                            arquivoRestricoes,
                            " - %.2lfx%d",
                            -coeficiente,
                            j + 1
                        );
                    }
                }
            }


            // ----------------------------------------------------
            // TERMO INDEPENDENTE
            // ----------------------------------------------------

            fprintf(
                problemaSimplexCompleto,
                " >= %.2lf\n",
                matrizMaiorIgual[i][numeroVariaveisDecisao]
            );


            fprintf(
                arquivoRestricoes,
                " >= %.2lf\n",
                matrizMaiorIgual[i][numeroVariaveisDecisao]
            );
        }
    }


    // ============================================================
    // NÃO NEGATIVIDADE DAS VARIÁVEIS
    // ============================================================

    // Gera x1 >= 0, x2 >= 0, ..., xn >= 0
    for (i = 0; i < numeroVariaveisDecisao; i++)
    {
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

            printf(
                "%.2lfx%d",
                matrizMenorIgual[i][j],
                j + 1
            );
        }

        printf(
            " <= %.2lf\n",
            matrizMenorIgual[i][numeroVariaveisDecisao]
        );
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

            printf(
                "%.2lfx%d",
                matrizMaiorIgual[i][j],
                j + 1
            );
        }

        printf(
            " >= %.2lf\n",
            matrizMaiorIgual[i][numeroVariaveisDecisao]
        );
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

            printf(
                "%.2lfx%d",
                matrizIgual[i][j],
                j + 1
            );
        }

        printf(
            " = %.2lf\n",
            matrizIgual[i][numeroVariaveisDecisao]
        );
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

    const char *caminhoEntrada =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplex.txt";

    const char *caminhoSaida =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplexNaFormaPadrao.txt";

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
        if (!substituirTodas(
                linha, "<=", " + F%d = ", etapa, sizeof(etapa),
                &quantidadeVariaveisF)) {
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

    const char *caminhoTabela =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tabelaSimplex.txt";

    if (!gerarTabelaSimplex(
            caminhoSaida,
            caminhoTabela,
            numeroVariaveisDecisao,
            quantidadeVariaveisF - 1,
            quantidadeVariaveisE - 1,
            numeroRestricoesMenorIgual +
            numeroRestricoesIgual +
            numeroRestricoesMaiorIgual)) {
        printf("Erro ao gerar a tabela do simplex.\n");
        free(coeficienteFuncaoObjetivo);
        return 1;
    }

    printf("\nArquivo da tabela gerado: %s\n", caminhoTabela);

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