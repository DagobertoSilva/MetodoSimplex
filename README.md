# 📐 Implementação Computacional do Método Simplex (Duas Fases)

Implementação do Método Simplex em Duas Fases para a resolução de problemas de Programação Linear (PL), desenvolvida na linguagem C para a disciplina de Pesquisa Operacional da Universidade Federal do Ceará (UFC - Campus Crateús).

O projeto consiste na criação de um programa via terminal capaz de ler o modelo matemático a partir de um ficheiro de texto, realizar a conversão das restrições ($\le$, $=$, $\ge$), executar as Fases I e II do Simplex e apresentar os tableaux e o resultado final de forma detalhada.

## 📌 Sumário

- [Sobre o Projeto](#-sobre-o-projeto)
- [Objetivos](#-objetivos)
- [Escopo do Trabalho](#-escopo-do-trabalho)
- [Requisitos da Implementação](#️-requisitos-da-implementação)
- [Formato do Ficheiro de Entrada](#-formato-do-ficheiro-de-entrada)
- [Saída e Exibição dos Tableaux](#-saída-e-exibição-dos-tableaux)
- [Estratégia de Testes e Validação](#-estratégia-de-testes-e-validação)
- [Relatório Técnico e Regras de IA](#-relatório-técnico-e-regras-de-ia)
- [Estrutura do Projeto](#️-estrutura-do-projeto)
- [Como Compilar e Executar](#️-como-compilar-e-executar)
- [Equipa e Avaliação](#-equipa-e-avaliação)

## 📖 Sobre o Projeto

Este projeto aborda a resolução computacional de problemas de Programação Linear na forma geral:

$$\text{Maximizar } Z = c_1 x_1 + c_2 x_2 + \dots + c_n x_n$$

Sujeito a restrições dos tipos:

- $a_{i1}x_1 + a_{i2}x_2 + \dots + a_{in}x_n \le b_i$
- $a_{i1}x_1 + a_{i2}x_2 + \dots + a_{in}x_n = b_i$
- $a_{i1}x_1 + a_{i2}x_2 + \dots + a_{in}x_n \ge b_i$
- $x_j \ge 0 \quad (\forall j = 1, \dots, n)$

Para tratar restrições dos tipos $\ge$ e $=$, o software utiliza a Fase I do Método Simplex com a adição de variáveis artificiais para encontrar uma Solução Básica Viável (SBV) inicial. Encontrada a SBV, o algoritmo elimina as variáveis artificiais e avança para a Fase II, buscando a solução ótima do problema original.

## 🎯 Objetivos

### Objetivo Geral

Implementar computacionalmente as Fases I e II do Método Simplex para resolver problemas de Programação Linear com restrições mistas via ficheiro de entrada no terminal.

### Objetivos Específicos

- Representar computacionalmente modelos de Programação Linear e as suas matrizes associadas.
- Tratar e introduzir automaticamente variáveis de folga, excesso e artificiais.
- Implementar as operações matriciais de pivoteamento e o teste da razão estritamente em C.
- Identificar com precisão o status da solução: Solução Ótima, Problema Inviável ou Solução Ilimitada.
- Exibir a evolução passo a passo do algoritmo por meio da impressão formatada do tableau.

## 📦 Escopo do Trabalho

- **Interface:** execução exclusiva via linha de comandos/terminal, sem interface gráfica.
- **Entrada/Saída:** leitura de ficheiros de texto (`.txt`) e geração de resultados diretamente no terminal.
- **Formato de Entrega:** ficheiro compactado (`.zip`) enviado via SIGAA contendo o código-fonte, relatório técnico em PDF, instruções de compilação/execução, ficheiros de teste e scripts.

## ⚙️ Requisitos da Implementação

A implementação atende aos seguintes requisitos funcionais e estruturais:

- [x] Leitura e parsing do ficheiro de entrada no formato especificado.
- [x] Introdução automática de variáveis de folga ($\le$), excesso ($\ge$) e artificiais ($=$, $\ge$).
- [x] Execução e controlo da Fase I do Simplex.
- [x] Deteção de inviabilidade (variável artificial a permanecer na base com valor positivo no fim da Fase I).
- [x] Remoção de variáveis artificiais e transição para a Fase II.
- [x] Execução da Fase II a partir da base viável.
- [x] Identificação de solução ótima ou solução ilimitada.
- [x] Apresentação do tableau a cada iteração de ambas as fases.
- [x] Exibição clara da transição de fase e do resumo final da solução.

## 📄 Formato do Ficheiro de Entrada

O programa processa ficheiros de entrada estruturados no padrão exigido:

1. **Linha 1:** inteiro $N$ (número de variáveis de decisão).
2. **Linha 2:** $N$ números reais (coeficientes da função objetivo).
3. **Linha 3:** inteiro $B$ (quantidade de restrições do tipo $\le$).
4. **Próximas $B$ linhas:** coeficientes das variáveis e o termo independente ($RHS$) de cada restrição $\le$.
5. **Linha seguinte:** inteiro $C$ (quantidade de restrições do tipo $=$).
6. **Próximas $C$ linhas:** coeficientes das variáveis e o termo independente ($RHS$) de cada restrição $=$.
7. **Linha seguinte:** inteiro $D$ (quantidade de restrições do tipo $\ge$).
8. **Próximas $D$ linhas:** coeficientes das variáveis e o termo independente ($RHS$) de cada restrição $\ge$.

### Exemplo de Entrada (`entrada.txt`)

```text
2
4 3
1
1 3 20
1
1 -1 0
1
2 -1 1
```

Representa o problema: Maximizar $Z = 4x_1 + 3x_2$, sujeito a $x_1 + 3x_2 \le 20$, $x_1 - x_2 = 0$, $2x_1 - x_2 \ge 1$ e $x_1, x_2 \ge 0$.

## 📊 Saída e Exibição dos Tableaux

A saída gerada no terminal detalha o progresso da resolução:

- **Tableaux das Iterações:** impressão organizada a cada passo, identificando a linha/coluna pivô e a fase atual (FASE I ou FASE II).
- **Indicação de Transição:** destaque visual a informar o término da Fase I e o início da Fase II.
- **Resumo da Solução:**
  - **Status:** Solução Ótima Encontrada / Problema Inviável / Solução Ilimitada.
  - **Valor Ótimo ($Z$):** valor final da função objetivo.
  - **Valores das Variáveis de Decisão:** valores assumidos por $x_1, x_2, \dots, x_n$.
  - **Número Total de Iterações:** soma das iterações da Fase I e Fase II.

## 🧪 Estratégia de Testes e Validação

O software é validado com um conjunto de pelo menos 5 problemas de Programação Linear, cobrindo os seguintes cenários:

1. Problemas resolvidos diretamente na Fase II (apenas restrições $\le$).
2. Problemas que exigem a Fase I (presença de restrições $\ge$ e $=$).
3. Deteção de Inviabilidade (Fase I encerra com custo $Z > 0$).
4. Deteção de Solução Ilimitada (todas as entradas da coluna pivô são $\le 0$ no teste da razão).
5. Problemas com múltiplas variáveis de decisão e coeficientes fracionários/decimais.

## 📑 Relatório Técnico e Regras de IA

A entrega acompanha um relatório académico formatado segundo as normas, contendo:

- Capa e Sumário
- Introdução e Fundamentação Teórica
- Método Experimental
- Resultados e Discussão (comparações e tabelas)
- Conclusão e Referências
- **Secção "Uso de ferramentas de IA":** conforme a Portaria N.º 39/PRPPG/UFC de 01/10/2025, o relatório declara o uso transparente de IA generativa (ferramentas, finalidades e prompts). É vedado o uso de IA para a geração de análises críticas, redação de secções substantivas, manipulação de dados ou plágio.

## 🏗️ Estrutura do Projeto

```text
simplex-c/
├── src/                  # Código-fonte em C (.c)
├── include/              # Ficheiros de cabeçalho (.h)
├── data/                 # Ficheiros de entrada para testes (*.txt)
├── docs/                 # Relatório técnico (PDF) e documentação
├── Makefile              # Automação de compilação
└── README.md             # Documentação do repositório
```

## ▶️ Como Compilar e Executar

### Pré-requisitos

Compilador C (GCC recomendável) instalado no sistema.

### Compilação

Utilizando o GCC diretamente no terminal:

```bash
gcc -Wall src/*.c -Iinclude -o simplex
```

Ou utilizando o Makefile (se disponível):

```bash
make
```

### Execução

Passe o caminho do ficheiro de entrada como argumento no terminal:

**Linux / macOS:**

```bash
./simplex data/entrada.txt
```

**Windows (PowerShell / CMD):**

```bash
.\simplex.exe data\entrada.txt
```

## 👨‍💻 Equipa e Avaliação

- **Disciplina:** Pesquisa Operacional
- **Professor:** Prof. Rafael Martins Barros
- **Instituição:** Universidade Federal do Ceará (UFC) — Campus Crateús
- **Curso:** Sistemas de Informação / Ciência da Computação

### Integrantes do Grupo (até 3 membros)

- Dagoberto Silva — Graduando em Ciência da Computação
- [Nome do Integrante 2] — Curso
- [Nome do Integrante 3] — Curso
