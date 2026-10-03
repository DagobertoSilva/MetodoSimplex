#include <stdio.h>
#include <stdlib.h>

int main(){

    FILE * arquivo; // ponteiro do tipo ARQUIVO(FILE)

    //arquivo = fopen("restricoes.txt", "w"); // fopen(arquivo.extensão, operação)  w é de escrita

     // Abre o arquivo no modo de escrita ("w")
    // Substitua pelo caminho do seu arquivo
    arquivo = fopen("C:\\Users\\Dagoberto\\Desktop\\MetodoSimples\\tests\\gerarGraficos\\restricoes.txt", "w");

    // Verifica se o arquivo foi aberto com sucesso
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    
    fprintf(arquivo, "Meu nome é vacalo\ntenho %d anos", 2026 - 2004); //fprintf(arquivo, texto) permite escrever um texto em um arquivo
    
    fclose(arquivo); // fecha o arquivo
    return 0;
}