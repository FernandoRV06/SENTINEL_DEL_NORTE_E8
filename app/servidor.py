"""
Descripcion: servidor local del SENTINEL Digital Twin. Sirve la pagina web
de la carpeta app/ y, por cada consulta de la pagina, ejecuta el nucleo en
C++ (sentinel) con la peticion JSON por la entrada estandar y devuelve su
respuesta. Asi todos los resultados de la aplicacion salen del C++ en el
momento; no hay resultados precalculados.
Solo usa la biblioteca estandar de Python 3. Si el ejecutable no existe,
lo compila con las mismas opciones que el autograder.
Uso: python3 app/servidor.py  y abrir http://localhost:8000
Equipo: Sentinela del Norte (Equipo 8)
Autores: Carlos Emiliano Nava Flores (A00842650),
         Luis Enrique Castillo Carrillo (A00842390),
         Desiderio Ivan Ortegon Morton (A00840591),
         Fernando Robles Vidrio (A00843290)
Fecha de creacion: 21/10/2026
"""

import http.server
import json
import os
import shutil
import subprocess
import sys
import time
import webbrowser

CARPETA_APP = os.path.dirname(os.path.abspath(__file__))
CARPETA_RAIZ = os.path.dirname(CARPETA_APP)
NOMBRE_EJECUTABLE = "sentinel.exe" if os.name == "nt" else "sentinel"
RUTA_EJECUTABLE = os.path.join(CARPETA_RAIZ, NOMBRE_EJECUTABLE)
RUTA_FUENTE = os.path.join(CARPETA_RAIZ, "src", "main.cpp")
BANDERAS_COMPILACION = ["-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic"]
PUERTO = 8000
TIEMPO_LIMITE_SEGUNDOS = 60
TAMANO_MAXIMO_PETICION = 64 * 1024 * 1024
MODOS_VALIDOS = ["mst", "tsp", "flow", "voronoi", "matching", "palindrome", "common"]


def buscarCompilador():
	"""
	Busca un compilador de C++ instalado.
	Parametros: ninguno.
	Retorno: el nombre del compilador (g++ o clang++), o None si no hay.
	"""
	compilador = None
	for candidato in ["g++", "clang++"]:
		if compilador is None and shutil.which(candidato) is not None:
			compilador = candidato
	return compilador


def compilarSiFalta():
	"""
	Compila src/main.cpp si el ejecutable no existe o es mas viejo que el
	codigo fuente.
	Parametros: ninguno.
	Retorno: texto que describe lo que se hizo.
	"""
	mensaje = "ejecutable al dia"
	fuentes = [os.path.join(CARPETA_RAIZ, "src", nombre) for nombre in os.listdir(os.path.join(CARPETA_RAIZ, "src"))]
	fechaFuentes = max(os.path.getmtime(ruta) for ruta in fuentes)
	necesitaCompilar = not os.path.exists(RUTA_EJECUTABLE) or os.path.getmtime(RUTA_EJECUTABLE) < fechaFuentes
	if necesitaCompilar:
		compilador = buscarCompilador()
		if compilador is None:
			sys.exit("No se encontro g++ ni clang++. Compile a mano: g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel")
		print("Compilando el nucleo en C++ con", compilador, "...")
		resultado = subprocess.run([compilador] + BANDERAS_COMPILACION + [RUTA_FUENTE, "-o", RUTA_EJECUTABLE], capture_output=True, text=True)
		if resultado.returncode != 0:
			sys.exit("Error al compilar:\n" + resultado.stderr)
		mensaje = "compilado con " + compilador
	return mensaje


def ejecutarNucleo(peticion):
	"""
	Ejecuta el nucleo en C++ con una peticion y mide cuanto tarda.
	Parametros:
	  peticion: diccionario con "mode" e "input".
	Retorno: diccionario con "ok", "respuesta" (JSON del C++), "segundos" y,
	  si algo falla, "error".
	"""
	resultado = {"ok": False}
	inicio = time.perf_counter()
	try:
		proceso = subprocess.run([RUTA_EJECUTABLE], input=json.dumps(peticion), capture_output=True, text=True, timeout=TIEMPO_LIMITE_SEGUNDOS)
		resultado["segundos"] = time.perf_counter() - inicio
		respuesta = json.loads(proceso.stdout) if proceso.stdout.strip() else None
		if proceso.returncode != 0 or respuesta is None:
			resultado["error"] = "el nucleo termino con codigo " + str(proceso.returncode) + " " + proceso.stderr[:300]
		elif "error" in respuesta:
			resultado["error"] = respuesta["error"]
		else:
			resultado["ok"] = True
			resultado["respuesta"] = respuesta
	except subprocess.TimeoutExpired:
		resultado["error"] = "el nucleo excedio " + str(TIEMPO_LIMITE_SEGUNDOS) + " segundos"
		resultado["segundos"] = time.perf_counter() - inicio
	return resultado


class ManejadorSentinel(http.server.SimpleHTTPRequestHandler):
	"""
	Atiende la pagina: los archivos de app/ con GET y las consultas al
	nucleo con POST /api/ejecutar.
	"""

	def __init__(self, *argumentos, **opciones):
		"""
		Fija app/ como carpeta raiz de los archivos estaticos.
		Parametros: argumentos y opciones del manejador base de http.server.
		Regresa: nada.
		"""
		super().__init__(*argumentos, directory=CARPETA_APP, **opciones)

	def responderJson(self, codigo, datos):
		"""
		Envia una respuesta JSON.
		Parametros:
		  codigo: codigo HTTP.
		  datos: objeto que se convierte a JSON.
		Retorno: ninguno.
		"""
		cuerpo = json.dumps(datos).encode("utf-8")
		self.send_response(codigo)
		self.send_header("Content-Type", "application/json; charset=utf-8")
		self.send_header("Content-Length", str(len(cuerpo)))
		self.end_headers()
		self.wfile.write(cuerpo)

	def do_GET(self):
		"""
		Responde el estado del servidor en /api/estado; cualquier otra ruta
		es un archivo de la pagina.
		Parametros: ninguno.
		Retorno: ninguno.
		"""
		if self.path == "/api/estado":
			self.responderJson(200, {"ok": True, "ejecutable": NOMBRE_EJECUTABLE, "modos": MODOS_VALIDOS})
		else:
			super().do_GET()

	def do_POST(self):
		"""
		Recibe {"mode": ..., "input": {...}} en /api/ejecutar y responde con el
		resultado del nucleo en C++.
		Parametros: ninguno.
		Retorno: ninguno.
		"""
		longitud = int(self.headers.get("Content-Length", "0"))
		if self.path != "/api/ejecutar":
			self.responderJson(404, {"ok": False, "error": "ruta desconocida"})
		elif longitud <= 0 or longitud > TAMANO_MAXIMO_PETICION:
			self.responderJson(413, {"ok": False, "error": "peticion vacia o demasiado grande"})
		else:
			peticion = json.loads(self.rfile.read(longitud).decode("utf-8"))
			if peticion.get("mode") not in MODOS_VALIDOS:
				self.responderJson(400, {"ok": False, "error": "modo desconocido"})
			else:
				self.responderJson(200, ejecutarNucleo(peticion))

	def log_message(self, formato, *argumentos):
		"""
		Muestra en la terminal solo las consultas al nucleo, no cada archivo.
		Parametros:
		  formato, argumentos: los del registro original.
		Retorno: ninguno.
		"""
		if "/api/" in self.path:
			super().log_message(formato, *argumentos)


def main():
	"""
	Compila si hace falta, inicia el servidor y abre el navegador.
	Parametros: ninguno.
	Retorno: ninguno.
	"""
	print(compilarSiFalta())
	direccion = "http://localhost:" + str(PUERTO)
	servidor = http.server.ThreadingHTTPServer(("127.0.0.1", PUERTO), ManejadorSentinel)
	print("SENTINEL Digital Twin en", direccion, "(Ctrl+C para detener)")
	if "--sin-navegador" not in sys.argv:
		webbrowser.open(direccion)
	try:
		servidor.serve_forever()
	except KeyboardInterrupt:
		print("\nServidor detenido.")


if __name__ == "__main__":
	main()
