# 📐 Implementação do Método Simplex em C

Implementação do Método Simplex para Programação Linear, desenvolvida em C, com foco em aprendizado de Pesquisa Operacional, algoritmos, estruturas de dados, álgebra linear, manipulação de matrizes e engenharia de software.

O projeto tem como objetivo transformar a formulação matemática de um problema de Programação Linear em uma implementação computacional capaz de executar as etapas do algoritmo Simplex, apresentando as iterações e a solução encontrada.

🎓 Projeto acadêmico e de portfólio desenvolvido para aprofundar conhecimentos em Programação Linear, Método Simplex, C e desenvolvimento de algoritmos.

---

## 📌 Sumário

- [Sobre o Projeto](#-sobre-o-projeto)
- [Objetivos](#-objetivos)
- [O que é o Método Simplex](#-o-que-é-o-método-simplex)
- [Exemplo do Problema](#-exemplo-do-problema)
- [Funcionamento do Algoritmo](#-funcionamento-do-algoritmo)
- [Escopo do Projeto](#-escopo-do-projeto)
- [Funcionalidades](#️-funcionalidades)
- [Arquitetura](#️-arquitetura)
- [Estrutura de Diretórios](#-estrutura-de-diretórios)
- [Tecnologias](#️-tecnologias)
- [Conceitos Estudados](#-conceitos-estudados)
- [Requisitos Funcionais](#-requisitos-funcionais)
- [Requisitos Não Funcionais](#️-requisitos-não-funcionais)
- [Precisão Numérica](#-precisão-numérica)
- [Casos Especiais](#️-casos-especiais)
- [Testes](#-testes)
- [Estratégia de Desenvolvimento](#-estratégia-de-desenvolvimento)
- [Git e GitHub](#-git-e-github)
- [Padrão de Commits](#-padrão-de-commits)
- [Roadmap](#-roadmap)
- [Documentação](#-documentação)
- [Possíveis Evoluções](#-possíveis-evoluções)
- [Como Executar](#️-como-executar)
- [Exemplo de Execução](#-exemplo-de-execução)
- [Aprendizados](#-aprendizados)
- [Checklist](#-checklist-do-projeto)
- [Licença](#-licença)
- [Autor](#-autor)

---

## 📖 Sobre o Projeto

Este projeto implementa o Método Simplex, um dos principais algoritmos utilizados para resolver problemas de Programação Linear (PL).

A proposta não é apenas criar um programa que encontre uma resposta, mas desenvolver uma implementação que permita compreender:

* Como um problema matemático é representado computacionalmente;
* Como uma matriz pode representar um problema de Programação Linear;
* Como funciona um tableau Simplex;
* Como uma variável entra na base;
* Como uma variável sai da base;
* Como funciona o teste da razão;
* Como ocorre o pivoteamento;
* Como as iterações são realizadas;
* Como identificar uma solução ótima;
* Como identificar situações como solução ilimitada ou problema inviável;
* Como transformar conceitos matemáticos em estruturas e algoritmos em C.

O projeto será desenvolvido de forma incremental, permitindo que cada etapa seja testada e documentada antes da implementação de novos recursos.

## 🎯 Objetivos

### Objetivo Geral
Desenvolver, em C, uma implementação do Método Simplex para problemas de Programação Linear, utilizando estruturas de dados e operações matriciais para representar e resolver os problemas.

### Objetivos Específicos
* Compreender os fundamentos da Programação Linear.
* Compreender matematicamente o Método Simplex.
* Implementar o tableau Simplex.
* Trabalhar com matrizes dinamicamente.
* Implementar operações de pivoteamento.
* Implementar o teste da razão.
* Identificar a variável que entra na base.
* Identificar a variável que sai da base.
* Implementar o critério de optimalidade.
* Identificar soluções ilimitadas.
* Trabalhar com números reais utilizando `double`.
* Implementar testes automatizados.
* Organizar o projeto seguindo boas práticas de engenharia de software.
* Utilizar Git e GitHub durante todo o desenvolvimento.
* Documentar decisões matemáticas e técnicas.
* Evoluir posteriormente para casos mais complexos.

## 🧮 O que é o Método Simplex

O Método Simplex é um algoritmo utilizado para resolver problemas de Programação Linear.
Um problema de Programação Linear normalmente possui:

**Variáveis de decisão**
São as incógnitas que queremos determinar.
Exemplo:
* $x_1$
* $x_2$

**Função objetivo**
Representa aquilo que queremos maximizar ou minimizar.
Exemplo:
$$Max Z = 3x_1 + 5x_2$$

**Restrições**
Representam as limitações do problema.
Exemplo:
$$x_1 + 2x_2 \leq 8$$
$$3x_1 + 2x_2 \leq 12$$

**Restrições de não negatividade**
$$x_1 \geq 0$$
$$x_2 \geq 0$$

O Simplex percorre soluções básicas factíveis, realizando operações matemáticas até encontrar uma solução ótima, quando ela existe dentro das condições consideradas.

## 📊 Exemplo do Problema

Para a primeira versão do projeto será utilizado um problema simples de maximização:

**Maximizar:**
$$Z = 3x_1 + 5x_2$$

**Sujeito a:**
$$x_1 + 2x_2 \leq 8$$
$$3x_1 + 2x_2 \leq 12$$
$$x_1 \geq 0, x_2 \geq 0$$

### ➕ Adicionando variáveis de folga
Para transformar as restrições em igualdades:

$$x_1 + 2x_2 + s_1 = 8$$
$$3x_1 + 2x_2 + s_2 = 12$$

Onde $s_1 \geq 0$ e $s_2 \geq 0$ são as variáveis de folga.

### 📋 Tableau Inicial
Uma representação possível do tableau é:

```text
       x₁   x₂   s₁   s₂   RHS
s₁      1    2    1    0    8
s₂      3    2    0    1   12
Z      -3   -5    0    0    0
```

Nesse projeto, essa convenção será utilizada inicialmente para problemas de maximização. A escolha de sinais e a forma de representação do tableau são uma convenção de implementação. O projeto deverá documentar essa convenção para que todas as funções trabalhem de maneira consistente.

## 🔄 Funcionamento do Algoritmo

De maneira simplificada:

```text
Problema de Programação Linear
            ↓
      Modelo matemático
            ↓
   Transformação para tableau
            ↓
   Escolha da variável de entrada
            ↓
      Teste da razão
            ↓
   Escolha da variável de saída
            ↓
        Pivoteamento
            ↓
       Novo tableau
            ↓
    Critério de optimalidade
        ↙           ↘
     Não ótimo       Ótimo
       ↓              ↓
   Nova iteração     Solução
```

### 🔎 Etapas Principais

1. **Criar o problema**: O programa recebe quantidade de variáveis, quantidade de restrições, coeficientes, termos independentes e coeficientes da função objetivo.
2. **Criar o tableau**: O modelo matemático é convertido para uma matriz utilizada pelo algoritmo.
3. **Escolher a variável que entra**: Na convenção utilizada inicialmente, para maximização, procura-se um coeficiente negativo na linha da função objetivo. A variável associada ao coeficiente escolhido entra na base.
4. **Teste da razão**: Para determinar qual variável deverá sair da base: $razão = \frac{RHS}{\text{elemento da coluna pivô}}$. São consideradas apenas as linhas cujo elemento da coluna pivô seja positivo. A menor razão positiva determina a linha pivô.
5. **Pivoteamento**: Após determinar linha pivô e coluna pivô o tableau é transformado por operações elementares. O elemento localizado na interseção é o elemento pivô.
6. **Nova iteração**: Após o pivoteamento, o algoritmo verifica novamente a linha da função objetivo. Se ainda houver possibilidade de melhoria, uma nova iteração é executada.
7. **Critério de parada**: Na convenção inicial, quando não existem mais coeficientes negativos relevantes na linha da função objetivo, o algoritmo considera que encontrou uma solução ótima.

## 📦 Escopo do Projeto

O projeto será desenvolvido em versões incrementais.

**Versão inicial — v0.1.0**
A primeira versão terá como objetivo implementar:
* Maximização;
* Restrições $\leq$;
* Variáveis não negativas;
* Coeficientes reais;
* Tableau inicial;
* Escolha da coluna pivô;
* Teste da razão;
* Escolha da linha pivô;
* Operação de pivoteamento;
* Critério de optimalidade;
* Obtenção da solução.

**Fora do escopo inicial**
Minimização direta, restrições $\geq$, restrições $=$, Big M, Método das Duas Fases, entrada complexa, interface gráfica. Esses recursos serão adicionados posteriormente.

## ⚙️ Funcionalidades

### Implementadas
- [x] Representação de um problema de Programação Linear
- [x] Criação do tableau
- [x] Exibição do tableau
- [x] Seleção da coluna pivô
- [x] Teste da razão
- [x] Seleção da linha pivô
- [x] Pivoteamento
- [x] Critério de optimalidade
- [x] Extração da solução
- [x] Exibição das iterações

### Planejadas
- [ ] Entrada dinâmica pelo terminal
- [ ] Leitura de problemas através de arquivos
- [ ] Detecção de solução ilimitada
- [ ] Detecção de problema inviável
- [ ] Detecção de múltiplas soluções ótimas
- [ ] Tratamento de degeneração
- [ ] Problemas de minimização
- [ ] Restrições $\geq$
- [ ] Restrições $=$
- [ ] Variáveis artificiais
- [ ] Método Big M
- [ ] Método das Duas Fases
- [ ] Testes automatizados
- [ ] Relatório detalhado das iterações

## 🏗️ Arquitetura

A implementação será organizada por responsabilidades.

```text
                 ┌──────────────┐
                 │    main.c    │
                 └──────┬───────┘
                        │
              ┌─────────▼─────────┐
              │      input        │
              └─────────┬─────────┘
                        │
              ┌─────────▼─────────┐
              │       model       │
              └─────────┬─────────┘
                        │
              ┌─────────▼─────────┐
              │      tableau      │
              └─────────┬─────────┘
                        │
              ┌─────────▼─────────┐
              │      simplex      │
              └─────────┬─────────┘
                        │
              ┌─────────▼─────────┐
              │      output       │
              └───────────────────┘
```

* **main.c**: Responsável por coordenar a execução.
* **input**: Responsável pela entrada dos dados.
* **model**: Representa matematicamente o problema.
* **tableau**: Responsável pela estrutura e operações da matriz Simplex.
* **simplex**: Contém a lógica principal do algoritmo.
* **output**: Responsável pela apresentação dos resultados e das iterações.

## 📁 Estrutura de Diretórios

Estrutura planejada:

```text
simplex-c/
│
├── src/
│   ├── main.c
│   ├── simplex.c
│   ├── tableau.c
│   ├── model.c
│   ├── input.c
│   └── output.c
│
├── include/
│   ├── simplex.h
│   ├── tableau.h
│   ├── model.h
│   ├── input.h
│   └── output.h
│
├── tests/
│   ├── test_simplex.c
│   ├── test_tableau.c
│   └── test_model.c
│
├── data/
│   └── problemas/
│
├── examples/
│
├── docs/
│   ├── mathematics.md
│   ├── simplex-algorithm.md
│   ├── tableau.md
│   ├── pivoting.md
│   ├── architecture.md
│   └── decisions.md
│
├── .gitignore
├── README.md
├── CHANGELOG.md
├── LICENSE
└── Makefile
```

## 🛠️ Tecnologias

**Linguagem:** C
**Ferramentas:** GCC, Visual Studio Code, Git, GitHub
**Conceitos computacionais:** Matrizes, Structs, Ponteiros, Alocação dinâmica, Funções, Modularização, Manipulação de memória, Arquivos, Testes, Algoritmos numéricos

## 🧠 Conceitos Estudados

O projeto envolve conhecimentos de diferentes áreas:

* **Matemática:** Álgebra linear, Matrizes, Sistemas de equações, Inequações, Função objetivo, Região factível, Solução ótima.
* **Pesquisa Operacional:** Programação Linear, Método Simplex, Variáveis de decisão, Variáveis de folga, Solução básica, Base, Pivoteamento, Degeneração, Solução ilimitada, Problema inviável.
* **Programação:** C, Structs, Ponteiros, Matrizes, Alocação dinâmica, Modularização, Manipulação de arquivos.
* **Engenharia de Software:** Arquitetura, Separação de responsabilidades, Testes, Versionamento, Documentação, Controle de mudanças.

## 📋 Requisitos Funcionais

| ID   | Requisito |
|------|-----------|
| RF01 | Representar um problema de Programação Linear |
| RF02 | Armazenar variáveis de decisão |
| RF03 | Armazenar restrições |
| RF04 | Armazenar função objetivo |
| RF05 | Criar tableau inicial |
| RF06 | Exibir tableau |
| RF07 | Identificar coluna pivô |
| RF08 | Executar teste da razão |
| RF09 | Identificar linha pivô |
| RF10 | Executar pivoteamento |
| RF11 | Atualizar o tableau |
| RF12 | Verificar optimalidade |
| RF13 | Executar múltiplas iterações |
| RF14 | Obter valores das variáveis |
| RF15 | Exibir valor da função objetivo |
| RF16 | Detectar solução ilimitada |
| RF17 | Registrar iterações |

## ⚙️ Requisitos Não Funcionais

* **RNF01 — Precisão**: O programa deverá utilizar `double` para representar valores reais.
* **RNF02 — Portabilidade**: O código deverá buscar compatibilidade com compiladores C padrão.
* **RNF03 — Organização**: A lógica matemática deverá ser separada da interface de entrada e saída.
* **RNF04 — Manutenibilidade**: As funções deverão possuir responsabilidades bem definidas.
* **RNF05 — Testabilidade**: As principais operações matemáticas deverão poder ser testadas individualmente.
* **RNF06 — Transparência**: O programa deverá permitir visualizar as etapas do algoritmo, facilitando a compreensão do funcionamento do Simplex.

## 🔢 Precisão Numérica

Como o Simplex trabalha com números reais, o projeto utilizará: `double`.
Comparações diretas entre números de ponto flutuante deverão ser evitadas quando apropriado. Será utilizado um valor de tolerância, por exemplo: $\varepsilon$ para determinar quando um valor pode ser considerado suficientemente próximo de zero. A tolerância deverá ser documentada e testada, pois uma escolha inadequada pode causar decisões incorretas durante o algoritmo.

## ⚠️ Casos Especiais

Uma implementação completa do Simplex precisa considerar situações que vão além do caso básico:
* **Solução ótima:** Existe uma solução factível que satisfaz o critério de optimalidade.
* **Solução ilimitada:** O valor da função objetivo pode continuar melhorando indefinidamente.
* **Problema inviável:** Não existe solução que satisfaça simultaneamente todas as restrições.
* **Múltiplas soluções ótimas:** Mais de uma solução pode produzir o mesmo valor ótimo da função objetivo.
* **Degeneração:** Pode ocorrer quando uma solução básica possui uma variável básica com valor zero.

## 🧪 Testes

Os testes serão desenvolvidos junto com as funcionalidades.
* **Teste do tableau:** Verificar se o tableau inicial foi construído corretamente.
* **Teste da coluna pivô:** Verificar se a variável de entrada foi identificada corretamente.
* **Teste da razão:** Verificar $\frac{RHS}{\text{coeficiente da coluna pivô}}$ e a seleção da menor razão válida.
* **Teste de pivoteamento:** Verificar se as operações de linha produzem o tableau esperado.
* **Teste de optimalidade:** Verificar se o algoritmo identifica corretamente quando não é necessária uma nova iteração.
* **Teste de solução:** Comparar a solução encontrada pelo programa com uma solução previamente conhecida.
* **Testes de casos especiais:** Serão adicionados casos para solução ilimitada, problema inviável, múltiplas soluções, degeneração e valores próximos de zero.

## 🔬 Estratégia de Desenvolvimento

O projeto seguirá uma abordagem incremental:

* **Fase 1 — Fundamentos matemáticos:** Entender Programação Linear, solução factível, variáveis de folga, tableau, base e pivoteamento.
* **Fase 2 — Estrutura do projeto:** Criar diretórios e configurar Git e GitHub.
* **Fase 3 — Modelo matemático:** Implementar estruturas capazes de representar variáveis, restrições, coeficientes, RHS e função objetivo.
* **Fase 4 — Tableau:** Implementar criação, armazenamento, acesso, impressão e liberação da memória.
* **Fase 5 — Algoritmo Simplex:** Implementar escolha da coluna/linha pivô, teste da razão, pivoteamento, atualização e critério de parada.
* **Fase 6 — Testes:** Criar problemas conhecidos e comparar os resultados.
* **Fase 7 — Casos especiais:** Adicionar ilimitado, inviável, degeneração, múltiplas soluções.
* **Fase 8 — Expansão:** Adicionar $\geq$, $=$, minimização, variáveis artificiais, Big M e Duas Fases.

## 🌿 Git e GitHub

O Git será utilizado desde o início do projeto. Cada funcionalidade importante deverá ser registrada separadamente.
* **Branch inicial:** `main`
* **Branches de desenvolvimento:** `feature/tableau`, `feature/pivotamento`, `feature/testes`, `feature/big-m`

### 📝 Padrão de Commits
Os commits seguirão uma convenção baseada em tipos:
* `chore: inicializa projeto simplex em C`
* `docs: adiciona documentação inicial do projeto`
* `feat: cria representação de problema linear`
* `test: adiciona testes do tableau`
* `refactor: separa lógica do simplex do programa principal`

### 🐙 GitHub
O repositório deverá conter: README, código-fonte, documentação, testes, exemplos, `.gitignore`, histórico de commits organizado, Issues, branches, Pull Requests, Releases e tags.

### 📌 Issues Planejadas
* **#1** — Configurar projeto
* **#2** — Criar representação do problema
* **#3** — Implementar tableau
* **#4** — Implementar seleção do pivô
* **#5** — Implementar pivoteamento
* **#6** — Implementar critério de parada
* **#7** — Criar problemas de teste
* **#8** — Tratar solução ilimitada
* **#9** — Tratar problema inviável
* **#10** — Implementar Big M
* **#11** — Implementar Duas Fases

## 🚀 Roadmap

```text
                    SIMPLEX EM C
                         │
                         ▼
              ┌─────────────────────┐
              │ Fundamentos         │
              │ Matemáticos         │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Estrutura do        │
              │ Projeto             │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Tableau             │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Pivoteamento        │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Simplex completo    │
              │ para caso básico    │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Testes              │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Casos especiais     │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Big M               │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ Duas Fases          │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │ v1.0.0              │
              └─────────────────────┘
```

## 📚 Documentação

A documentação será dividida em arquivos específicos:
* `mathematics.md`: Conceitos matemáticos utilizados.
* `simplex-algorithm.md`: Descrição passo a passo do algoritmo.
* `tableau.md`: Explicação da estrutura do tableau.
* `pivoting.md`: Explicação matemática e computacional do pivoteamento.
* `architecture.md`: Descrição da arquitetura do software.
* `decisions.md`: Registro das decisões técnicas e matemáticas do projeto.

## 🔮 Possíveis Evoluções

* **Entrada pelo terminal:** Permitir que o usuário informe dinamicamente.
* **Entrada por arquivo:** Exemplo: `problema.txt`.
* **Visualização das iterações:** Exibir cada passo e tableau correspondente.
* **Minimização:** Adicionar suporte a $Min Z = ...$
* **Restrições gerais:** Adicionar $\leq$, $\geq$, $=$
* **Big M & Duas Fases:** Suporte para variáveis artificiais.
* **Interface:** Criar uma interface para visualização do processo.

## ▶️ Como Executar

**Pré-requisitos:** É necessário possuir um compilador C (Ex: `gcc --version`).

**Compilação:**
```bash
gcc src/main.c -o simplex
# Após a modularização:
gcc src/*.c -Iinclude -o simplex
```

**Execução no Windows:**
```bash
simplex.exe
# ou:
.\simplex.exe
```

**Execução no Linux:**
```bash
./simplex
```

## 💻 Exemplo de Execução

Exemplo conceitual:

```text
========================================
       MÉTODO SIMPLEX EM C
========================================

Problema:
Max Z = 3x1 + 5x2

Sujeito a:
x1 + 2x2 <= 8
3x1 + 2x2 <= 12
x1 >= 0
x2 >= 0

----------------------------------------
TABLEAU INICIAL
----------------------------------------
        x1      x2      s1      s2      RHS
s1      1       2       1       0        8
s2      3       2       0       1       12
Z      -3      -5       0       0        0

----------------------------------------
ITERACAO 1
----------------------------------------
Coluna pivô: x2
Linha pivô: s1
...

----------------------------------------
SOLUÇÃO
----------------------------------------
x1 = 2
x2 = 3
Z = 21
```

## 📈 Versionamento

O projeto utilizará Versionamento Semântico no formato `MAJOR.MINOR.PATCH` (Ex: `v0.1.0`).

### 📋 Histórico de Versões

| Versão | Descrição | Status |
|--------|-----------|--------|
| v0.1.0 | Simplex básico | 🔄 Em desenvolvimento |
| v0.2.0 | Tableau completo | ⏳ Planejado |
| v0.3.0 | Entrada dinâmica | ⏳ Planejado |
| v0.4.0 | Restrições gerais | ⏳ Planejado |
| v0.5.0 | Casos especiais | ⏳ Planejado |
| v0.6.0 | Big M | ⏳ Planejado |
| v0.7.0 | Duas Fases | ⏳ Planejado |
| v0.8.0 | Entrada por arquivos | ⏳ Planejado |
| v0.9.0 | Testes e refinamentos | ⏳ Planejado |
| v1.0.0 | Solver inicial completo | ⏳ Planejado |

## 🎓 Aprendizados

Este projeto busca desenvolver conhecimentos em três níveis:
* **🧮 Nível Matemático:** Programação Linear, matrizes, sistemas lineares, álgebra linear, otimização, Método Simplex.
* **💻 Nível de Programação:** linguagem C, ponteiros, structs, matrizes, memória dinâmica, modularização, algoritmos, tratamento de números reais.
* **🏗️ Nível de Engenharia de Software:** arquitetura, organização de código, testes, Git, GitHub, documentação, versionamento, Issues, Pull Requests, Releases. 

## 🔐 Boas Práticas

O projeto seguirá algumas práticas: validar entradas, verificar falhas de alocação, evitar acessos inválidos à memória, liberar memória alocada, evitar código duplicado, separar responsabilidades, documentar decisões importantes, testar casos extremos.

🚫 **Arquivos que não devem ser versionados:** O `.gitignore` deverá impedir o envio de arquivos como `*.exe`, `*.o`, `*.obj`, `build/`, `bin/`, `.vscode/`.

## 📌 Checklist do Projeto

**Fundamentos**
- [ ] Entender Programação Linear
- [ ] Entender função objetivo
- [ ] Entender restrições
- [ ] Entender variáveis de folga
- [ ] Entender tableau
- [ ] Entender pivoteamento

**Implementação**
- [ ] Criar modelo
- [ ] Criar matriz
- [ ] Criar tableau
- [ ] Implementar coluna pivô
- [ ] Implementar teste da razão
- [ ] Implementar linha pivô
- [ ] Implementar pivoteamento
- [ ] Implementar critério de parada
- [ ] Extrair solução

**Testes**
- [ ] Testar tableau
- [ ] Testar pivoteamento
- [ ] Testar solução conhecida
- [ ] Testar múltiplas iterações
- [ ] Testar números decimais
- [ ] Testar solução ilimitada
- [ ] Testar problema inviável
- [ ] Testar degeneração

**Engenharia**
- [ ] README
- [ ] .gitignore
- [ ] Organização de diretórios
- [ ] Git & Commits
- [ ] Issues & Branches
- [ ] Pull Requests & Tags
- [ ] Releases & CHANGELOG

**Documentação**
- [ ] Matemática
- [ ] Algoritmo
- [ ] Tableau
- [ ] Pivoteamento
- [ ] Arquitetura
- [ ] Decisões técnicas

## 📌 Status
🚧 **Em desenvolvimento**
O projeto encontra-se em fase de implementação e estudo. O escopo inicial prioriza o entendimento e a implementação correta do Simplex para problemas de maximização com restrições $\leq$ e variáveis não negativas.

## 📄 Licença
Este projeto poderá ser distribuído sob a licença **MIT License**. A licença definitiva deverá ser adicionada ao arquivo `LICENSE`.

## 👨‍💻 Autor
**Dagoberto Silva**
🎓 Graduando em Ciência da Computação — UFC
💻 Interesse em: Algoritmos, Programação em C/C++, Sistemas Embarcados, IoT, Robótica, Redes de Computadores, Inteligência Artificial, Pesquisa Operacional.

---
⭐ **Objetivo do Projeto**
Mais do que implementar um algoritmo, este projeto busca demonstrar a capacidade de transformar um problema matemático em uma solução computacional estruturada, passando por todas as etapas:
`Matemática → Modelagem → Algoritmo → Implementação → Testes → Documentação → Versionamento → Software`
O objetivo final é possuir não apenas um programa que execute o Método Simplex, mas uma implementação compreensível, testável, documentada e evolutiva, capaz de servir como projeto acadêmico e peça de portfólio.