#include <stdio.h>
#include <stdlib.h>

int main(){
    int numeroVariaveisDecisao, i, j;
    int numeroRestricoesMenorIgual;
    int numeroRestricoesIgual;
    int numeroRestricoesMaiorIgual;
    FILE* arquivoRestricoes;
    FILE* problemaSimplexCompleto;

  //----------------------------------------------------------------------RECEBENDO AS VARIÁVEIS DAS RETRIÇÕES NA FUNÇÃO OBJETIVO----------------------------------------------------------------------------
    printf("Digite a quantidade de variaveis de Decisao: ");
    scanf("%d", &numeroVariaveisDecisao);
    int *coeficienteFuncaoObjetivo;

    coeficienteFuncaoObjetivo = (int *) malloc(sizeof(int) * numeroVariaveisDecisao);

    if (coeficienteFuncaoObjetivo != NULL){
        for (i = 0; i < numeroVariaveisDecisao; i++)
        {
           printf("VAlor da VAriavel de decisao x%d da funcao objetivo: ", i+1);
           scanf("%d", &coeficienteFuncaoObjetivo[i]);
        }
        
    }
     
    problemaSimplexCompleto = fopen("C:\\Users\\Dagoberto\\Desktop\\MetodoSimples\\problemaSimplex.txt", "w");
    fprintf(problemaSimplexCompleto, "z = %dx1 + %dx2 \n",  coeficienteFuncaoObjetivo[0], coeficienteFuncaoObjetivo[1]);

    //----------------------------------------------------------------------RECEBENDO AS VARIÁVEIS DAS RETRIÇÕES----------------------------------------------------------------------------
    int ** matrizdeCoeficienteseTermoIndependentesMaiorIgual;
    
    printf("Digite a quantidade de Restricoes do Tipo <=: ");
    scanf("%d", &numeroRestricoesMenorIgual);

    

     // 1. Aloca o vetor de ponteiros para as linhas
    matrizdeCoeficienteseTermoIndependentesMaiorIgual =  malloc(numeroRestricoesMenorIgual * sizeof(int *));

    // 2. Aloca o vetor de elementos para cada linha
    for (i = 0; i < numeroRestricoesMenorIgual; i++) {
        matrizdeCoeficienteseTermoIndependentesMaiorIgual[i] =   malloc((numeroVariaveisDecisao + 1) * sizeof(int));
    }

    /* 3. Preenche a matriz */
    for (i = 0; i < numeroRestricoesMenorIgual; i++){
        for (j = 0; j < numeroVariaveisDecisao+1; j++)
        {
            scanf("%d", &matrizdeCoeficienteseTermoIndependentesMaiorIgual[i][j]);
        }
        
    }

    /*4. Mostrar Matriz dos coeficientes e termos independentes da restrições de menor e igual*/
      arquivoRestricoes = fopen("C:\\Users\\Dagoberto\\Desktop\\MetodoSimples\\tests\\gerarGraficos\\restricoes.txt", "r");
        for (i = 0; i < numeroRestricoesMenorIgual; i++){
            for (j = 0; j < numeroVariaveisDecisao; j++) {
                fprintf(problemaSimplexCompleto,"%dx%d", matrizdeCoeficienteseTermoIndependentesMaiorIgual[i][j],j + 1);

                if (j < numeroVariaveisDecisao - 1) {
                    fprintf(problemaSimplexCompleto, " + ");
                }
            }
             printf(" <= %d\n",matrizdeCoeficienteseTermoIndependentesMaiorIgual[i][numeroVariaveisDecisao]);
        }


    //arquivoRestricoes = fopen("C:\\Users\\Dagoberto\\Desktop\\MetodoSimples\\tests\\gerarGraficos\\restricoes.txt", "w");
   
    //fprintf(arquivoRestricoes, "%dx1 + %dx2 ≤ 20\n",  coeficienteFuncaoObjetivo[0], coeficienteFuncaoObjetivo[1]);
     //system("python -u  "); tentativa de chamar e executar arquivo python usando C
    
   // fclose(arquivoRestricoes);
    fclose(problemaSimplexCompleto);
    return 0;
}