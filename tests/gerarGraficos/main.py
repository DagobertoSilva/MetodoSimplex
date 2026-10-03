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
# 4. EXPRESSÃO REGULAR
# ==========================================

padrao = r"([+-]?\d*\.?\d*)x1\s*([+-]?\d*\.?\d*)x2\s*(<=|>=|=|≤|≥)\s*([+-]?\d*\.?\d*)"


# Lista para guardar as restrições
restricoes = []


# ==========================================
# 5. LER CADA LINHA
# ==========================================

for linha in linhas:

    linha = linha.strip()

    # Ignorar linhas vazias
    if not linha:
        continue

    print("\nRestrição lida:")
    print(linha)

    # Remover espaços
    linha_sem_espacos = linha.replace(" ", "")

    # Procurar os valores
    resultado = re.fullmatch(
        padrao,
        linha_sem_espacos
    )

    if not resultado:

        print("Erro: formato inválido:")
        print(linha)

        continue


    # ======================================
    # 6. PEGAR OS VALORES
    # ======================================

    coef_x1 = resultado.group(1)
    coef_x2 = resultado.group(2)
    operador = resultado.group(3)
    valor = resultado.group(4)


    # ======================================
    # 7. CONVERTER COEFICIENTES
    # ======================================

    def converter_coeficiente(valor):

        if valor == "" or valor == "+":
            return 1

        if valor == "-":
            return -1

        return float(valor)


    a = converter_coeficiente(coef_x1)
    b = converter_coeficiente(coef_x2)
    c = float(valor)


    # ======================================
    # 8. MOSTRAR RESULTADO
    # ======================================

    print("Coeficiente de x1:", a)
    print("Coeficiente de x2:", b)
    print("Operador:", operador)
    print("Valor:", c)


    # ======================================
    # 9. GUARDAR A RESTRIÇÃO
    # ======================================

    restricoes.append({
        "a": a,
        "b": b,
        "c": c,
        "operador": operador,
        "texto": linha
    })


# ==========================================
# 10. VERIFICAR SE ENCONTROU RESTRIÇÕES
# ==========================================

if not restricoes:

    print("\nNenhuma restrição válida encontrada.")

    sys.exit()


# ==========================================
# 11. CRIAR O GRÁFICO
# ==========================================

plt.figure(figsize=(10, 7))


# ==========================================
# 12. DESENHAR CADA RESTRIÇÃO
# ==========================================

for restricao in restricoes:

    a = restricao["a"]
    b = restricao["b"]
    c = restricao["c"]
    texto = restricao["texto"]


    # --------------------------------------
    # Verificar divisão por zero
    # --------------------------------------

    if b == 0:

        print(
            f"\nNão é possível desenhar {texto}: "
            "coeficiente de x2 é zero."
        )

        continue


    # --------------------------------------
    # Gerar valores de x1
    # --------------------------------------

    x1 = list(range(0, 31))


    # --------------------------------------
    # Calcular x2
    # --------------------------------------

    x2 = []

    for valor_x1 in x1:

        valor_x2 = (c - a * valor_x1) / b

        x2.append(valor_x2)


    # --------------------------------------
    # Desenhar reta
    # --------------------------------------

    plt.plot(
        x1,
        x2,
        linewidth=2,
        label=texto
    )


# ==========================================
# 13. DESENHAR EIXOS
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
# 14. CONFIGURAÇÕES
# ==========================================

plt.xlabel("x1")
plt.ylabel("x2")

plt.title("Gráfico das Restrições")

plt.grid(True)

plt.legend()


# ==========================================
# 15. MOSTRAR
# ==========================================

plt.show()