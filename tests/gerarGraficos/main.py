
import sys
from pathlib import Path
import re
import math
import matplotlib.pyplot as plt

# ==========================================
# 1. CONFIGURAÇÃO DO TERMINAL
# ==========================================

sys.stdout.reconfigure(encoding="utf-8")

# ==========================================
# 2. LOCALIZAR O ARQUIVO
# ==========================================

caminho = Path(__file__).parent / "restricoes.txt"

if not caminho.exists():
    print("Erro: arquivo restricoes.txt não encontrado.")
    print("Caminho procurado:", caminho)
    sys.exit()

# ==========================================
# 3. LER O ARQUIVO
# ==========================================

with open(caminho, "r", encoding="utf-8") as arquivo:
    linhas = arquivo.readlines()

# ==========================================
# 4. EXPRESSÕES REGULARES
# ==========================================

numero = r"(?:\d+(?:\.\d*)?|\.\d+)"

padrao_duas_variaveis = (
    rf"([+-]?(?:{numero})?)x1"
    rf"([+-](?:{numero})?)x2"
    rf"(<=|>=|=)"
    rf"([+-]?{numero})"
)

padrao_x1 = rf"x1(<=|>=|=)([+-]?{numero})"
padrao_x2 = rf"x2(<=|>=|=)([+-]?{numero})"

# ==========================================
# 5. LISTAS DE RESTRIÇÕES
# ==========================================

restricoes = []
nao_negatividade_x1 = False
nao_negatividade_x2 = False

# ==========================================
# 6. CONVERTER COEFICIENTES
# ==========================================

def converter_coeficiente(valor):
    if valor in ("", "+"):
        return 1.0
    if valor == "-":
        return -1.0
    return float(valor)

# ==========================================
# 7. INTERPRETAR AS RESTRIÇÕES
# ==========================================

for linha in linhas:
    linha = linha.strip()

    if not linha:
        continue

    print("\n" + "=" * 60)
    print("Restrição lida:", linha)

    texto = (
        linha.replace(" ", "")
        .replace("≤", "<=")
        .replace("≥", ">=")
    )

    # Não negatividade de x1
    resultado_x1 = re.fullmatch(padrao_x1, texto)

    if resultado_x1:
        operador = resultado_x1.group(1)
        valor = float(resultado_x1.group(2))

        if operador == ">=" and valor == 0:
            nao_negatividade_x1 = True
            print("Não negatividade detectada: x1 >= 0")
        else:
            print("Restrição somente de x1:", texto)

        continue

    # Não negatividade de x2
    resultado_x2 = re.fullmatch(padrao_x2, texto)

    if resultado_x2:
        operador = resultado_x2.group(1)
        valor = float(resultado_x2.group(2))

        if operador == ">=" and valor == 0:
            nao_negatividade_x2 = True
            print("Não negatividade detectada: x2 >= 0")
        else:
            print("Restrição somente de x2:", texto)

        continue

    # Restrição com duas variáveis
    resultado = re.fullmatch(padrao_duas_variaveis, texto)

    if not resultado:
        print("Erro: formato inválido:", linha)
        continue

    a = converter_coeficiente(resultado.group(1))
    b = converter_coeficiente(resultado.group(2))
    operador = resultado.group(3)
    c = float(resultado.group(4))

    print(f"Coeficiente de x1: {a}")
    print(f"Coeficiente de x2: {b}")
    print(f"Operador: {operador}")
    print(f"Resultado: {c}")

    if a == 0 and b == 0:
        print("Aviso: restrição sem coeficientes variáveis.")
        continue

    restricoes.append({
        "a": a,
        "b": b,
        "c": c,
        "operador": operador,
        "texto": linha
    })

# ==========================================
# 8. ANALISAR NÃO NEGATIVIDADE
# ==========================================

print("\n" + "=" * 60)
print("ANÁLISE DE NÃO NEGATIVIDADE")
print("=" * 60)

print(
    "x1 >= 0:",
    "Presente" if nao_negatividade_x1 else "Ausente"
)
print(
    "x2 >= 0:",
    "Presente" if nao_negatividade_x2 else "Ausente"
)

if not restricoes and not nao_negatividade_x1 and not nao_negatividade_x2:
    print("\nNenhuma restrição válida encontrada.")
    sys.exit()

# ==========================================
# 9. CONFIGURAR O GRÁFICO
# ==========================================

LIMITE = 30
TOLERANCIA = 1e-9

fig, ax = plt.subplots(figsize=(12, 8))

# ==========================================
# 10. DESENHAR AS RETAS
# ==========================================

for r in restricoes:
    a = r["a"]
    b = r["b"]
    c = r["c"]

    if b != 0:
        x1 = [i / 10 for i in range(LIMITE * 10 + 1)]
        x2 = [(c - a * x) / b for x in x1]

        ax.plot(
            x1,
            x2,
            linewidth=2,
            label=r["texto"]
        )

    elif a != 0:
        x1_valor = c / a

        ax.plot(
            [x1_valor, x1_valor],
            [0, LIMITE],
            linewidth=2,
            label=r["texto"]
        )

    print(f"Reta desenhada: {r['texto']}")

# ==========================================
# 11. DESENHAR NÃO NEGATIVIDADE
# ==========================================

if nao_negatividade_x1:
    ax.axvline(
        x=0,
        linewidth=2,
        linestyle="--",
        color="gray",
        label="x1 >= 0"
    )

if nao_negatividade_x2:
    ax.axhline(
        y=0,
        linewidth=2,
        linestyle="--",
        color="gray",
        label="x2 >= 0"
    )

# ==========================================
# 12. CALCULAR INTERSEÇÕES ENTRE RETAS
# ==========================================

intersecoes = []

for i in range(len(restricoes)):
    for j in range(i + 1, len(restricoes)):

        r1 = restricoes[i]
        r2 = restricoes[j]

        a1, b1, c1 = r1["a"], r1["b"], r1["c"]
        a2, b2, c2 = r2["a"], r2["b"], r2["c"]

        determinante = a1 * b2 - a2 * b1

        print("\n" + "-" * 60)
        print(f"Analisando R{i + 1} e R{j + 1}")
        print(r1["texto"])
        print(r2["texto"])

        if math.isclose(
            determinante, 0,
            rel_tol=0,
            abs_tol=TOLERANCIA
        ):
            print("Retas paralelas ou coincidentes.")
            continue

        # Regra de Cramer
        x1 = (c1 * b2 - c2 * b1) / determinante
        x2 = (a1 * c2 - a2 * c1) / determinante

        print(f"Interseção calculada: ({x1:.4f}, {x2:.4f})")

        if not (
            -TOLERANCIA <= x1 <= LIMITE + TOLERANCIA
            and -TOLERANCIA <= x2 <= LIMITE + TOLERANCIA
        ):
            print("Ponto fora da área visível.")
            continue

        if abs(x1) < TOLERANCIA:
            x1 = 0.0
        if abs(x2) < TOLERANCIA:
            x2 = 0.0

        duplicado = any(
            math.isclose(x1, p["x1"], abs_tol=TOLERANCIA)
            and math.isclose(x2, p["x2"], abs_tol=TOLERANCIA)
            for p in intersecoes
        )

        if not duplicado:
            intersecoes.append({
                "x1": x1,
                "x2": x2
            })

# ==========================================
# 13. DESTACAR INTERSEÇÕES ENTRE RETAS
# ==========================================

if intersecoes:
    print("\n" + "=" * 60)
    print("INTERSEÇÕES ENTRE RETAS")
    print("=" * 60)

    for indice, ponto in enumerate(intersecoes, start=1):
        x1 = ponto["x1"]
        x2 = ponto["x2"]

        print(f"P{indice}: ({x1:.4f}, {x2:.4f})")

        ax.scatter(
            x1,
            x2,
            s=110,
            color="red",
            edgecolors="black",
            linewidths=1,
            zorder=5,
            label="Interseção entre retas" if indice == 1 else None
        )

        ax.annotate(
            f"P{indice} ({x1:.2f}, {x2:.2f})",
            xy=(x1, x2),
            xytext=(8, 10),
            textcoords="offset points",
            fontsize=9,
            fontweight="bold",
            color="darkred",
            bbox=dict(
                boxstyle="round,pad=0.3",
                facecolor="white",
                edgecolor="red",
                alpha=0.9
            ),
            zorder=6
        )

# ==========================================
# 14. CALCULAR INTERSEÇÕES COM OS EIXOS
# ==========================================

pontos_eixos = []

print("\n" + "=" * 60)
print("INTERSEÇÕES COM OS EIXOS COORDENADOS")
print("=" * 60)

for i, r in enumerate(restricoes, start=1):
    a = r["a"]
    b = r["b"]
    c = r["c"]

    candidatos = []

    # Eixo x1: x2 = 0
    if a != 0:
        candidatos.append((c / a, 0.0, "x1"))

    # Eixo x2: x1 = 0
    if b != 0:
        candidatos.append((0.0, c / b, "x2"))

    for x1, x2, eixo in candidatos:

        # Somente primeiro quadrante e área visível
        if not (
            -TOLERANCIA <= x1 <= LIMITE + TOLERANCIA
            and -TOLERANCIA <= x2 <= LIMITE + TOLERANCIA
        ):
            print(
                f"R{i}: interseção com {eixo} "
                f"({x1:.4f}, {x2:.4f}) fora da área visível."
            )
            continue

        if abs(x1) < TOLERANCIA:
            x1 = 0.0
        if abs(x2) < TOLERANCIA:
            x2 = 0.0

        duplicado = any(
            math.isclose(x1, p["x1"], abs_tol=TOLERANCIA)
            and math.isclose(x2, p["x2"], abs_tol=TOLERANCIA)
            for p in pontos_eixos
        )

        if duplicado:
            continue

        pontos_eixos.append({
            "x1": x1,
            "x2": x2,
            "eixo": eixo
        })

        print(
            f"R{i} com eixo {eixo}: "
            f"({x1:.4f}, {x2:.4f})"
        )

        ax.scatter(
            x1,
            x2,
            s=85,
            color="blue",
            edgecolors="black",
            linewidths=1,
            zorder=7,
            label="Interseção com os eixos"
            if len(pontos_eixos) == 1 else None
        )

        ax.annotate(
            f"({x1:.2f}, {x2:.2f})",
            xy=(x1, x2),
            xytext=(8, -18),
            textcoords="offset points",
            fontsize=9,
            fontweight="bold",
            color="blue",
            bbox=dict(
                boxstyle="round,pad=0.25",
                facecolor="white",
                edgecolor="blue",
                alpha=0.9
            ),
            zorder=8
        )

# ==========================================
# 15. DESTACAR A ORIGEM (0, 0)
# ==========================================

print("\n" + "=" * 60)
print("ORIGEM DO PLANO CARTESIANO")
print("=" * 60)
print("Origem identificada: O = (0.00, 0.00)")

ax.scatter(
    0,
    0,
    s=150,
    color="green",
    edgecolors="black",
    linewidths=1.5,
    marker="o",
    zorder=10,
    label="Origem (0, 0)"
)

ax.annotate(
    "O (0, 0)",
    xy=(0, 0),
    xytext=(12, 12),
    textcoords="offset points",
    fontsize=10,
    fontweight="bold",
    color="darkgreen",
    bbox=dict(
        boxstyle="round,pad=0.3",
        facecolor="white",
        edgecolor="green",
        alpha=0.95
    ),
    zorder=11
)

# ==========================================
# 16. DESTACAR A REGIÃO DO PRIMEIRO QUADRANTE
# ==========================================

if nao_negatividade_x1 and nao_negatividade_x2:
    ax.fill_between(
        [0, LIMITE],
        0,
        LIMITE,
        alpha=0.06,
        color="gray"
    )

# ==========================================
# 17. CONFIGURAR E EXIBIR O GRÁFICO
# ==========================================

ax.axhline(y=0, linewidth=1, color="black")
ax.axvline(x=0, linewidth=1, color="black")

ax.set_xlim(0, LIMITE)
ax.set_ylim(0, LIMITE)

ax.set_xlabel("x1", fontsize=12)
ax.set_ylabel("x2", fontsize=12)

ax.set_title(
    "Restrições e Interseções entre Retas e Eixos",
    fontsize=14
)

ax.grid(True, linestyle=":", alpha=0.7)
ax.legend(loc="best", fontsize=8)
fig.tight_layout()

plt.show()
