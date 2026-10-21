
#include <stdio.h>   // Permite usar printf(), fopen(), fgets(), fputs(), fclose(), snprintf() etc.
#include <string.h>  // Permite usar strlen(), strstr() e memcpy().

// Define a quantidade máxima de caracteres que podem ser lidos por vez do arquivo.
#define TAM_ENTRADA 1024

// Define a capacidade máxima do vetor que armazenará o texto modificado.
#define TAM_SAIDA 4096

// Define a capacidade máxima do vetor que armazenará o texto substituto.
#define TAM_SUBSTITUTO 50


/*
    FUNCAO: substituirTodas

    OBJETIVO:
    Encontrar todas as ocorrências de um texto e substituí-las por variáveis de folga.

    EXEMPLO:
    Entrada:  "2x1 + 3x2 <= 100"
    Saída:    "2x1 + 3x2 + F1 = 100"

    entrada:
        Texto original que será analisado.

    procurar:
        Texto que desejamos encontrar, neste caso "<=".

    saida:
        Vetor que receberá o texto modificado.

    capacidade:
        Quantidade máxima de caracteres que cabem no vetor saida,
        incluindo o espaço necessário para o caractere '\0'.

    quantidadeVariaveisDeFolga:
        Endereço do contador que numera as variáveis F1, F2, F3 etc.

    RETORNO:
        1 = operação concluída com sucesso.
        0 = ocorreu um erro ou faltou espaço no vetor de saída.
*/

int substituirTodas(                       // Declara uma função que retorna um número inteiro.
    const char entrada[],                  // Recebe o texto original; const impede sua alteração pela função.
    const char procurar[],                 // Recebe o texto que será procurado, por exemplo, "<=".
    char saida[],                           // Recebe o endereço do vetor que guardará o resultado.
    size_t capacidade,                      // Recebe o tamanho disponível no vetor de saída.
    int *quantidadeVariaveisDeFolga          // Recebe o endereço do contador de variáveis de folga.
) {                                         // Início do corpo da função.

    // Verifica se o texto procurado está vazio ou se a saída não possui espaço.
    if (procurar[0] == '\0' || capacidade == 0) {

        return 0;                           // Retorna 0 para indicar que não foi possível executar a operação.
    }

    const char *atual = entrada;            // Cria um ponteiro que começa no primeiro caractere da entrada.

    size_t usados = 0;                      // Guarda quantos caracteres já foram colocados na saída.

    size_t tamanhoProcurar = strlen(procurar); // Calcula o tamanho do texto procurado; "<=" possui 2 caracteres.

    saida[0] = '\0';                        // Inicializa a saída como uma string vazia.

    // Continua enquanto o ponteiro atual não encontrar o fim do texto.
    while (*atual != '\0') {

        // Procura a próxima ocorrência do texto desejado a partir da posição atual.
        // strstr() retorna um ponteiro para a ocorrência encontrada ou NULL se não encontrar.
        const char *posicao = strstr(atual, procurar);

        // Verifica se não existem mais ocorrências do texto procurado.
        if (posicao == NULL) {

            // Calcula o tamanho do trecho restante da entrada.
            size_t restante = strlen(atual);

            // Verifica se o trecho restante cabe na saída, reservando espaço para '\0'.
            if (restante >= capacidade - usados) {

                return 0;                   // Retorna 0 porque não há espaço suficiente.
            }

            // Copia o trecho restante para a saída, começando na posição "usados".
            // memcpy() copia exatamente a quantidade de bytes informada.
            memcpy(saida + usados, atual, restante);

            // Atualiza a quantidade de caracteres já copiados.
            usados += restante;

            // Adiciona o terminador para que a saída seja uma string válida em C.
            saida[usados] = '\0';

            return 1;                       // Retorna 1 porque a substituição terminou com sucesso.
        }

        // Calcula quantos caracteres existem entre "atual" e a ocorrência encontrada.
        // A subtração entre ponteiros indica a distância entre as duas posições.
        size_t trecho = (size_t)(posicao - atual);

        // Verifica se o trecho anterior à ocorrência cabe na saída.
        if (trecho >= capacidade - usados) {

            return 0;                       // Retorna 0 se faltar espaço.
        }

        // Copia os caracteres anteriores ao operador encontrado para a saída.
        memcpy(saida + usados, atual, trecho);

        // Atualiza o total de caracteres escritos.
        usados += trecho;

        // Coloca o terminador no final do conteúdo atual da saída.
        saida[usados] = '\0';

        // Declara um vetor para armazenar a substituição, como " + F1 = ".
        char substituto[TAM_SUBSTITUTO];

        // Monta a string de substituição usando o número atual da variável de folga.
        // %d será substituído pelo valor inteiro do contador.
        // Exemplo: se o contador for 1, o texto será " + F1 = ".
        int tamanhoSubstituto = snprintf(
            substituto,                      // Vetor onde a string formatada será armazenada.
            sizeof(substituto),              // Capacidade total do vetor substituto.
            " + F%d = ",                     // Modelo do texto, com %d representando um inteiro.
            *quantidadeVariaveisDeFolga      // Valor do contador acessado por meio do ponteiro.
        );

        // Verifica se snprintf() falhou ou se o texto não coube no vetor.
        if (tamanhoSubstituto < 0 ||
            (size_t)tamanhoSubstituto >= sizeof(substituto)) {

            return 0;                       // Retorna 0 caso não seja possível montar a substituição.
        }

        // Verifica se o texto substituto cabe na parte restante da saída.
        if ((size_t)tamanhoSubstituto >= capacidade - usados) {

            return 0;                       // Retorna 0 se não houver espaço suficiente.
        }

        // Copia o texto substituto para a saída.
        memcpy(
            saida + usados,                  // Destino: posição disponível na saída.
            substituto,                      // Origem: texto substituto, como " + F1 = ".
            (size_t)tamanhoSubstituto        // Quantidade de caracteres que serão copiados.
        );

        // Atualiza a quantidade de caracteres escritos na saída.
        usados += (size_t)tamanhoSubstituto;

        // Garante que a saída termine com '\0' após a inserção da variável de folga.
        saida[usados] = '\0';

        // Incrementa o contador para que a próxima ocorrência receba outro número.
        // Exemplo: depois de F1, o contador passa a valer 2.
        (*quantidadeVariaveisDeFolga)++;

        // Avança o ponteiro para depois do operador encontrado na entrada original.
        // Assim, o operador "<=" não é copiado novamente para a saída.
        atual = posicao + tamanhoProcurar;
    }

    // Se o laço terminou sem encontrar erros, retorna 1 indicando sucesso.
    return 1;
}                                           // Fim da função substituirTodas.


int main() {                            // Ponto de entrada do programa.

    // Inicializa o contador em 1, pois a primeira variável será F1.
    int quantidadeVariaveisDeFolga = 1;

    // Armazena o caminho completo do arquivo original.
    // A barra invertida precisa ser escrita como "\\" dentro de uma string em C.
    const char *caminhoArquivoBase =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\arquivos\\arquivoBase.txt";

    // Armazena o caminho completo do arquivo que receberá o resultado.
    const char *caminhoArquivoResultado =
        "C:\\Users\\Dagoberto\\Desktop\\MetodoSimplex\\tests\\arquivos\\resultado.txt";

    // Abre o arquivo original no modo "r", que significa leitura.
    FILE *entrada = fopen(caminhoArquivoBase, "r");

    // Verifica se a abertura do arquivo falhou.
    if (entrada == NULL) {

        // Exibe uma mensagem caso o arquivo original não possa ser aberto.
        printf("Erro ao abrir o arquivo original!\n");

        return 1;                           // Encerra o programa indicando erro.
    }

    // Abre o arquivo de resultado no modo "w", que permite escrita.
    // Se o arquivo já existir, seu conteúdo anterior será apagado.
    FILE *saida = fopen(caminhoArquivoResultado, "w");

    // Verifica se o arquivo de resultado foi aberto corretamente.
    if (saida == NULL) {

        // Informa que não foi possível criar ou abrir o arquivo de saída.
        printf("Erro ao criar o arquivo de resultado!\n");

        fclose(entrada);                    // Fecha o arquivo original para liberar o recurso.

        return 1;                           // Encerra o programa indicando erro.
    }

    // Declara um vetor que armazenará cada trecho lido do arquivo.
    char linha[TAM_ENTRADA];

    // Declara um vetor que armazenará o conteúdo após as substituições.
    char modificada[TAM_SAIDA];

    // Lê o arquivo original linha por linha.
    // fgets() termina quando encontra uma quebra de linha, chega ao fim do arquivo
    // ou lê o limite de caracteres disponível no vetor.
    while (fgets(linha, sizeof(linha), entrada) != NULL) {

        // Chama a função para substituir todas as ocorrências de "<=".
        // Cada ocorrência recebe uma variável numerada: F1, F2, F3 etc.
        if (!substituirTodas(
                linha,                      // Texto original da linha atual.
                "<=",                       // Operador que será procurado.
                modificada,                  // Vetor que receberá a linha modificada.
                sizeof(modificada),          // Capacidade total do vetor modificada.
                &quantidadeVariaveisDeFolga  // Passa o endereço do contador para a função.
            )) {                             // Entra aqui se a função retornar 0.

            // Exibe uma mensagem caso ocorra um erro na substituição.
            printf("Erro na substituicao: linha muito grande "
                   "ou falta de espaco.\n");

            fclose(entrada);                // Fecha o arquivo original.
            fclose(saida);                  // Fecha o arquivo de resultado.

            return 1;                       // Encerra o programa indicando erro.
        }

        // Escreve a linha modificada no arquivo de resultado.
        // fputs() retorna EOF quando ocorre um erro de escrita.
        if (fputs(modificada, saida) == EOF) {

            // Informa que não foi possível gravar o texto.
            printf("Erro ao escrever no arquivo de resultado!\n");

            fclose(entrada);                // Fecha o arquivo original.
            fclose(saida);                  // Fecha o arquivo de resultado.

            return 1;                       // Encerra o programa indicando erro.
        }
    }

    // Verifica se o laço terminou por causa de um erro de leitura.
    if (ferror(entrada)) {

        // Exibe uma mensagem caso tenha ocorrido um erro ao ler o arquivo.
        printf("Erro durante a leitura do arquivo original!\n");

        fclose(entrada);                    // Fecha o arquivo original.
        fclose(saida);                      // Fecha o arquivo de resultado.

        return 1;                           // Encerra o programa indicando erro.
    }

    // Fecha o arquivo original após terminar a leitura.
    fclose(entrada);

    // Fecha o arquivo de resultado e verifica se houve erro ao finalizar a escrita.
    if (fclose(saida) == EOF) {

        // Informa que houve um problema ao finalizar o arquivo.
        printf("Erro ao finalizar o arquivo de resultado!\n");

        return 1;                           // Encerra o programa indicando erro.
    }

    // Informa que todas as substituições foram concluídas.
    printf("Substituicoes concluidas com sucesso!\n");

    // Exibe quantas variáveis foram criadas.
    // Subtrai 1 porque o contador aponta para o próximo número disponível.
    printf("Quantidade de variaveis de folga criadas: %d\n",
           quantidadeVariaveisDeFolga - 1);

    // Informa onde o usuário pode conferir o resultado.
    printf("Confira o arquivo resultado.txt.\n");

    return 0;                               // Encerra o programa indicando sucesso.
}                                           // Fim da função main.
