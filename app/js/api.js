/*
 * Descripcion: estado compartido de la aplicacion y comunicacion con el
 * nucleo en C++. Convierte el escenario activo (red, enlaces caidos,
 * transmisiones y firmas) en las peticiones JSON de cada modo y las envia
 * al servidor, que ejecuta ./sentinel.
 * Equipo: Sentinela del Norte (Equipo 8)
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

const LIMITES_VORONOI = [-12, 34, 32, 63];

const estado = {
	red: null,
	escenarios: [],
	escenario: null,
	resultados: {},
	caminosTsp: null
};

/*
 * Pide al nucleo en C++ que resuelva un modo.
 * Parametros:
 *   modo: "mst", "tsp", "flow", "voronoi", "matching", "palindrome" o "common".
 *   entrada: objeto que va en el campo "input".
 * Retorno: promesa con {respuesta, segundos}; se rechaza con el error del nucleo.
 */
async function llamarNucleo(modo, entrada) {
	const respuestaHttp = await fetch("/api/ejecutar", {
		method: "POST",
		headers: { "Content-Type": "application/json" },
		body: JSON.stringify({ mode: modo, input: entrada })
	});
	const datos = await respuestaHttp.json();
	if (!datos.ok) {
		throw new Error(datos.error || "error desconocido del nucleo");
	}
	return { respuesta: datos.respuesta, segundos: datos.segundos };
}

/*
 * Descarga un archivo JSON de la carpeta de la aplicacion.
 * Parametros:
 *   ruta: ruta relativa del archivo.
 * Retorno: promesa con el objeto leido.
 */
async function cargarJson(ruta) {
	const respuestaHttp = await fetch(ruta);
	return respuestaHttp.json();
}

/*
 * Busca el indice de una central por su codigo de pais.
 * Parametros:
 *   codigo: por ejemplo "UK".
 * Retorno: el indice, o -1 si no existe.
 */
function indicePorCodigo(codigo) {
	return estado.red.nodos.findIndex((nodo) => nodo.codigo === codigo);
}

/*
 * Arma la clave de un enlace sin importar su orden, por ejemplo "AT-DE".
 * Parametros:
 *   u, v: indices de las centrales.
 * Retorno: la clave del enlace.
 */
function claveEnlace(u, v) {
	const codigos = [estado.red.nodos[u].codigo, estado.red.nodos[v].codigo].sort();
	return codigos.join("-");
}

/*
 * Indica si un enlace esta caido en el escenario activo.
 * Parametros:
 *   enlace: objeto con "u" y "v".
 * Retorno: true si el escenario lo marca como caido.
 */
function enlaceCaido(enlace) {
	const clave = claveEnlace(enlace.u, enlace.v);
	return estado.escenario.enlacesCaidos.some((par) => [...par].sort().join("-") === clave);
}

/*
 * Lista los enlaces activos del escenario.
 * Parametros: ninguno.
 * Retorno: arreglo de enlaces que no estan caidos.
 */
function enlacesActivos() {
	return estado.red.enlaces.filter((enlace) => !enlaceCaido(enlace));
}

/*
 * Calcula la distancia minima entre todas las centrales siguiendo los
 * enlaces activos (Floyd-Warshall, O(n^3)). El TSP necesita una matriz
 * completa porque no todas las centrales tienen enlace directo: entre dos
 * paradas se puede pasar por centrales intermedias.
 * Parametros: ninguno.
 * Retorno: {distancias, siguiente}; siguiente[i][j] es el primer paso del
 *   camino de i a j (sirve para dibujar la ruta real).
 */
function calcularCaminosMinimos() {
	const total = estado.red.nodos.length;
	const distancias = Array.from({ length: total }, (_, i) => Array.from({ length: total }, (_, j) => (i === j ? 0 : Infinity)));
	const siguiente = Array.from({ length: total }, (_, i) => Array.from({ length: total }, (_, j) => (i === j ? j : -1)));
	for (const enlace of enlacesActivos()) {
		distancias[enlace.u][enlace.v] = enlace.distanciaKm;
		distancias[enlace.v][enlace.u] = enlace.distanciaKm;
		siguiente[enlace.u][enlace.v] = enlace.v;
		siguiente[enlace.v][enlace.u] = enlace.u;
	}
	for (let k = 0; k < total; k++) {
		for (let i = 0; i < total; i++) {
			for (let j = 0; j < total; j++) {
				if (distancias[i][k] + distancias[k][j] < distancias[i][j]) {
					distancias[i][j] = distancias[i][k] + distancias[k][j];
					siguiente[i][j] = siguiente[i][k];
				}
			}
		}
	}
	return { distancias, siguiente };
}

/*
 * Reconstruye el camino real entre dos centrales.
 * Parametros:
 *   origen, destino: indices de las centrales.
 * Retorno: arreglo de indices desde origen hasta destino.
 */
function caminoEntre(origen, destino) {
	const camino = [origen];
	let actual = origen;
	while (actual !== destino && actual !== -1) {
		actual = estado.caminosTsp.siguiente[actual][destino];
		camino.push(actual);
	}
	return camino;
}

/*
 * Construye la entrada ("input") de un modo con los datos del escenario.
 * Parametros:
 *   modo: nombre del modo.
 * Retorno: el objeto que se envia al nucleo; null si el escenario no lo permite.
 */
function construirEntrada(modo) {
	const red = estado.red;
	const escenario = estado.escenario;
	const activos = enlacesActivos();
	let entrada = null;
	if (modo === "mst") {
		entrada = { nodes: red.nodos.map((nodo) => nodo.id), edges: activos.map((e) => ({ u: e.u, v: e.v, weight: e.costo })) };
	} else if (modo === "tsp") {
		estado.caminosTsp = calcularCaminosMinimos();
		const conexa = estado.caminosTsp.distancias.every((fila) => fila.every(Number.isFinite));
		entrada = conexa ? { distances: estado.caminosTsp.distancias } : null;
	} else if (modo === "flow") {
		const aristas = [];
		for (const e of activos) {
			aristas.push({ u: e.u, v: e.v, capacity: e.capacidadGbps });
			aristas.push({ u: e.v, v: e.u, capacity: e.capacidadGbps });
		}
		entrada = { n: red.nodos.length, edges: aristas, source: indicePorCodigo(escenario.flujo.origen), sink: indicePorCodigo(escenario.flujo.destino), detail: true };
	} else if (modo === "voronoi") {
		entrada = { sites: red.nodos.map((nodo) => [nodo.lon, nodo.lat]), bounds: LIMITES_VORONOI };
	} else if (modo === "matching") {
		entrada = { transmissions: escenario.transmisiones, patterns: escenario.firmas };
	} else {
		entrada = { transmissions: escenario.transmisiones };
	}
	return entrada;
}

/*
 * Ejecuta en el nucleo los cuatro algoritmos de infraestructura del
 * escenario activo y guarda sus resultados.
 * Parametros: ninguno.
 * Retorno: promesa que termina cuando llegan los cuatro resultados.
 */
async function calcularInfraestructura() {
	estado.resultados = {};
	const modos = ["mst", "tsp", "flow", "voronoi"];
	const tareas = modos.map(async (modo) => {
		const entrada = construirEntrada(modo);
		if (entrada === null) {
			estado.resultados[modo] = { error: "la red quedo desconectada" };
		} else {
			estado.resultados[modo] = await llamarNucleo(modo, entrada).catch((error) => ({ error: error.message }));
		}
	});
	await Promise.all(tareas);
}
