"""
Descripcion: pruebas automatizadas de los modos "tsp" y "voronoi".
Ejecuta ./sentinel con cada peticion y valida la respuesta contra oraculos
independientes del codigo C++:
  - TSP: fuerza bruta (n <= 9), Held-Karp en Python (n <= 16), perimetro
    del poligono para puntos en un circulo, y validez del ciclo siempre.
  - Voronoi: areas esperadas calculadas a mano, el mismo recorte que usa el
    autograder de la pagina (tolerancia de 0.3 %), muestreo por sitio mas
    cercano e invariantes (suma de areas, vertices dentro del dominio y mas
    cerca de su sitio que de cualquier otro).
Incluye casos propios fijos, casos aleatorios con el formato de la pagina y
pruebas de estres con los tamanos de los perfiles lite, normal y huge.
Uso: python3 tests/probarTspVoronoi.py  (compila src/main.cpp antes)
Autores: Carlos Emiliano Nava Flores, Luis Enrique Castillo Carrillo,
         Desiderio Ivan Ortegon Morton, Fernando Robles Vidrio
Fecha de creacion: 20/10/2026
"""

import itertools
import json
import math
import os
import random
import subprocess
import sys
import time

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EJECUTABLE = os.path.join(RAIZ, "sentinel")
PETICIONES = os.path.join(RAIZ, "tests", "peticiones")
TOLERANCIA_COSTO = 1e-5
TOLERANCIA_AREA = 0.003
# Tiempos limite del autograder (segundos) por perfil de estres.
PERFILES_ESTRES = {"lite": (60, 80, 6), "normal": (120, 180, 8), "huge": (450, 500, 18)}

# Costos optimos conocidos de los casos propios (calculados a mano).
COSTOS_ESPERADOS = {
	"tsp.json": 80, "tsp_vacio.json": 0, "tsp_un_nodo.json": 0, "tsp_dos_nodos.json": 14,
	"tsp_triangulo.json": 12, "tsp_asimetrico.json": 5, "tsp_no_metrico.json": 4,
	"tsp_iguales.json": 30, "tsp_ceros.json": 0, "tsp_decimales.json": 6.625,
	"tsp_grandes.json": 4800000,
}
# Areas esperadas de cada celda de los casos propios (calculadas a mano).
AREAS_ESPERADAS = {
	"voronoi.json": [5000, 5000], "voronoi_un_sitio.json": [10000],
	"voronoi_cuadrantes.json": [2500, 2500, 2500, 2500], "voronoi_colineales.json": [2000, 3500, 4500],
	"voronoi_diagonal.json": [5000, 5000], "voronoi_borde.json": [5000, 5000],
	"voronoi_duplicados.json": [5000, 5000, 5000], "voronoi_rectangulo.json": [2000, 2000],
	"voronoi_cuadricula_9.json": [100] * 9,
}

resultados = {"pasaron": 0, "fallaron": 0}


def ejecutar(modo, entrada):
	"""Corre el ejecutable con una peticion; regresa (respuesta, segundos)."""
	peticion = json.dumps({"mode": modo, "input": entrada})
	inicio = time.perf_counter()
	proceso = subprocess.run([EJECUTABLE], input=peticion, capture_output=True, text=True, timeout=120)
	segundos = time.perf_counter() - inicio
	return json.loads(proceso.stdout), segundos


def reportar(nombre, correcto, detalle, segundos):
	"""Imprime una linea de resultado y lleva la cuenta."""
	resultados["pasaron" if correcto else "fallaron"] += 1
	marca = "OK   " if correcto else "FALLA"
	print(f"  {marca} {nombre:<34} {segundos * 1000:9.1f} ms  {detalle}")


def costosIguales(a, b):
	"""Misma comparacion relativa que usa el autograder."""
	return abs(a - b) <= TOLERANCIA_COSTO * max(1, abs(b))


# ---------------------------------------------------------------- TSP

def tspFuerzaBruta(distancias):
	"""Costo optimo probando todas las permutaciones (n <= 9)."""
	cantidad = len(distancias)
	if cantidad <= 1:
		return 0
	mejor = math.inf
	for orden in itertools.permutations(range(1, cantidad)):
		ruta = (0,) + orden + (0,)
		mejor = min(mejor, sum(distancias[ruta[i]][ruta[i + 1]] for i in range(cantidad)))
	return mejor


def tspHeldKarp(distancias):
	"""Costo optimo con Held-Karp escrito aparte en Python (n <= 16)."""
	cantidad = len(distancias)
	if cantidad <= 1:
		return 0
	resto = cantidad - 1
	costo = [[math.inf] * resto for _ in range(1 << resto)]
	for nodo in range(resto):
		costo[1 << nodo][nodo] = distancias[0][nodo + 1]
	for mascara in range(1, 1 << resto):
		fila = costo[mascara]
		for ultimo in range(resto):
			actual = fila[ultimo]
			if actual == math.inf:
				continue
			origen = distancias[ultimo + 1]
			for siguiente in range(resto):
				bit = 1 << siguiente
				if not mascara & bit:
					candidato = actual + origen[siguiente + 1]
					if candidato < costo[mascara | bit][siguiente]:
						costo[mascara | bit][siguiente] = candidato
	completa = (1 << resto) - 1
	return min(costo[completa][nodo] + distancias[nodo + 1][0] for nodo in range(resto))


def validarRutaTsp(distancias, respuesta):
	"""Mismas reglas que el autograder; regresa (correcto, motivo, costoRuta)."""
	cantidad = len(distancias)
	ruta = respuesta.get("route")
	costo = respuesta.get("cost")
	if not isinstance(ruta, list) or len(ruta) != cantidad + 1:
		return False, "la ruta debe tener n+1 nodos", None
	if ruta[0] != 0 or ruta[-1] != 0 or sorted(ruta[:-1]) != list(range(cantidad)):
		return False, "no es un ciclo hamiltoniano desde 0", None
	costoRuta = sum(distancias[ruta[i]][ruta[i + 1]] for i in range(cantidad))
	if not isinstance(costo, (int, float)) or not costosIguales(costo, costoRuta):
		return False, f"cost={costo} no coincide con la ruta ({costoRuta})", None
	return True, "", costoRuta


def perimetroCirculo(nombre, distancias):
	"""Para puntos en un circulo el optimo es recorrerlos en orden angular."""
	cantidad = len(distancias)
	desfase = 3 if cantidad == 10 else 7
	# El nodo i esta en la posicion angular (i * desfase) mod n.
	porPosicion = sorted(range(cantidad), key=lambda nodo: (nodo * desfase) % cantidad)
	return sum(distancias[porPosicion[i]][porPosicion[(i + 1) % cantidad]] for i in range(cantidad))


def probarTsp(nombre, distancias, exigirOptimo=True, costoEsperado=None):
	"""Corre un caso de TSP y lo valida contra el oraculo que aplique."""
	respuesta, segundos = ejecutar("tsp", {"distances": distancias})
	correcto, motivo, costoRuta = validarRutaTsp(distancias, respuesta)
	cantidad = len(distancias)
	detalle = f"n={cantidad}"
	if correcto:
		optimo = costoEsperado
		if exigirOptimo and cantidad <= 9:
			optimo = tspFuerzaBruta(distancias)
		elif exigirOptimo and cantidad <= 16:
			optimo = tspHeldKarp(distancias)
		# El valor escrito a mano tambien debe coincidir con el oraculo.
		if costoEsperado is not None and optimo is not None and not costosIguales(costoEsperado, optimo):
			correcto, motivo = False, f"el costo esperado {costoEsperado} no es el optimo {optimo}"
		if optimo is not None and not costosIguales(costoRuta, optimo):
			correcto, motivo = False, f"costo {costoRuta} != optimo {optimo}"
		detalle += f" cost={respuesta['cost']}" + ("" if optimo is None else " (optimo)")
	reportar(nombre, correcto, motivo or detalle, segundos)
	return segundos


# ------------------------------------------------------------ Voronoi

def areaPoligono(poligono):
	"""Formula del zapato; 0 si tiene menos de tres vertices."""
	if len(poligono) < 3:
		return 0
	return abs(sum(p[0] * q[1] - q[0] * p[1] for p, q in zip(poligono, poligono[1:] + poligono[:1]))) / 2


def voronoiReferencia(sitios, limites):
	"""Recorte por semiplanos igual al de la pagina, sin poda."""
	xMin, yMin, xMax, yMax = limites
	celdas = []
	for propio, (xs, ys) in enumerate(sitios):
		celda = [[xMin, yMin], [xMax, yMin], [xMax, yMax], [xMin, yMax]]
		for otro, (xo, yo) in enumerate(sitios):
			if otro == propio or not celda:
				continue
			a, b = 2 * (xo - xs), 2 * (yo - ys)
			c = xo * xo + yo * yo - xs * xs - ys * ys
			nueva = []
			for i, p in enumerate(celda):
				q = celda[(i + 1) % len(celda)]
				lp, lq = a * p[0] + b * p[1] - c, a * q[0] + b * q[1] - c
				if lp <= 1e-9:
					nueva.append(p)
				if (lp < -1e-9 and lq > 1e-9) or (lp > 1e-9 and lq < -1e-9):
					t = lp / (lp - lq)
					nueva.append([p[0] + t * (q[0] - p[0]), p[1] + t * (q[1] - p[1])])
			celda = nueva
		celdas.append(celda)
	return celdas


def areasPorMuestreo(sitios, limites, lado=200):
	"""Estima el area de cada region asignando una cuadricula al sitio mas cercano."""
	xMin, yMin, xMax, yMax = limites
	ancho, alto = (xMax - xMin) / lado, (yMax - yMin) / lado
	conteo = [0] * len(sitios)
	for i in range(lado):
		x = xMin + (i + 0.5) * ancho
		for j in range(lado):
			y = yMin + (j + 0.5) * alto
			cercano = min(range(len(sitios)), key=lambda k: (sitios[k][0] - x) ** 2 + (sitios[k][1] - y) ** 2)
			conteo[cercano] += 1
	return [c * ancho * alto for c in conteo]


def validarVoronoi(nombre, sitios, limites, respuesta, muestrear):
	"""Revisa formato, areas contra la referencia y las invariantes geometricas."""
	celdas = respuesta.get("cells")
	if not isinstance(celdas, list) or len(celdas) != len(sitios):
		return False, "numero de celdas incorrecto"
	xMin, yMin, xMax, yMax = limites
	areaTotal = (xMax - xMin) * (yMax - yMin)
	escala = max(xMax - xMin, yMax - yMin)
	referencia = voronoiReferencia(sitios, limites)
	for indice, celda in enumerate(celdas):
		if not isinstance(celda, list) or len(celda) < 3:
			return False, f"celda {indice} invalida"
		esperada = areaPoligono(referencia[indice])
		if abs(areaPoligono(celda) - esperada) > TOLERANCIA_AREA * max(1, esperada):
			return False, f"area de la celda {indice}: {areaPoligono(celda)} != {esperada}"
		for x, y in celda:
			tolerancia = 1e-7 * escala
			if not (xMin - tolerancia <= x <= xMax + tolerancia and yMin - tolerancia <= y <= yMax + tolerancia):
				return False, f"vertice ({x}, {y}) fuera del dominio"
			propia = math.hypot(x - sitios[indice][0], y - sitios[indice][1])
			minima = min(math.hypot(x - sx, y - sy) for sx, sy in sitios)
			if propia > minima + 1e-6 * escala:
				return False, f"vertice ({x}, {y}) de la celda {indice} esta mas cerca de otro sitio"
	sinDuplicados = len({tuple(s) for s in sitios}) == len(sitios)
	suma = sum(areaPoligono(c) for c in celdas)
	if sinDuplicados and abs(suma - areaTotal) > 1e-6 * areaTotal:
		return False, f"las areas suman {suma}, el dominio mide {areaTotal}"
	esperadas = AREAS_ESPERADAS.get(nombre)
	if esperadas is not None:
		for indice, area in enumerate(esperadas):
			if abs(areaPoligono(celdas[indice]) - area) > 1e-6 * max(1, area):
				return False, f"celda {indice}: area {areaPoligono(celdas[indice])}, esperada {area}"
	if muestrear and sinDuplicados:
		muestras = areasPorMuestreo(sitios, limites)
		for indice, area in enumerate(muestras):
			if abs(areaPoligono(celdas[indice]) - area) > 0.02 * areaTotal:
				return False, f"celda {indice} no coincide con el muestreo ({area})"
	return True, ""


def probarVoronoi(nombre, sitios, limites, muestrear=True):
	"""Corre un caso de Voronoi y lo valida."""
	respuesta, segundos = ejecutar("voronoi", {"sites": sitios, "bounds": limites})
	correcto, motivo = validarVoronoi(nombre, sitios, limites, respuesta, muestrear)
	reportar(nombre, correcto, motivo or f"sitios={len(sitios)}", segundos)
	return segundos


# ------------------------------------------------- generadores (pagina)

def matrizAleatoria(generador, cantidad, minimo=1, maximo=60):
	"""Matriz simetrica con pesos enteros, como los casos exactos de la pagina."""
	matriz = [[0] * cantidad for _ in range(cantidad)]
	for i in range(cantidad):
		for j in range(i + 1, cantidad):
			matriz[i][j] = matriz[j][i] = generador.randint(minimo, maximo)
	return matriz


def matrizCircular(generador, cantidad):
	"""Puntos casi en un circulo de radio 1e5, como la prueba de estres de la pagina."""
	puntos = []
	for i in range(cantidad):
		angulo = 2 * math.pi * i / cantidad
		radio = 1e5 + generador.randint(-100, 100)
		puntos.append((radio * math.cos(angulo), radio * math.sin(angulo)))
	return [[round(math.hypot(p[0] - q[0], p[1] - q[1])) for q in puntos] for p in puntos]


def sitiosAleatorios(generador, cantidad, maximo=99999):
	"""Sitios enteros distintos en (0, maximo]."""
	vistos = set()
	sitios = []
	while len(sitios) < cantidad:
		sitio = (generador.randint(1, maximo), generador.randint(1, maximo))
		if sitio not in vistos:
			vistos.add(sitio)
			sitios.append(list(sitio))
	return sitios


def main():
	"""Corre todos los grupos de pruebas e imprime el resumen."""
	if not os.path.exists(EJECUTABLE):
		print("Falta ./sentinel: g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel")
		sys.exit(1)
	generador = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 2038)

	print("TSP: casos propios (tests/peticiones/tsp*.json)")
	for archivo in sorted(f for f in os.listdir(PETICIONES) if f.startswith("tsp")):
		with open(os.path.join(PETICIONES, archivo)) as entrada:
			distancias = json.load(entrada)["input"]["distances"]
		esperado = COSTOS_ESPERADOS.get(archivo)
		if archivo.startswith("tsp_circulo"):
			esperado = perimetroCirculo(archivo, distancias)
		probarTsp(archivo, distancias, exigirOptimo=len(distancias) <= 16, costoEsperado=esperado)

	print("TSP: 150 casos aleatorios como los de la pagina (n = 4, 6, 9, pesos 1..60, optimo exigido)")
	for repeticion in range(50):
		for cantidad in (4, 6, 9):
			distancias = matrizAleatoria(generador, cantidad)
			if repeticion < 3:
				probarTsp(f"aleatorio #{repeticion} n={cantidad}", distancias)
			else:
				silencioso(lambda: probarTsp("", distancias))
	print(f"  ... (se muestran 9; el resto se cuenta en el resumen)")

	print("TSP: aleatorios entre 10 y 16 nodos contra Held-Karp en Python")
	for cantidad in (10, 13, 16):
		probarTsp(f"aleatorio n={cantidad}", matrizAleatoria(generador, cantidad, 1, 1000))

	print("TSP: estres (solo ciclo valido y tiempo, como la pagina)")
	for perfil, (nodos, _, limite) in PERFILES_ESTRES.items():
		segundos = probarTsp(f"estres {perfil} n={nodos}", matrizCircular(generador, nodos), exigirOptimo=False)
		revisarTiempo(perfil, segundos, limite)
	probarTsp("estres extra n=1000", matrizCircular(generador, 1000), exigirOptimo=False)

	print("Voronoi: casos propios (tests/peticiones/voronoi*.json)")
	for archivo in sorted(f for f in os.listdir(PETICIONES) if f.startswith("voronoi")):
		with open(os.path.join(PETICIONES, archivo)) as entrada:
			datos = json.load(entrada)["input"]
		probarVoronoi(archivo, datos["sites"], datos["bounds"])

	print("Voronoi: 150 casos aleatorios como los de la pagina (2, 4, 7 sitios en cuadricula de 10)")
	cuadricula = [[x, y] for x in range(5, 96, 10) for y in range(5, 96, 10)]
	for repeticion in range(50):
		for cantidad in (2, 4, 7):
			sitios = generador.sample(cuadricula, cantidad)
			if repeticion < 3:
				probarVoronoi(f"aleatorio #{repeticion} sitios={cantidad}", sitios, [0, 0, 100, 100])
			else:
				silencioso(lambda: probarVoronoi("", sitios, [0, 0, 100, 100], muestrear=False))
	print(f"  ... (se muestran 9; el resto se cuenta en el resumen)")

	print("Voronoi: estres (areas contra la referencia, tiempo)")
	for perfil, (_, sitios, limite) in PERFILES_ESTRES.items():
		segundos = probarVoronoi(f"estres {perfil} sitios={sitios}", sitiosAleatorios(generador, sitios), [0, 0, 100000, 100000], muestrear=False)
		revisarTiempo(perfil, segundos, limite)
	probarVoronoi("estres extra sitios=2000", sitiosAleatorios(generador, 2000), [0, 0, 100000, 100000], muestrear=False)

	print(f"\nResumen: {resultados['pasaron']} pasaron, {resultados['fallaron']} fallaron")
	sys.exit(1 if resultados["fallaron"] else 0)


def silencioso(prueba):
	"""Corre una prueba sin imprimir si pasa; si falla, la imprime."""
	salidaOriginal = sys.stdout
	antes = resultados["fallaron"]
	sys.stdout = open(os.devnull, "w")
	try:
		prueba()
	finally:
		sys.stdout.close()
		sys.stdout = salidaOriginal
	if resultados["fallaron"] > antes:
		print("  FALLA en un caso aleatorio; vuelve a correr con la misma semilla para verlo")


def revisarTiempo(perfil, segundos, limite):
	"""Avisa si el tiempo nativo se acerca al limite del autograder (que corre en WebAssembly, mas lento)."""
	if segundos > limite / 4:
		print(f"    aviso: {segundos:.2f} s nativo; la pagina da {limite} s en WebAssembly para el perfil {perfil}")


if __name__ == "__main__":
	main()
