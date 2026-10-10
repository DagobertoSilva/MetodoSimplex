#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        printf("C:\\Users\\Dagoberto\\Desktop\\MetodoSimples\\tests\\gerarGraficos\n");

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


//-------------------------------------deixando  as restrições na forma padrão----------------------------------
 FILE *arquivoEntrada;
    FILE *arquivoSaida;

    char linha[500];
    int numeroVariavelFolga = 0;

    const char *caminhoEntrada =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplex.txt";

    const char *caminhoSaida =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\problemaSimplexNaFormaPadrao.txt";

    // Abre o arquivo original para leitura
    arquivoEntrada = fopen(caminhoEntrada, "r");

    if (arquivoEntrada == NULL) {
        printf("Erro ao abrir o arquivo de entrada.\n");
        return 1;
    }

    // Cria o arquivo que recebera as restricoes transformadas
    arquivoSaida = fopen(caminhoSaida, "w");

    if (arquivoSaida == NULL) {
        printf("Erro ao criar o arquivo de saida.\n");
        fclose(arquivoEntrada);
        return 1;
    }

    // Percorre todas as linhas do arquivo original
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL) {
        char *operador = strstr(linha, "<=");

        // Verifica se a linha possui o operador <=
        if (operador != NULL) {
            char ladoEsquerdo[400];
            char ladoDireito[400];

            // Copia a parte anterior ao operador
            size_t tamanhoEsquerdo = (size_t)(operador - linha);

            if (tamanhoEsquerdo >= sizeof(ladoEsquerdo)) {
                printf("Restricao muito grande.\n");
                fclose(arquivoEntrada);
                fclose(arquivoSaida);
                return 1;
            }

            memcpy(ladoEsquerdo, linha, tamanhoEsquerdo);
            ladoEsquerdo[tamanhoEsquerdo] = '\0';

            // Remove os espacos do final do lado esquerdo
            while (tamanhoEsquerdo > 0 &&
                   ladoEsquerdo[tamanhoEsquerdo - 1] == ' ') {
                ladoEsquerdo[--tamanhoEsquerdo] = '\0';
            }

            // Copia a parte posterior ao operador <=
            snprintf(
                ladoDireito,
                sizeof(ladoDireito),
                "%s",
                operador + 2
            );

            // Remove os espacos do inicio do lado direito
            char *inicioDireito = ladoDireito;

            while (*inicioDireito == ' ' ||
                   *inicioDireito == '\t') {
                inicioDireito++;
            }

            // Remove a quebra de linha para reconstruir a restricao
            inicioDireito[strcspn(inicioDireito, "\r\n")] = '\0';

            // Adiciona uma variavel de folga e transforma <= em =
            numeroVariavelFolga++;

            fprintf(
                arquivoSaida,
                "%s + vf%d = %s\n",
                ladoEsquerdo,
                numeroVariavelFolga,
                inicioDireito
            );

        } else {
            // Mantem as linhas que nao possuem <=
            fputs(linha, arquivoSaida);
        }
    }

  

    printf("Transformacao concluida com sucesso!\n");
    printf("Restricoes transformadas: %d\n", numeroVariavelFolga);
    printf("Arquivo gerado:\n%s\n", caminhoSaida);





printf("\n");
printf("============================================\n");

    // ============================================================
    // FECHANDO OS ARQUIVOS
    //
    // IMPORTANTE:
    // Os arquivos só são fechados aqui, depois de todas
    // as operações de escrita.
    // ============================================================

    fclose(arquivoRestricoes);

    fclose(problemaSimplexCompleto);


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