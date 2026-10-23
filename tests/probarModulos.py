"""
Descripcion: pruebas automatizadas de los modos "mst", "flow", "matching",
"palindrome" y "common". Ejecuta ./sentinel con cada peticion y valida la
respuesta contra oraculos escritos aparte en Python:
  - mst: Prim O(n^2) y las mismas reglas que el autograder (n - 1 aristas
    existentes, sin ciclos, costo reportado igual a la suma y minimo).
  - flow: Dinic (algoritmo distinto al Edmonds-Karp del C++).
  - matching: str.find.
  - palindrome: expansion desde cada centro (fuerza bruta) en casos chicos.
  - common: programacion dinamica O(n * m) con la regla de empate.
Incluye casos borde propios, casos aleatorios con el formato de la pagina,
pruebas de estres con los tamanos de los perfiles lite, normal y huge, y las
tecnicas alternativas (prim, ford-fulkerson, naive, center, dp).
Uso: python3 tests/probarModulos.py [semilla]  (compila src/main.cpp antes)
Equipo: Sentinela del Norte (Equipo 8)
Autores: Carlos Emiliano Nava Flores (A00842650),
         Luis Enrique Castillo Carrillo (A00842390),
         Desiderio Ivan Ortegon Morton (A00840591),
         Fernando Robles Vidrio (A00843290)
Fecha de creacion: 20/10/2026
"""

import json
import math
import os
import random
import subprocess
import sys
import time
from collections import deque

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EJECUTABLE = os.path.join(RAIZ, "sentinel")
PETICIONES = os.path.join(RAIZ, "tests", "peticiones")
HEXADECIMAL = "0123456789ABCDEF"
TOLERANCIA = 1e-5
PERFILES_ESTRES = {
	"lite": (400, 2000, 80, 600, 50000, 50000, 12000, 6),
	"normal": (1200, 7000, 180, 1800, 220000, 220000, 55000, 8),
	"huge": (5000, 30000, 450, 9000, 1000000, 1000000, 180000, 18),
}

resultados = {"pasaron": 0, "fallaron": 0}
silenciar = {"activo": False}


def ejecutar(modo, entrada):
	"""Corre el ejecutable con una peticion; regresa (respuesta, segundos)."""
	peticion = json.dumps({"mode": modo, "input": entrada})
	inicio = time.perf_counter()
	proceso = subprocess.run([EJECUTABLE], input=peticion, capture_output=True, text=True, timeout=300)
	return json.loads(proceso.stdout), time.perf_counter() - inicio


def reportar(nombre, correcto, detalle, segundos, limite=None):
	"""Imprime una linea de resultado y lleva la cuenta."""
	resultados["pasaron" if correcto else "fallaron"] += 1
	aviso = ""
	if limite is not None and segundos > limite / 4:
		aviso = f"  (aviso: la pagina da {limite} s en WebAssembly)"
	if not correcto or not silenciar["activo"]:
		marca = "OK   " if correcto else "FALLA"
		print(f"  {marca} {nombre:<36} {segundos * 1000:9.1f} ms  {detalle}{aviso}")


def iguales(a, b):
	"""Misma comparacion relativa que usa el autograder."""
	return isinstance(a, (int, float)) and abs(a - b) <= TOLERANCIA * max(1, abs(b))



def mstPrim(cantidad, aristas):
	"""Costo minimo con Prim O(n^2) sobre la matriz de pesos minimos."""
	peso = {}
	for arista in aristas:
		clave = (min(arista["u"], arista["v"]), max(arista["u"], arista["v"]))
		peso[clave] = min(peso.get(clave, math.inf), arista["weight"])
	vecinos = [dict() for _ in range(cantidad)]
	for (u, v), w in peso.items():
		if u != v:
			vecinos[u][v] = w
			vecinos[v][u] = w
	distancia = [math.inf] * cantidad
	enArbol = [False] * cantidad
	distancia[0] = 0
	total = 0
	for _ in range(cantidad):
		elegido = min((d, nodo) for nodo, d in enumerate(distancia) if not enArbol[nodo])[1]
		if distancia[elegido] == math.inf:
			return None
		enArbol[elegido] = True
		total += distancia[elegido]
		for vecino, w in vecinos[elegido].items():
			if not enArbol[vecino] and w < distancia[vecino]:
				distancia[vecino] = w
	return total


def validarMst(entrada, respuesta, optimo):
	"""Mismas reglas que el autograder."""
	cantidad = len(entrada["nodes"])
	aristas = respuesta.get("edges")
	if not isinstance(aristas, list) or len(aristas) != cantidad - 1:
		return False, "debe regresar exactamente n - 1 aristas"
	peso = {}
	for arista in entrada["edges"]:
		clave = (min(arista["u"], arista["v"]), max(arista["u"], arista["v"]))
		peso[clave] = min(peso.get(clave, math.inf), arista["weight"])
	padre = list(range(cantidad))

	def raiz(nodo):
		"""Representante del conjunto de nodo, con compresion de caminos."""
		while padre[nodo] != nodo:
			padre[nodo] = padre[padre[nodo]]
			nodo = padre[nodo]
		return nodo

	suma = 0
	for arista in aristas:
		u, v = arista[0], arista[1]
		clave = (min(u, v), max(u, v))
		if clave not in peso:
			return False, f"usa la arista inexistente {clave}"
		if raiz(u) == raiz(v):
			return False, "contiene un ciclo"
		padre[raiz(u)] = raiz(v)
		suma += peso[clave]
	if not iguales(respuesta.get("cost"), suma):
		return False, f"cost={respuesta.get('cost')} no coincide con las aristas ({suma})"
	if not iguales(suma, optimo):
		return False, f"costo {suma} no es minimo ({optimo})"
	return True, ""


def probarMst(nombre, entrada, limite=None):
	"""Corre un caso de MST y lo valida contra Prim."""
	respuesta, segundos = ejecutar("mst", entrada)
	optimo = mstPrim(len(entrada["nodes"]), entrada["edges"])
	correcto, motivo = validarMst(entrada, respuesta, optimo)
	reportar(nombre, correcto, motivo or f"n={len(entrada['nodes'])} cost={respuesta.get('cost')}", segundos, limite)



def flujoDinic(cantidad, aristas, fuente, sumidero):
	"""Flujo maximo con Dinic (niveles BFS + bloqueo DFS iterativo)."""
	if fuente == sumidero:
		return 0
	destino, capacidad, siguiente = [], [], []
	primera = [-1] * cantidad
	for arista in aristas:
		for u, v, c in ((arista["u"], arista["v"], arista["capacity"]), (arista["v"], arista["u"], 0)):
			destino.append(v)
			capacidad.append(c)
			siguiente.append(primera[u])
			primera[u] = len(destino) - 1
	total = 0
	while True:
		nivel = [-1] * cantidad
		nivel[fuente] = 0
		cola = deque([fuente])
		while cola:
			nodo = cola.popleft()
			arista = primera[nodo]
			while arista != -1:
				if capacidad[arista] > 1e-12 and nivel[destino[arista]] < 0:
					nivel[destino[arista]] = nivel[nodo] + 1
					cola.append(destino[arista])
				arista = siguiente[arista]
		if nivel[sumidero] < 0:
			return total
		actual = primera[:]
		hayCamino = True
		while hayCamino:
			camino = []
			nodo = fuente
			atorado = False
			# Avanza por aristas del grafo de niveles; si se atora en la fuente, ya no hay camino
			while nodo != sumidero and not atorado:
				arista = actual[nodo]
				while arista != -1 and not (capacidad[arista] > 1e-12 and nivel[destino[arista]] == nivel[nodo] + 1):
					arista = siguiente[arista]
				actual[nodo] = arista
				if arista == -1 and not camino:
					atorado = True
				elif arista == -1:
					nivel[nodo] = -1
					ultima = camino.pop()
					nodo = destino[ultima ^ 1]
					actual[nodo] = siguiente[actual[nodo]]
				else:
					camino.append(arista)
					nodo = destino[arista]
			hayCamino = nodo == sumidero
			if hayCamino:
				cuello = min(capacidad[a] for a in camino)
				for a in camino:
					capacidad[a] -= cuello
					capacidad[a ^ 1] += cuello
				total += cuello


def probarFlujo(nombre, entrada, limite=None):
	"""Corre un caso de flujo y lo compara con Dinic."""
	respuesta, segundos = ejecutar("flow", entrada)
	esperado = flujoDinic(entrada["n"], entrada["edges"], entrada["source"], entrada["sink"])
	correcto = iguales(respuesta.get("flow"), esperado)
	motivo = "" if correcto else f"flow={respuesta.get('flow')}, esperado {esperado}"
	reportar(nombre, correcto, motivo or f"n={entrada['n']} flow={respuesta.get('flow')}", segundos, limite)



def probarFirmas(nombre, entrada, limite=None):
	"""Corre un caso de firmas y lo compara con str.find."""
	respuesta, segundos = ejecutar("matching", entrada)
	esperado = [[(t.find(p) + 1) if t.find(p) >= 0 else -1 for p in entrada["patterns"]] for t in entrada["transmissions"]]
	correcto = respuesta.get("matches") == esperado
	motivo = "" if correcto else f"{respuesta.get('matches')} != {esperado}"
	reportar(nombre, correcto, motivo or f"matches={esperado}", segundos, limite)


def palindromoFuerzaBruta(texto):
	"""Expande desde cada centro; con empate gana el menor inicio."""
	if not texto:
		return [0, 0]
	mejor = [1, 1]
	for centro in range(len(texto)):
		for ajuste in (0, 1):
			izquierda, derecha = centro, centro + ajuste
			while izquierda >= 0 and derecha < len(texto) and texto[izquierda] == texto[derecha]:
				izquierda -= 1
				derecha += 1
			longitud = derecha - izquierda - 1
			if longitud > mejor[1] - mejor[0] + 1 or (longitud == mejor[1] - mejor[0] + 1 and longitud > 0 and izquierda + 2 < mejor[0]):
				mejor = [izquierda + 2, derecha]
	return mejor


def palindromoManacherReferencia(texto):
	"""Manacher sobre "^#a#b#...#$", como la referencia de la pagina (para estres)."""
	if not texto:
		return [0, 0]
	t = "^#" + "#".join(texto) + "#$"
	radio = [0] * len(t)
	centro = derecha = 0
	mejorLongitud, mejorInicio = 1, 0
	for i in range(1, len(t) - 1):
		if i < derecha:
			radio[i] = min(derecha - i, radio[2 * centro - i])
		while t[i + 1 + radio[i]] == t[i - 1 - radio[i]]:
			radio[i] += 1
		if i + radio[i] > derecha:
			centro, derecha = i, i + radio[i]
		inicio = (i - radio[i]) >> 1
		if radio[i] > mejorLongitud or (radio[i] == mejorLongitud and inicio < mejorInicio):
			mejorLongitud, mejorInicio = radio[i], inicio
	return [mejorInicio + 1, mejorInicio + mejorLongitud]


def probarPalindromo(nombre, entrada, oraculo=palindromoFuerzaBruta, limite=None):
	"""Corre un caso de palindromo y lo compara con el oraculo."""
	respuesta, segundos = ejecutar("palindrome", entrada)
	esperado = [oraculo(t) for t in entrada["transmissions"]]
	correcto = respuesta.get("ranges") == esperado
	motivo = "" if correcto else f"{respuesta.get('ranges')} != {esperado}"
	reportar(nombre, correcto, motivo or f"ranges={esperado}", segundos, limite)


def comunProgramacionDinamica(primera, segunda):
	"""Subcadena comun mas larga O(n * m); empate: menor inicio en t1 y luego en t2."""
	mejor = (0, 0, 0)
	anterior = [0] * (len(segunda) + 1)
	for i in range(len(primera)):
		actual = [0] * (len(segunda) + 1)
		for j in range(len(segunda)):
			if primera[i] == segunda[j]:
				longitud = anterior[j] + 1
				actual[j + 1] = longitud
				candidato = (longitud, -(i - longitud + 1), -(j - longitud + 1))
				if candidato > mejor:
					mejor = candidato
		anterior = actual
	if mejor[0] == 0:
		return [0, 0]
	return [-mejor[1] + 1, -mejor[1] + mejor[0]]


def probarComun(nombre, entrada, limite=None, longitudMinima=None):
	"""Corre un caso de subcadena comun; en estres valida como la pagina."""
	respuesta, segundos = ejecutar("common", entrada)
	primera, segunda = entrada["transmissions"]
	rango = respuesta.get("range")
	if longitudMinima is None:
		esperado = comunProgramacionDinamica(primera, segunda)
		correcto = rango == esperado
		motivo = "" if correcto else f"{rango} != {esperado}"
	else:
		correcto = isinstance(rango, list) and len(rango) == 2 and 1 <= rango[0] <= rango[1] <= len(primera)
		if correcto:
			subcadena = primera[rango[0] - 1:rango[1]]
			correcto = len(subcadena) >= longitudMinima and subcadena in segunda
		motivo = "" if correcto else f"rango {rango} invalido o muy corto (minimo {longitudMinima})"
	reportar(nombre, correcto, motivo or f"range={rango}", segundos, limite)



def grafoMst(generador, cantidad, extra, pesoMaximo=25, densidad=None):
	"""Camino 0-1-...-n-1 mas aristas extra, como los casos de la pagina."""
	aristas = [{"u": i, "v": i + 1, "weight": generador.randint(1, pesoMaximo)} for i in range(cantidad - 1)]
	if densidad is not None:
		for u in range(cantidad):
			for v in range(u + 2, cantidad):
				if generador.random() < densidad:
					aristas.append({"u": u, "v": v, "weight": generador.randint(1, pesoMaximo)})
	usadas = {(a["u"], a["v"]) for a in aristas}
	while len(aristas) < cantidad - 1 + extra:
		u, v = sorted((generador.randrange(cantidad), generador.randrange(cantidad)))
		if u != v and (u, v) not in usadas:
			usadas.add((u, v))
			aristas.append({"u": u, "v": v, "weight": generador.randint(1, pesoMaximo)})
	return {"nodes": list(range(cantidad)), "edges": aristas}


def redFlujo(generador, cantidad, extra, densidad=None, capacidadMaxima=35):
	"""Camino 0 -> 1 -> ... -> n-1 mas aristas dirigidas extra."""
	aristas = [{"u": i, "v": i + 1, "capacity": generador.randint(5, 30)} for i in range(cantidad - 1)]
	if densidad is not None:
		for u in range(cantidad):
			for v in range(cantidad):
				if u != v and generador.random() < densidad:
					aristas.append({"u": u, "v": v, "capacity": generador.randint(1, capacidadMaxima)})
	usadas = {(a["u"], a["v"]) for a in aristas}
	while len(aristas) < cantidad - 1 + extra:
		u, v = generador.randrange(cantidad), generador.randrange(cantidad)
		if u != v and (u, v) not in usadas:
			usadas.add((u, v))
			aristas.append({"u": u, "v": v, "capacity": generador.randint(1, capacidadMaxima)})
	return {"n": cantidad, "edges": aristas, "source": 0, "sink": cantidad - 1}


def hexAleatorio(generador, longitud, alfabeto=HEXADECIMAL):
	"""Cadena aleatoria sobre el alfabeto dado."""
	return "".join(generador.choice(alfabeto) for _ in range(longitud))


def conTecnica(entrada, tecnica):
	"""Copia la entrada agregando el campo opcional "technique"."""
	copia = dict(entrada)
	copia["technique"] = tecnica
	return copia


def leerPeticion(archivo):
	"""Lee el campo "input" de una peticion de tests/peticiones."""
	with open(os.path.join(PETICIONES, archivo)) as entrada:
		return json.load(entrada)["input"]


def correrSilencioso(titulo, pruebas):
	"""Corre muchas pruebas imprimiendo solo las que fallan."""
	antes = dict(resultados)
	silenciar["activo"] = True
	for prueba in pruebas:
		prueba()
	silenciar["activo"] = False
	pasaron = resultados["pasaron"] - antes["pasaron"]
	fallaron = resultados["fallaron"] - antes["fallaron"]
	print(f"  {'OK   ' if fallaron == 0 else 'FALLA'} {titulo}: {pasaron} pasaron, {fallaron} fallaron")


def main():
	"""Corre todos los grupos de pruebas e imprime el resumen."""
	if not os.path.exists(EJECUTABLE):
		print("Falta ./sentinel: g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel")
		sys.exit(1)
	generador = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 2038)

	print("MST")
	probarMst("mst.json (ejemplo)", leerPeticion("mst.json"))
	probarMst("un nodo", {"nodes": [0], "edges": []})
	probarMst("dos nodos", {"nodes": [0, 1], "edges": [{"u": 0, "v": 1, "weight": 9}]})
	probarMst("aristas repetidas", {"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 1, "weight": 9}, {"u": 1, "v": 0, "weight": 2}, {"u": 1, "v": 2, "weight": 3}, {"u": 0, "v": 2, "weight": 3}]})
	probarMst("pesos iguales", {"nodes": list(range(5)), "edges": [{"u": u, "v": v, "weight": 7} for u in range(5) for v in range(u + 1, 5)]})
	probarMst("pesos negativos y decimales", {"nodes": [0, 1, 2, 3], "edges": [{"u": 0, "v": 1, "weight": -2.5}, {"u": 1, "v": 2, "weight": 0.25}, {"u": 2, "v": 3, "weight": -1}, {"u": 0, "v": 3, "weight": 0.5}, {"u": 0, "v": 2, "weight": -0.75}]})
	probarMst("lazos (u == v)", {"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 0, "weight": -5}, {"u": 0, "v": 1, "weight": 1}, {"u": 1, "v": 2, "weight": 1}]})
	probarMst("grafo completo 40", grafoMst(generador, 40, 0, 1000, densidad=1.0))
	correrSilencioso("300 aleatorios como la pagina (4, 7, 11 nodos)", [lambda n=n: probarMst("", grafoMst(generador, n, 0, densidad=0.4)) for _ in range(100) for n in (4, 7, 11)])
	for perfil, (n, extra, *_, limite) in PERFILES_ESTRES.items():
		probarMst(f"estres {perfil} n={n} m={n - 1 + extra}", grafoMst(generador, n, extra, 1000), limite)

	print("Flujo maximo")
	probarFlujo("flow.json (ejemplo)", leerPeticion("flow.json"))
	probarFlujo("sumidero inalcanzable", {"n": 3, "edges": [{"u": 0, "v": 1, "capacity": 5}, {"u": 2, "v": 1, "capacity": 5}], "source": 0, "sink": 2})
	probarFlujo("aristas dirigidas (sentido contrario)", {"n": 2, "edges": [{"u": 1, "v": 0, "capacity": 10}], "source": 0, "sink": 1})
	probarFlujo("aristas paralelas", {"n": 2, "edges": [{"u": 0, "v": 1, "capacity": 3}, {"u": 0, "v": 1, "capacity": 4}], "source": 0, "sink": 1})
	probarFlujo("antiparalelas y ciclo", {"n": 4, "edges": [{"u": 0, "v": 1, "capacity": 10}, {"u": 1, "v": 2, "capacity": 4}, {"u": 2, "v": 1, "capacity": 6}, {"u": 2, "v": 3, "capacity": 8}, {"u": 1, "v": 3, "capacity": 2}, {"u": 3, "v": 0, "capacity": 9}], "source": 0, "sink": 3})
	probarFlujo("capacidad cero", {"n": 3, "edges": [{"u": 0, "v": 1, "capacity": 0}, {"u": 1, "v": 2, "capacity": 7}], "source": 0, "sink": 2})
	probarFlujo("capacidades decimales", {"n": 3, "edges": [{"u": 0, "v": 1, "capacity": 1.5}, {"u": 0, "v": 2, "capacity": 0.25}, {"u": 1, "v": 2, "capacity": 2.75}], "source": 0, "sink": 2})
	probarFlujo("fuente igual a sumidero", {"n": 2, "edges": [{"u": 0, "v": 1, "capacity": 5}], "source": 1, "sink": 1})
	probarFlujo("fuente y sumidero en medio", {"n": 5, "edges": [{"u": 0, "v": 1, "capacity": 4}, {"u": 1, "v": 2, "capacity": 3}, {"u": 2, "v": 3, "capacity": 5}, {"u": 3, "v": 4, "capacity": 6}, {"u": 1, "v": 3, "capacity": 2}], "source": 1, "sink": 3})
	correrSilencioso("300 aleatorios como la pagina (4, 7, 10 nodos)", [lambda n=n: probarFlujo("", redFlujo(generador, n, 0, densidad=0.18)) for _ in range(100) for n in (4, 7, 10)])
	for perfil, (_, _, n, extra, *_, limite) in PERFILES_ESTRES.items():
		red = redFlujo(generador, n, extra, capacidadMaxima=10000)
		for arista in red["edges"][:n - 1]:
			arista["capacity"] = generador.randint(100, 10000)
		probarFlujo(f"estres {perfil} n={n} m={len(red['edges'])}", red, limite)

	print("Deteccion de firmas")
	probarFirmas("matching1.json (ejemplo)", leerPeticion("matching1.json"))
	probarFirmas("matching2.json (ejemplo)", leerPeticion("matching2.json"))
	probarFirmas("traslapes y prefijos repetidos", {"transmissions": ["AAAAAB", "ABABABAC"], "patterns": ["AAAB", "ABABAC", "BA"]})
	probarFirmas("firma al final y mas larga", {"transmissions": ["0123456789", "FF"], "patterns": ["789", "0123456789A", "FFF"]})
	probarFirmas("firma igual a la transmision", {"transmissions": ["DEADBEEF", "BEEF"], "patterns": ["DEADBEEF", "BEEF", "F"]})
	probarFirmas("transmision vacia", {"transmissions": ["", "ABC"], "patterns": ["A", "C", "D"]})
	correrSilencioso("300 aleatorios (alfabeto chico para forzar traslapes)", [lambda: probarFirmas("", {"transmissions": [hexAleatorio(generador, 60, "AB"), hexAleatorio(generador, 60, "AB")], "patterns": [hexAleatorio(generador, generador.randint(1, 6), "AB") for _ in range(3)]}) for _ in range(300)])
	for perfil, (*_, largo, _, _, limite) in PERFILES_ESTRES.items():
		firmas = ["DEADBEEFCAFEBABE", "0123456789ABCDEF0123456789ABCDEF", "FACEFACEFACEFACEFACE", "FFFFEEEEDDDDCCCCBBBB"]
		t1, t2 = hexAleatorio(generador, largo), hexAleatorio(generador, largo)
		t1 = t1[:largo // 3] + firmas[0] + t1[largo // 3:]
		t2 = t2[:2 * largo // 3] + firmas[1] + t2[2 * largo // 3:]
		probarFirmas(f"estres {perfil} largo={largo}", {"transmissions": [t1, t2], "patterns": firmas}, limite)

	print("Palindromo mas largo")
	probarPalindromo("palindrome1.json (ejemplo)", leerPeticion("palindrome1.json"))
	probarPalindromo("palindrome2.json (ejemplo)", leerPeticion("palindrome2.json"))
	probarPalindromo("vacia, un caracter, todo igual", {"transmissions": ["", "A", "FFFFFF"]})
	probarPalindromo("empates par e impar", {"transmissions": ["ABBACDC", "ABACDDC", "12321ABBA"]})
	probarPalindromo("palindromo al final", {"transmissions": ["0123ABCBA", "01ABCCBA"]})
	correrSilencioso("400 aleatorios contra fuerza bruta", [lambda: probarPalindromo("", {"transmissions": [hexAleatorio(generador, generador.randint(1, 80), "AB"), hexAleatorio(generador, generador.randint(1, 80), "ABC")]}) for _ in range(400)])
	for perfil, (*_, largo, _, limite) in PERFILES_ESTRES.items():
		mitad = hexAleatorio(generador, largo // 4)
		centro = mitad + "A" + mitad[::-1]
		t1 = hexAleatorio(generador, largo // 4) + centro + hexAleatorio(generador, largo // 4)
		probarPalindromo(f"estres {perfil} largo={largo}", {"transmissions": [t1, hexAleatorio(generador, largo)]}, palindromoManacherReferencia, limite)

	print("Subcadena comun mas larga")
	probarComun("common1.json (ejemplo)", leerPeticion("common1.json"))
	probarComun("common2.json (ejemplo)", leerPeticion("common2.json"))
	probarComun("sin caracteres en comun", {"transmissions": ["AAAA", "BBBB"]})
	probarComun("una vacia", {"transmissions": ["", "ABC"]})
	probarComun("identicas", {"transmissions": ["ABCDEF", "ABCDEF"]})
	probarComun("una dentro de otra", {"transmissions": ["CDE", "ABCDEFG"]})
	probarComun("repetidas (empate de inicio)", {"transmissions": ["ABAB", "BABA"]})
	correrSilencioso("400 aleatorios contra programacion dinamica", [lambda: probarComun("", {"transmissions": [hexAleatorio(generador, generador.randint(1, 60), "ABC"), hexAleatorio(generador, generador.randint(1, 60), "ABC")]}) for _ in range(400)])
	probarComun("mediano 700x700 contra la DP", {"transmissions": [hexAleatorio(generador, 700, "AB"), hexAleatorio(generador, 700, "AB")]})
	for perfil, (*_, largo, limite) in PERFILES_ESTRES.items():
		bloque = hexAleatorio(generador, max(500, largo // 8))
		mitad = largo // 2
		t1 = hexAleatorio(generador, mitad) + bloque + hexAleatorio(generador, mitad)
		t2 = hexAleatorio(generador, mitad) + bloque + hexAleatorio(generador, mitad)
		probarComun(f"estres {perfil} largo={len(t1)}", {"transmissions": [t1, t2]}, limite, longitudMinima=len(bloque))

	print("Tecnicas alternativas (campo opcional \"technique\", mismos oraculos)")
	correrSilencioso("mst prim: 300 aleatorios", [lambda n=n: probarMst("", conTecnica(grafoMst(generador, n, 0, densidad=0.4), "prim")) for _ in range(100) for n in (4, 7, 11)])
	probarMst("mst prim: aristas repetidas y lazos", conTecnica({"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 0, "weight": -5}, {"u": 0, "v": 1, "weight": 9}, {"u": 1, "v": 0, "weight": 2}, {"u": 1, "v": 2, "weight": 3}]}, "prim"))
	probarMst("mst prim: estres n=5000", conTecnica(grafoMst(generador, 5000, 30000, 1000), "prim"))
	correrSilencioso("flow ford-fulkerson: 300 aleatorios", [lambda n=n: probarFlujo("", conTecnica(redFlujo(generador, n, 0, densidad=0.18), "ford-fulkerson")) for _ in range(100) for n in (4, 7, 10)])
	probarFlujo("flow ford-fulkerson: estres n=180", conTecnica(redFlujo(generador, 180, 1800, capacidadMaxima=10000), "ford-fulkerson"))
	correrSilencioso("matching naive: 300 aleatorios", [lambda: probarFirmas("", conTecnica({"transmissions": [hexAleatorio(generador, 60, "AB"), hexAleatorio(generador, 60, "AB")], "patterns": [hexAleatorio(generador, generador.randint(0, 6), "AB") for _ in range(3)]}, "naive")) for _ in range(300)])
	correrSilencioso("palindrome center: 400 aleatorios", [lambda: probarPalindromo("", conTecnica({"transmissions": ["", hexAleatorio(generador, generador.randint(1, 80), "AB"), hexAleatorio(generador, generador.randint(1, 80), "ABC")]}, "center")) for _ in range(400)])
	correrSilencioso("common dp: 400 aleatorios", [lambda: probarComun("", conTecnica({"transmissions": [hexAleatorio(generador, generador.randint(0, 60), "ABC"), hexAleatorio(generador, generador.randint(0, 60), "ABC")]}, "dp")) for _ in range(400)])
	respuesta, segundos = ejecutar("mst", conTecnica({"nodes": [0], "edges": []}, "inexistente"))
	reportar("tecnica desconocida -> error", "error" in respuesta, f"respuesta={respuesta}", segundos)

	print(f"\nResumen: {resultados['pasaron']} pasaron, {resultados['fallaron']} fallaron")
	sys.exit(1 if resultados["fallaron"] else 0)


if __name__ == "__main__":
	main()
