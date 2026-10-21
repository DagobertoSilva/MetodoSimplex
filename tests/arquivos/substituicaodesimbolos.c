
#include <stdio.h>   // Funcoes de entrada, saida e manipulacao de arquivos
#include <string.h>  // Funcoes para manipular strings

#define TAM_ENTRADA 1024
#define TAM_SAIDA 4096
#define TAM_SUBSTITUTO 50

/*
    Funcao: substituirTodas

    Objetivo:
    Encontrar todas as ocorrencias de um operador e substitui-las
    por uma variavel numerada.

    Exemplos:
    "<=" -> " + F1 = "
    ">=" -> " - E1 = "

    Retorno:
    1 = sucesso
    0 = erro ou falta de espaco
*/
int substituirTodas(
    const char entrada[],
    const char procurar[],
    const char formatoSubstituto[],
    char saida[],
    size_t capacidade,
    int *contador
) {
    // Verifica se o operador procurado esta vazio ou se a saida nao tem espaco.
    if (procurar[0] == '\0' || capacidade == 0) {
        return 0;
    }

    // Ponteiro que percorre o texto original.
    const char *atual = entrada;

    // Quantidade de caracteres escritos na saida.
    size_t usados = 0;

    // Tamanho do operador procurado.
    size_t tamanhoProcurar = strlen(procurar);

    // Inicializa a saida como uma string vazia.
    saida[0] = '\0';

    // Percorre o texto ate encontrar o terminador '\0'.
    while (*atual != '\0') {

        // Procura a proxima ocorrencia do operador.
        const char *posicao = strstr(atual, procurar);

        // Se nao encontrar mais ocorrencias, copia o restante do texto.
        if (posicao == NULL) {
            size_t restante = strlen(atual);

            // Verifica se o restante cabe, incluindo o terminador '\0'.
            if (restante >= capacidade - usados) {
                return 0;
            }

            // Copia o restante do texto para a saida.
            memcpy(saida + usados, atual, restante);

            // Atualiza a quantidade de caracteres utilizados.
            usados += restante;

            // Finaliza a string.
            saida[usados] = '\0';

            return 1;
        }

        // Calcula o tamanho do trecho anterior ao operador encontrado.
        size_t trecho = (size_t)(posicao - atual);

        // Verifica se o trecho anterior cabe na saida.
        if (trecho >= capacidade - usados) {
            return 0;
        }

        // Copia o trecho anterior ao operador para a saida.
        memcpy(saida + usados, atual, trecho);

        // Atualiza a quantidade de caracteres escritos.
        usados += trecho;

        // Finaliza temporariamente a string.
        saida[usados] = '\0';

        // Vetor que armazenara o texto substituto.
        char substituto[TAM_SUBSTITUTO];

        // Monta a substituicao com o numero atual do contador.
        // Exemplo: " + F1 = " ou " - E1 = ".
        int tamanhoSubstituto = snprintf(
            substituto,
            sizeof(substituto),
            formatoSubstituto,
            *contador
        );

        // Verifica se houve erro ao montar o texto substituto.
        if (tamanhoSubstituto < 0 ||
            (size_t)tamanhoSubstituto >= sizeof(substituto)) {
            return 0;
        }

        // Verifica se o texto substituto cabe no espaco restante.
        if ((size_t)tamanhoSubstituto >= capacidade - usados) {
            return 0;
        }

        // Copia o texto substituto para a saida.
        memcpy(
            saida + usados,
            substituto,
            (size_t)tamanhoSubstituto
        );

        // Atualiza a quantidade de caracteres utilizados.
        usados += (size_t)tamanhoSubstituto;

        // Garante que a saida seja uma string valida.
        saida[usados] = '\0';

        // Incrementa o contador para a proxima variavel.
        (*contador)++;

        // Continua a busca depois do operador original.
        atual = posicao + tamanhoProcurar;
    }

    // Informa que a substituicao terminou com sucesso.
    return 1;
}

int main(void) {

    // Contador das variaveis de folga: F1, F2, F3...
    int quantidadeVariaveisF = 1;

    // Contador das variaveis de excesso: E1, E2, E3...
    int quantidadeVariaveisE = 1;

    // Caminho do arquivo original.
    const char *caminhoArquivoBase =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\arquivos\\arquivoBase.txt";

    // Caminho do arquivo que recebera o resultado.
    const char *caminhoArquivoResultado =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\arquivos\\resultado.txt";

    // Abre o arquivo original para leitura.
    FILE *entrada = fopen(caminhoArquivoBase, "r");

    // Verifica se o arquivo original foi aberto corretamente.
    if (entrada == NULL) {
        printf("Erro ao abrir o arquivo original!\n");
        return 1;
    }

    // Abre o arquivo de resultado para escrita.
    // Se ele ja existir, seu conteudo anterior sera apagado.
    FILE *saida = fopen(caminhoArquivoResultado, "w");

    // Verifica se o arquivo de resultado foi aberto corretamente.
    if (saida == NULL) {
        printf("Erro ao criar o arquivo de resultado!\n");
        fclose(entrada);
        return 1;
    }

    // Armazena a linha lida do arquivo original.
    char linha[TAM_ENTRADA];

    // Armazena o resultado da substituicao de "<=".
    char etapa[TAM_SAIDA];

    // Armazena o resultado final apos tratar os dois operadores.
    char modificada[TAM_SAIDA];

    // Le o arquivo original linha por linha.
    while (fgets(linha, sizeof(linha), entrada) != NULL) {

        // PRIMEIRA ETAPA:
        // Substitui "<=" por variaveis de folga: F1, F2, F3...
        if (!substituirTodas(
                linha,
                "<=",
                " + F%d = ",
                etapa,
                sizeof(etapa),
                &quantidadeVariaveisF
            )) {

            printf("Erro ao substituir o operador <=!\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }

        // SEGUNDA ETAPA:
        // Substitui ">=" por variaveis de excesso: E1, E2, E3...
        if (!substituirTodas(
                etapa,
                ">=",
                " - E%d = ",
                modificada,
                sizeof(modificada),
                &quantidadeVariaveisE
            )) {

            printf("Erro ao substituir o operador >=!\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }

        // Escreve a linha final no arquivo de resultado.
        if (fputs(modificada, saida) == EOF) {
            printf("Erro ao escrever no arquivo de resultado!\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }
    }

    // Verifica se ocorreu algum erro durante a leitura.
    if (ferror(entrada)) {
        printf("Erro durante a leitura do arquivo original!\n");

        fclose(entrada);
        fclose(saida);

        return 1;
    }

    // Fecha o arquivo original.
    fclose(entrada);

    // Fecha o arquivo de resultado e verifica erros de escrita.
    if (fclose(saida) == EOF) {
        printf("Erro ao finalizar o arquivo de resultado!\n");
        return 1;
    }

    // Exibe uma mensagem de sucesso.
    printf("Substituicoes concluidas com sucesso!\n");

    // Mostra quantas variaveis F foram criadas.
    printf("Variaveis de folga criadas: %d\n",
           quantidadeVariaveisF - 1);

    // Mostra quantas variaveis E foram criadas.
    printf("Variaveis de excesso criadas: %d\n",
           quantidadeVariaveisE - 1);

    // Informa onde encontrar o resultado.
    printf("Confira o arquivo resultado.txt.\n");

    return 0;
}
