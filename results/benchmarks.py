"""
Descripcion: benchmarks reproducibles de los siete modos y sus tecnicas.
Genera instancias chicas, medianas y grandes con una semilla fija, corre
./sentinel con cada tecnica (campo opcional "technique") sobre la MISMA
instancia y registra tiempo (mediana de varias corridas), memoria maxima
del proceso, si la respuesta coincide entre tecnicas y el exponente
empirico de crecimiento para compararlo con la complejidad teorica.
Escribe results/benchmarks.csv y results/benchmarks.md.
Uso: python3 results/benchmarks.py [--rapido]  (compila src/main.cpp antes)
Autores: Carlos Emiliano Nava Flores (A00842650),
         Luis Enrique Castillo Carrillo (A00842390),
         Desiderio Ivan Ortegon Morton (A00840591),
         Fernando Robles Vidrio (A00843290)
Fecha de creacion: 20/10/2026
"""

import csv
import datetime
import json
import math
import os
import platform
import random
import statistics
import subprocess
import sys
import time

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EJECUTABLE = os.path.join(RAIZ, "sentinel")
CARPETA = os.path.join(RAIZ, "results")
SEMILLA = 2038
BANDERAS = "-std=c++17 -O2 -Wall -Wextra -Wpedantic"
HEXADECIMAL = "0123456789ABCDEF"
# Si una tecnica tarda mas que esto en un tamano, ya no se prueba con los mayores.
PRESUPUESTO_SEGUNDOS = 20
RAPIDO = "--rapido" in sys.argv
REPETICIONES = 1 if RAPIDO else 3


def medir(modo, entrada):
	"""Mediana del tiempo de REPETICIONES corridas y memoria maxima residente."""
	tiempos = []
	memoria = 0.0
	respuesta = None
	for _ in range(REPETICIONES):
		peticion = json.dumps({"mode": modo, "input": entrada}).encode()
		inicio = time.perf_counter()
		proceso = subprocess.Popen([EJECUTABLE], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		proceso.stdin.write(peticion)
		proceso.stdin.close()
		salida = proceso.stdout.read()
		# wait4 da el uso de recursos de ESTE proceso (getrusage acumularia todos).
		_, _, uso = os.wait4(proceso.pid, 0)
		tiempos.append(time.perf_counter() - inicio)
		proceso.stdout.close()
		# Ya se espero con wait4; se marca terminado para que Popen no lo espere otra vez.
		proceso.returncode = 0
		# ru_maxrss viene en bytes en macOS y en KB en Linux.
		escala = 1 if sys.platform == "darwin" else 1024
		memoria = max(memoria, uso.ru_maxrss * escala / (1024 * 1024))
		respuesta = json.loads(salida)
	return respuesta, statistics.median(tiempos), memoria


# ----------------------------------------------------------- instancias

def instanciaMst(generador, cantidad):
	"""Grafo conexo: camino base mas 4n aristas aleatorias (E = 5n)."""
	aristas = [{"u": i, "v": i + 1, "weight": generador.randint(1, 1000)} for i in range(cantidad - 1)]
	for _ in range(4 * cantidad):
		aristas.append({"u": generador.randrange(cantidad), "v": generador.randrange(cantidad), "weight": generador.randint(1, 1000)})
	return {"nodes": list(range(cantidad)), "edges": aristas}, len(aristas)


def instanciaTsp(generador, cantidad):
	"""Puntos aleatorios en el plano; distancia euclidiana redondeada."""
	puntos = [(generador.uniform(0, 1000), generador.uniform(0, 1000)) for _ in range(cantidad)]
	return {"distances": [[round(math.dist(p, q)) for q in puntos] for p in puntos]}, cantidad


def instanciaFlujo(generador, cantidad):
	"""Red dirigida: camino base mas 9n aristas, capacidades 1..10000 (E = 10n)."""
	aristas = [{"u": i, "v": i + 1, "capacity": generador.randint(100, 10000)} for i in range(cantidad - 1)]
	for _ in range(9 * cantidad):
		u, v = generador.randrange(cantidad), generador.randrange(cantidad)
		if u != v:
			aristas.append({"u": u, "v": v, "capacity": generador.randint(1, 10000)})
	return {"n": cantidad, "edges": aristas, "source": 0, "sink": cantidad - 1}, len(aristas)


def instanciaVoronoi(generador, cantidad):
	"""Sitios aleatorios distintos en [0, 100000]^2."""
	sitios = {(generador.randint(1, 99999), generador.randint(1, 99999)) for _ in range(cantidad * 2)}
	return {"sites": [list(s) for s in list(sitios)[:cantidad]], "bounds": [0, 0, 100000, 100000]}, cantidad


def instanciaFirmasAleatoria(generador, cantidad):
	"""Dos transmisiones hexadecimales aleatorias y tres firmas de 16 caracteres."""
	transmisiones = ["".join(generador.choice(HEXADECIMAL) for _ in range(cantidad)) for _ in range(2)]
	firmas = [transmisiones[0][cantidad // 2:cantidad // 2 + 16], "DEADBEEFCAFEBABE", "0123456789ABCDEF"]
	return {"transmissions": transmisiones, "patterns": firmas}, cantidad


def instanciaFirmasPeorCaso(generador, cantidad):
	"""Peor caso de la busqueda ingenua: "AAAA..." y firmas "AAA...AB" de 1000."""
	del generador
	return {"transmissions": ["A" * cantidad, "A" * cantidad], "patterns": ["A" * 999 + "B", "A" * 499 + "B", "B"]}, cantidad


def instanciaPalindromoAleatoria(generador, cantidad):
	"""Dos transmisiones hexadecimales aleatorias."""
	return {"transmissions": ["".join(generador.choice(HEXADECIMAL) for _ in range(cantidad)) for _ in range(2)]}, cantidad


def instanciaPalindromoPeorCaso(generador, cantidad):
	"""Peor caso de la expansion desde el centro: todos los caracteres iguales."""
	del generador
	return {"transmissions": ["A" * cantidad, "AB" * (cantidad // 2)]}, cantidad


def instanciaComun(generador, cantidad):
	"""Dos transmisiones aleatorias que comparten un bloque de n / 8."""
	bloque = "".join(generador.choice(HEXADECIMAL) for _ in range(max(1, cantidad // 8)))
	mitad = (cantidad - len(bloque)) // 2
	partes = ["".join(generador.choice(HEXADECIMAL) for _ in range(mitad)) for _ in range(4)]
	return {"transmissions": [partes[0] + bloque + partes[1], partes[2] + bloque + partes[3]]}, cantidad


# Cada experimento: (modo, escenario, generador, tamanos, tecnicas, funcion que
# resume la respuesta, tamano que se usa como "n" en la tabla, complejidad teorica).
EXPERIMENTOS = [
	("mst", "aleatorio E = 5V", instanciaMst, [1000, 10000, 50000], ["kruskal", "prim"],
	 lambda r: r["cost"], "V", {"kruskal": "O(E log E)", "prim": "O(E log V)"}),
	("tsp", "puntos en el plano", instanciaTsp, [8, 12, 16, 18], ["held-karp", "nearest-neighbor", "2-opt"],
	 lambda r: r["cost"], "n", {"held-karp": "O(2^n n^2)", "nearest-neighbor": "O(n^2)", "2-opt": "O(n^2) por pasada"}),
	("tsp", "puntos en el plano (grande)", instanciaTsp, [100, 500, 1000, 2000], ["nearest-neighbor", "2-opt"],
	 lambda r: r["cost"], "n", {"nearest-neighbor": "O(n^2)", "2-opt": "O(n^2) por pasada"}),
	("flow", "aleatorio E = 10V", instanciaFlujo, [100, 300, 1000], ["edmonds-karp", "ford-fulkerson"],
	 lambda r: r["flow"], "V", {"edmonds-karp": "O(V E^2)", "ford-fulkerson": "O(E f)"}),
	("voronoi", "sitios aleatorios", instanciaVoronoi, [100, 500, 2000, 5000], ["half-plane", "half-plane-naive"],
	 lambda r: round(sum(abs(sum(p[0] * q[1] - q[0] * p[1] for p, q in zip(c, c[1:] + c[:1]))) / 2 for c in r["cells"])), "n",
	 {"half-plane": "O(n k log n) con poda", "half-plane-naive": "O(n^2 k)"}),
	("matching", "aleatorio", instanciaFirmasAleatoria, [10000, 100000, 1000000], ["kmp", "naive"],
	 lambda r: r["matches"], "n", {"kmp": "O(n + m)", "naive": "O(n m) peor caso"}),
	("matching", "peor caso (AAAA...)", instanciaFirmasPeorCaso, [10000, 100000, 1000000], ["kmp", "naive"],
	 lambda r: r["matches"], "n", {"kmp": "O(n + m)", "naive": "O(n m) peor caso"}),
	("palindrome", "aleatorio", instanciaPalindromoAleatoria, [10000, 100000, 1000000], ["manacher", "center"],
	 lambda r: r["ranges"], "n", {"manacher": "O(n)", "center": "O(n^2) peor caso"}),
	("palindrome", "peor caso (AAAA...)", instanciaPalindromoPeorCaso, [1000, 10000, 30000, 100000], ["manacher", "center"],
	 lambda r: r["ranges"], "n", {"manacher": "O(n)", "center": "O(n^2) peor caso"}),
	("common", "bloque compartido n/8", instanciaComun, [1000, 5000, 20000, 200000], ["suffix-array", "dp"],
	 lambda r: r["range"][1] - r["range"][0] + 1, "n", {"suffix-array": "O(n log n)", "dp": "O(n m)"}),
]


def describirEntorno():
	"""Datos para reproducir: hardware, sistema, compilador y banderas."""
	procesador = platform.processor()
	if sys.platform == "darwin":
		procesador = subprocess.run(["sysctl", "-n", "machdep.cpu.brand_string"], capture_output=True, text=True).stdout.strip()
	compilador = subprocess.run(["g++", "--version"], capture_output=True, text=True).stdout.splitlines()[0]
	return {
		"Fecha": datetime.date.today().isoformat(),
		"Procesador": procesador,
		"Sistema": f"{platform.system()} {platform.release()} ({platform.machine()})",
		"Compilador": compilador,
		"Banderas": BANDERAS,
		"Python": platform.python_version(),
		"Semilla": str(SEMILLA),
		"Repeticiones": f"{REPETICIONES} (se reporta la mediana)",
	}


def main():
	"""Corre todos los experimentos y escribe CSV y Markdown."""
	if not os.path.exists(EJECUTABLE):
		print(f"Falta ./sentinel: g++ {BANDERAS} src/main.cpp -o sentinel")
		sys.exit(1)
	filas = []
	for modo, escenario, crear, tamanos, tecnicas, resumir, nombreTamano, teoria in EXPERIMENTOS:
		print(f"{modo} · {escenario}")
		agotadas = set()
		for tamano in tamanos:
			generador = random.Random(f"{SEMILLA}-{modo}-{escenario}-{tamano}")
			entrada, tamanoExtra = crear(generador, tamano)
			resultadosTamano = {}
			for tecnica in tecnicas:
				fila = {"modo": modo, "escenario": escenario, "tecnica": tecnica, "n": tamano, "m": tamanoExtra, "teoria": teoria[tecnica]}
				if tecnica in agotadas:
					fila.update({"segundos": "", "memoria_mb": "", "resultado": "omitido (supero el presupuesto en un tamano menor)"})
				else:
					respuesta, segundos, memoria = medir(modo, dict(entrada, technique=tecnica))
					valor = resumir(respuesta) if "error" not in respuesta else respuesta["error"]
					resultadosTamano[tecnica] = valor
					fila.update({"segundos": round(segundos, 4), "memoria_mb": round(memoria, 1), "resultado": json.dumps(valor)})
					if segundos > PRESUPUESTO_SEGUNDOS:
						agotadas.add(tecnica)
					print(f"  {nombreTamano}={tamano:<8} {tecnica:<18} {segundos:9.3f} s  {memoria:8.1f} MB  {str(valor)[:40]}")
				filas.append(fila)
			# Coincidencia: en modos exactos todas deben dar lo mismo; en TSP se
			# reporta la brecha de cada heuristica contra Held-Karp cuando existe.
			valores = list(resultadosTamano.values())
			for fila in filas[-len(tecnicas):]:
				if fila["tecnica"] not in resultadosTamano:
					fila["coincide"] = ""
				elif modo == "tsp" and "held-karp" in resultadosTamano:
					brecha = resultadosTamano[fila["tecnica"]] / resultadosTamano["held-karp"] - 1
					fila["coincide"] = f"brecha {brecha * 100:.2f} %"
				elif modo == "tsp":
					fila["coincide"] = "heuristica"
				else:
					fila["coincide"] = "si" if all(v == valores[0] for v in valores) else "NO"
	# Exponente empirico: pendiente de log(t) contra log(n) entre los dos mayores tamanos medidos.
	for fila in filas:
		previas = [f for f in filas if f["modo"] == fila["modo"] and f["escenario"] == fila["escenario"] and f["tecnica"] == fila["tecnica"] and f["segundos"] != "" and f["n"] < fila["n"]]
		fila["exponente"] = ""
		if previas and fila["segundos"] != "" and fila["segundos"] > 0.05:
			anterior = previas[-1]
			if anterior["segundos"] > 0:
				fila["exponente"] = round(math.log(fila["segundos"] / anterior["segundos"]) / math.log(fila["n"] / anterior["n"]), 2)
	columnas = ["modo", "escenario", "tecnica", "n", "m", "segundos", "memoria_mb", "exponente", "teoria", "coincide", "resultado"]
	with open(os.path.join(CARPETA, "benchmarks.csv"), "w", newline="") as archivo:
		escritor = csv.DictWriter(archivo, fieldnames=columnas, lineterminator="\n")
		escritor.writeheader()
		escritor.writerows(filas)
	entorno = describirEntorno()
	with open(os.path.join(CARPETA, "benchmarks.md"), "w") as archivo:
		archivo.write("# Benchmarks\n\nGenerado por `python3 results/benchmarks.py`. Datos completos en `benchmarks.csv`.\n\n## Entorno\n\n")
		for clave, valor in entorno.items():
			archivo.write(f"- **{clave}:** {valor}\n")
		archivo.write("\n## Cómo leer las columnas\n\n")
		archivo.write("- **segundos**: mediana del tiempo total del proceso (incluye leer y escribir el JSON).\n")
		archivo.write("- **memoria**: memoria residente máxima del proceso.\n")
		archivo.write("- **exponente**: pendiente de log(t) contra log(n) respecto al tamaño anterior; se compara con la complejidad teórica (por ejemplo 1 para O(n), 2 para O(n²)). Solo se calcula si el tiempo supera 0.05 s, porque abajo de eso domina el arranque del proceso.\n")
		archivo.write("- **coincide**: si todas las técnicas exactas dan la misma respuesta en la misma instancia; en TSP, la brecha de la heurística contra el óptimo de Held-Karp.\n\n")
		archivo.write("## Limitaciones conocidas\n\n")
		archivo.write("- En Held-Karp el crecimiento es exponencial, así que el exponente log-log no aplica; para contrastarlo con la teoría conviene el cociente t(n) / t(n - 2), que debería acercarse a 4 · (n / (n - 2))² cuando el tiempo ya no lo domina el arranque.\n")
		archivo.write("- La memoria de las entradas grandes la domina el lector `src/json.h`, que guarda cada número como un `ValorJson` completo (TSP con 2000 nodos: unos 4 millones de valores). El algoritmo en sí usa mucho menos (la matriz de 2000 × 2000 son 32 MB). Los tamaños del autograder (hasta 450 nodos en TSP) quedan muy por debajo.\n")
		archivo.write("- Los tiempos incluyen arrancar el proceso y leer el JSON, así que en instancias chicas (menos de 0.01 s) las técnicas no se distinguen.\n\n")
		escenarioActual = None
		for fila in filas:
			clave = (fila["modo"], fila["escenario"])
			if clave != escenarioActual:
				escenarioActual = clave
				archivo.write(f"\n## {fila['modo']} · {fila['escenario']}\n\n| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |\n| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |\n")
			segundos = fila["segundos"] if fila["segundos"] != "" else fila["resultado"]
			archivo.write(f"| {fila['tecnica']} | {fila['n']} | {fila['m']} | {segundos} | {fila['memoria_mb']} | {fila['exponente']} | {fila['teoria']} | {fila['coincide']} |\n")
	print(f"\nEscrito {os.path.join(CARPETA, 'benchmarks.csv')} y benchmarks.md")


if __name__ == "__main__":
	main()
