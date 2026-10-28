#ifndef DAGOSHOWSIMPLEX_H
#define DAGOSHOWSIMPLEX_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#define TAM_ENTRADA 1024
#define TAM_SAIDA 4096
#define TAM_SUBSTITUTO 50

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

/* ---------- Transformacao para a forma padrao ---------- */
int substituirTodas(const char entrada[], const char procurar[],
                    const char formatoSubstituto[], char saida[],
                    size_t capacidade, int *contador);
int ehRestricaoNaoNegatividade(const char linha[]);
int ehFuncaoObjetivo(const char linha[]);
int escreverFuncaoObjetivoPadrao(FILE *arquivo, const double coeficientes[],
                                 int quantidade, int quantidadeF,
                                 int quantidadeE);

/* ---------- Leitura e impressao da tabela inicial ---------- */
void lerLinhaForma(const char linha[], double destino[], int n, int nf, int ne);
void nomeColuna(char nome[], size_t tamanho, int coluna, int n, int nf);
void imprimirTabela(FILE *saida, const double tabela[], int linhasTabela,
                    int n, int nf, int ne);

/* ---------- Metodo Simplex em duas fases ---------- */
void emitir(FILE *log, const char *formato, ...);
double limpar(double v);
void nomeVariavel(const Simplex *s, int col, char nome[], size_t tam);
void imprimirTableau(FILE *f, const Simplex *s, int fase, const double razao[]);
void emitirTableau(FILE *log, const Simplex *s, int fase, const double razao[]);
void pivotear(Simplex *s, int lin, int col);
int executarFase(Simplex *s, int fase, FILE *log);
int resolverSimplex(const double tabela[], int m0, int n, int nf, int ne,
                    const char *caminhoSolucao);
int gerarTabelaSimplex(const char *caminhoEntrada, const char *caminhoSaida,
                       int n, int nf, int ne, int numeroRestricoes,
                       const char *caminhoSolucao);

/* ---------- Entrada ---------- */
void lerEntrada(const char *formato, void *destino);

#endif /* BIBLIOTECA_H */