#include <stdio.h>
#include <stdlib.h>

int main(){
    int numeroVariaveisDecisao, i;
    FILE* arquivoRestricoes;

   // arquivoRestricoes = fopen(C:/Users/Dagoberto/Desktop/MetodoSimples/tests);


    printf("Digite a quantidade de variaveis de Decisao: ");
    scanf("%d", &numeroVariaveisDecisao);
    int *coeficienteFuncaoObjetivo;

    coeficienteFuncaoObjetivo = (int *) malloc(sizeof(int) * numeroVariaveisDecisao);
    if (coeficienteFuncaoObjetivo != NULL)
    {
        for (i = 0; i < numeroVariaveisDecisao; i++)
        {
           printf("VAlor da VAriavel de decisao x%d: ", i+1);
           scanf("%d", &coeficienteFuncaoObjetivo[i]);
        }
        
    }

    for (i = 0; i < numeroVariaveisDecisao; i++)
        {
           printf("VAlor da VAriavel de decisao x%d: %d", i+1, coeficienteFuncaoObjetivo[i]);
       
        }
    

    return 0;
}