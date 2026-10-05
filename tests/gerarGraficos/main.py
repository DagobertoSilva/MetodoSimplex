import sys
from pathlib import Path
import re
import matplotlib.pyplot as plt


# ==========================================
# CONFIGURAÇÃO DO TERMINAL
# ==========================================

sys.stdout.reconfigure(encoding="utf-8")


# ==========================================
# 1. LOCALIZAR O ARQUIVO
# ==========================================

caminho = Path(__file__).parent / "restricoes.txt"


# ==========================================
# 2. VERIFICAR SE O ARQUIVO EXISTE
# ==========================================

if not caminho.exists():

    print("Erro: arquivo restricoes.txt não encontrado.")
    print("Caminho procurado:")
    print(caminho)

    sys.exit()


# ==========================================
# 3. LER O ARQUIVO
# ==========================================

with open(caminho, "r", encoding="utf-8") as arquivo:
    linhas = arquivo.readlines()


# ==========================================
# 4. EXPRESSÕES REGULARES
# ==========================================

# ------------------------------------------------
# Restrição com x1 e x2
#
# Exemplos:
# x1 + 3x2 <= 20
# 2x1 - x2 >= 10
# 3x1 + 2x2 = 15
# 1x1 + 0x2 <= 4
# 0x1 + 1x2 <= 6
# ------------------------------------------------

padrao_duas_variaveis = (
    r"([+-]?(?:\d+(?:\.\d+)?)?)x1"
    r"([+-](?:\d+(?:\.\d+)?)?)x2"
    r"(<=|>=|=|≤|≥)"
    r"([+-]?\d+(?:\.\d+)?)"
)


# ------------------------------------------------
# Não negatividade de x1
#
# Exemplos:
# x1 >= 0
# x1 >= 0.0
# x1 ≥ 0
# ------------------------------------------------

padrao_x1 = (
    r"x1"
    r"(<=|>=|=|≤|≥)"
    r"([+-]?\d+(?:\.\d+)?)"
)


# ------------------------------------------------
# Não negatividade de x2
#
# Exemplos:
# x2 >= 0
# x2 >= 0.0
# x2 ≥ 0
# ------------------------------------------------

padrao_x2 = (
    r"x2"
    r"(<=|>=|=|≤|≥)"
    r"([+-]?\d+(?:\.\d+)?)"
)


# ==========================================
# 5. LISTAS PARA GUARDAR AS RESTRIÇÕES
# ==========================================

restricoes = []

nao_negatividade_x1 = False
nao_negatividade_x2 = False


# ==========================================
# 6. FUNÇÃO PARA CONVERTER COEFICIENTES
# ==========================================

def converter_coeficiente(valor):

    if valor == "" or valor == "+":
        return 1.0

    if valor == "-":
        return -1.0

    return float(valor)


# ==========================================
# 7. LER CADA LINHA
# ==========================================

for linha in linhas:

    linha = linha.strip()

    # Ignorar linhas vazias
    if not linha:
        continue

    print("\n" + "=" * 50)
    print("Restrição lida:")
    print(linha)

    # ------------------------------------------
    # Remover espaços
    # ------------------------------------------

    linha_sem_espacos = linha.replace(" ", "")

    # ------------------------------------------
    # Converter operadores Unicode
    # ------------------------------------------

    linha_sem_espacos = (
        linha_sem_espacos
        .replace("≤", "<=")
        .replace("≥", ">=")
    )


    # ==========================================
    # 8. VERIFICAR x1 >= 0
    # ==========================================

    resultado_x1 = re.fullmatch(
        padrao_x1,
        linha_sem_espacos
    )

    if resultado_x1:

        operador = resultado_x1.group(1)
        valor = float(resultado_x1.group(2))

        print("\nTipo: Não negatividade de x1")
        print("Variável: x1")
        print("Operador:", operador)
        print("Valor:", valor)

        if operador == ">=" and valor == 0:

            nao_negatividade_x1 = True

            print("✓ Restrição de não negatividade detectada:")
            print("  x1 >= 0")

        else:

            print(
                "Aviso: esta é uma restrição somente de x1, "
                "mas não representa x1 >= 0."
            )

        continue


    # ==========================================
    # 9. VERIFICAR x2 >= 0
    # ==========================================

    resultado_x2 = re.fullmatch(
        padrao_x2,
        linha_sem_espacos
    )

    if resultado_x2:

        operador = resultado_x2.group(1)
        valor = float(resultado_x2.group(2))

        print("\nTipo: Não negatividade de x2")
        print("Variável: x2")
        print("Operador:", operador)
        print("Valor:", valor)

        if operador == ">=" and valor == 0:

            nao_negatividade_x2 = True

            print("✓ Restrição de não negatividade detectada:")
            print("  x2 >= 0")

        else:

            print(
                "Aviso: esta é uma restrição somente de x2, "
                "mas não representa x2 >= 0."
            )

        continue


    # ==========================================
    # 10. VERIFICAR RESTRIÇÃO COM x1 E x2
    # ==========================================

    resultado = re.fullmatch(
        padrao_duas_variaveis,
        linha_sem_espacos
    )

    if not resultado:

        print("\nErro: formato inválido:")
        print(linha)

        continue


    # ==========================================
    # 11. PEGAR OS VALORES
    # ==========================================

    coef_x1 = resultado.group(1)
    coef_x2 = resultado.group(2)
    operador = resultado.group(3)
    valor = resultado.group(4)


    # ==========================================
    # 12. CONVERTER COEFICIENTES
    # ==========================================

    a = converter_coeficiente(coef_x1)
    b = converter_coeficiente(coef_x2)
    c = float(valor)


    # ==========================================
    # 13. MOSTRAR RESULTADO
    # ==========================================

    print("\nTipo: Restrição com duas variáveis")

    print("Coeficiente de x1:", a)
    print("Coeficiente de x2:", b)
    print("Operador:", operador)
    print("Valor:", c)


    # ==========================================
    # 14. GUARDAR RESTRIÇÃO
    # ==========================================

    restricoes.append({
        "a": a,
        "b": b,
        "c": c,
        "operador": operador,
        "texto": linha
    })


# ==========================================
# 15. ANALISAR NÃO NEGATIVIDADE
# ==========================================

print("\n")
print("=" * 60)
print("             ANÁLISE DE NÃO NEGATIVIDADE")
print("=" * 60)


if nao_negatividade_x1:

    print("✓ x1 >= 0 está presente.")

else:

    print("⚠ x1 >= 0 NÃO está presente.")


if nao_negatividade_x2:

    print("✓ x2 >= 0 está presente.")

else:

    print("⚠ x2 >= 0 NÃO está presente.")


# ==========================================
# 16. VERIFICAR RESTRIÇÕES
# ==========================================

if not restricoes and not nao_negatividade_x1 and not nao_negatividade_x2:

    print("\nNenhuma restrição válida encontrada.")

    sys.exit()


# ==========================================
# 17. CRIAR O GRÁFICO
# ==========================================

plt.figure(figsize=(10, 7))


# ==========================================
# 18. DESENHAR RESTRIÇÕES
# ==========================================

for restricao in restricoes:

    a = restricao["a"]
    b = restricao["b"]
    c = restricao["c"]
    texto = restricao["texto"]


    # ======================================
    # CASO 1:
    # b == 0
    #
    # Exemplo:
    # 1x1 + 0x2 <= 4
    #
    # Isso significa:
    #
    # x1 = 4
    #
    # Portanto é uma reta VERTICAL.
    # ======================================

    if b == 0:

        if a == 0:

            print(
                f"\nNão é possível desenhar {texto}: "
                "coeficientes de x1 e x2 são zero."
            )

            continue


        # Calcula x1 = c/a
        x1_valor = c / a

        # Valores de x2 para desenhar a reta
        x2 = list(range(0, 31))

        # x1 permanece constante
        x1 = [x1_valor] * len(x2)


        print(
            f"\n✓ Desenhando restrição vertical: "
            f"x1 = {x1_valor}"
        )


        plt.plot(
            x1,
            x2,
            linewidth=2,
            label=texto
        )


    # ======================================
    # CASO 2:
    # b != 0
    #
    # Exemplo:
    #
    # 3x1 + 2x2 <= 18
    #
    # x2 = (18 - 3x1) / 2
    # ======================================

    else:

        # Gerar valores de x1
        x1 = list(range(0, 31))

        # Lista para armazenar x2
        x2 = []


        # Calcular x2
        for valor_x1 in x1:

            valor_x2 = (
                c - a * valor_x1
            ) / b

            x2.append(valor_x2)


        print(
            f"\n✓ Desenhando restrição:"
            f" {texto}"
        )


        # Desenhar reta
        plt.plot(
            x1,
            x2,
            linewidth=2,
            label=texto
        )


# ==========================================
# 19. DESENHAR NÃO NEGATIVIDADE DE x1
# ==========================================

if nao_negatividade_x1:

    # x1 = 0
    #
    # x1 corresponde ao eixo horizontal
    # x2 corresponde ao eixo vertical
    #
    # Portanto x1 = 0 é o eixo vertical.

    x2_eixo = list(range(0, 31))

    x1_eixo = [0] * len(x2_eixo)


    plt.plot(
        x1_eixo,
        x2_eixo,
        linewidth=3,
        linestyle="--",
        label="x1 >= 0"
    )


# ==========================================
# 20. DESENHAR NÃO NEGATIVIDADE DE x2
# ==========================================

if nao_negatividade_x2:

    # x2 = 0
    #
    # Portanto é o eixo horizontal.

    x1_eixo = list(range(0, 31))

    x2_eixo = [0] * len(x1_eixo)


    plt.plot(
        x1_eixo,
        x2_eixo,
        linewidth=3,
        linestyle="--",
        label="x2 >= 0"
    )


# ==========================================
# 21. DESTACAR A REGIÃO DE NÃO NEGATIVIDADE
# ==========================================

if nao_negatividade_x1 and nao_negatividade_x2:

    # Quando temos:
    #
    # x1 >= 0
    # x2 >= 0
    #
    # estamos trabalhando apenas no
    # primeiro quadrante.

    plt.fill_between(
        [0, 30],
        0,
        30,
        alpha=0.08
    )


# ==========================================
# 22. DESENHAR EIXOS
# ==========================================

plt.axhline(
    y=0,
    linewidth=1
)

plt.axvline(
    x=0,
    linewidth=1
)


# ==========================================
# 23. CONFIGURAÇÕES DO GRÁFICO
# ==========================================

plt.xlim(0, 30)

plt.ylim(0, 30)

plt.xlabel("x1")

plt.ylabel("x2")

plt.title(
    "Gráfico das Restrições e Condições de Não Negatividade"
)

plt.grid(True)

plt.legend()


# ==========================================
# 24. MOSTRAR GRÁFICO
# ==========================================

plt.show()