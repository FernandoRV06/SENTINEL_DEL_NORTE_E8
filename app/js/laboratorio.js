/*
 * Descripcion: vista Algorithm Laboratory. Corre todas las tecnicas de un
 * problema sobre los mismos datos (los del escenario o una instancia
 * aleatoria del tamano indicado) y compara su tiempo y su resultado.
 * Equipo: Sentinela del Norte (Equipo 8)
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

const HEXADECIMAL = "0123456789ABCDEF";
const MAXIMO_CARACTERES_VISIBLES = 3000;

const PROBLEMAS = {
	mst: {
		nombre: "MST · conectar todo al menor costo",
		tecnicas: { kruskal: "Kruskal", prim: "Prim" },
		tamano: 2000,
		descripcion: "Las dos técnicas deben dar el mismo costo. Kruskal ordena los enlaces y toma los más baratos; Prim crece el árbol desde una central."
	},
	tsp: {
		nombre: "TSP · ruta del técnico",
		tecnicas: { "held-karp": "Held-Karp (exacto)", "nearest-neighbor": "Vecino más cercano", "2-opt": "Vecino más cercano + 2-opt" },
		tamano: 12,
		descripcion: "Held-Karp da la ruta óptima pero su tiempo crece exponencialmente: solo sirve hasta unas 20 centrales. Las otras dos son aproximadas: muy rápidas, pero su ruta puede ser un poco más larga."
	},
	flow: {
		nombre: "Flujo máximo",
		tecnicas: { "edmonds-karp": "Edmonds-Karp", "ford-fulkerson": "Ford-Fulkerson" },
		tamano: 300,
		descripcion: "Las dos técnicas deben dar el mismo flujo. Edmonds-Karp busca siempre el camino más corto, por eso su tiempo no depende de qué tan grandes sean las capacidades."
	},
	voronoi: {
		nombre: "Voronoi · zona de cada central",
		tecnicas: { "half-plane": "Recorte por semiplanos", "half-plane-naive": "Recorte sin poda" },
		tamano: 300,
		descripcion: "Las dos técnicas deben dar las mismas zonas. La principal descarta antes las centrales lejanas, por eso trabaja menos."
	},
	matching: {
		nombre: "Firmas de malware",
		tecnicas: { kmp: "KMP", naive: "Búsqueda ingenua" },
		tamano: 200000,
		descripcion: "Las dos técnicas deben encontrar las mismas posiciones. KMP nunca retrocede en la transmisión; la ingenua puede comparar el mismo carácter muchas veces."
	},
	palindrome: {
		nombre: "Palíndromo más largo",
		tecnicas: { manacher: "Manacher", center: "Expansión desde el centro" },
		tamano: 100000,
		descripcion: "Las dos técnicas deben dar el mismo palíndromo. Manacher es lineal en el peor caso; la expansión desde el centro es cuadrática en el peor caso, aunque con datos aleatorios suele ser rápida."
	},
	common: {
		nombre: "Subcadena común más larga",
		tecnicas: { "suffix-array": "Arreglo de sufijos", dp: "Programación dinámica" },
		tamano: 20000,
		descripcion: "Las dos técnicas deben dar la misma subcadena. La programación dinámica compara cada carácter contra cada carácter; con transmisiones grandes se nota mucho más lenta."
	}
};

/*
 * Generador pseudoaleatorio con semilla (mulberry32), para que las
 * instancias aleatorias se puedan repetir.
 * Parametros:
 *   semilla: numero entero inicial.
 * Retorno: funcion que devuelve numeros entre 0 y 1.
 */
function crearAleatorio(semilla) {
	let s = semilla >>> 0;
	return () => {
		s = (s + 0x6d2b79f5) >>> 0;
		let t = Math.imul(s ^ (s >>> 15), 1 | s);
		t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
		return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
	};
}

/*
 * Genera una instancia aleatoria con el formato del autograder.
 * Parametros:
 *   modo: nombre del modo.
 *   n: tamano de la instancia.
 * Retorno: el objeto "input" del modo.
 */
function generarInstancia(modo, n) {
	const azar = crearAleatorio(2038);
	const entero = (minimo, maximo) => minimo + Math.floor(azar() * (maximo - minimo + 1));
	const cadena = (largo) => Array.from({ length: Math.floor(largo) }, () => HEXADECIMAL[entero(0, 15)]).join("");
	const tercio = Math.floor(n / 3);
	const mitad = Math.floor(n / 2);
	let entrada = null;
	if (modo === "mst" || modo === "flow") {
		const aristas = [];
		for (let i = 0; i + 1 < n; i++) {
			aristas.push({ u: i, v: i + 1, valor: entero(1, 1000) });
		}
		for (let k = 0; k < 4 * n; k++) {
			aristas.push({ u: entero(0, n - 1), v: entero(0, n - 1), valor: entero(1, 1000) });
		}
		const validas = aristas.filter((a) => a.u !== a.v);
		entrada = modo === "mst"
			? { nodes: Array.from({ length: n }, (_, i) => i), edges: validas.map((a) => ({ u: a.u, v: a.v, weight: a.valor })) }
			: { n: n, edges: validas.map((a) => ({ u: a.u, v: a.v, capacity: a.valor })), source: 0, sink: n - 1 };
	} else if (modo === "tsp") {
		const puntos = Array.from({ length: n }, () => [entero(0, 1000), entero(0, 1000)]);
		entrada = { distances: puntos.map((p) => puntos.map((q) => Math.round(Math.hypot(p[0] - q[0], p[1] - q[1])))) };
	} else if (modo === "voronoi") {
		const sitios = new Map();
		while (sitios.size < n) {
			const sitio = [entero(1, 99999), entero(1, 99999)];
			sitios.set(sitio.join(","), sitio);
		}
		entrada = { sites: [...sitios.values()], bounds: [0, 0, 100000, 100000] };
	} else if (modo === "matching") {
		const firmas = ["DEADBEEFCAFEBABE", "0123456789ABCDEF", "FACEFACEFACE"];
		const t1 = cadena(n);
		const t2 = cadena(n);
		entrada = { transmissions: [t1.slice(0, tercio) + firmas[0] + t1.slice(tercio), t2.slice(0, mitad) + firmas[1] + t2.slice(mitad)], patterns: firmas };
	} else if (modo === "palindrome") {
		const lado = cadena(n / 8);
		const espejo = lado + "A" + [...lado].reverse().join("");
		entrada = { transmissions: [cadena(n / 4) + espejo + cadena(n / 4), cadena(n)] };
	} else {
		const compartida = cadena(Math.max(10, n / 10));
		entrada = { transmissions: [cadena(n / 2) + compartida + cadena(n / 2), cadena(n / 3) + compartida + cadena(n / 2)] };
	}
	return entrada;
}

/*
 * Resume una respuesta del nucleo en una linea legible.
 * Parametros:
 *   modo: nombre del modo.
 *   respuesta: JSON devuelto por el nucleo.
 * Retorno: texto corto con el resultado principal.
 */
function resumirRespuesta(modo, respuesta) {
	const resumenes = {
		mst: () => `costo ${respuesta.cost.toLocaleString()}`,
		tsp: () => `ruta de ${respuesta.cost.toLocaleString()}`,
		flow: () => `flujo de ${respuesta.flow.toLocaleString()}`,
		voronoi: () => `${respuesta.cells.length} zonas`,
		matching: () => `posiciones ${JSON.stringify(respuesta.matches)}`,
		palindrome: () => `posiciones ${respuesta.ranges.map((r) => r.join("–")).join(" y ")}`,
		common: () => `posiciones ${respuesta.range.join("–")}`
	};
	return resumenes[modo]();
}

/*
 * Valor que debe coincidir entre tecnicas exactas. En Voronoi se comparan
 * las areas de las zonas.
 * Parametros:
 *   modo: nombre del modo.
 *   respuesta: JSON devuelto por el nucleo.
 * Retorno: texto comparable.
 */
function valorComparable(modo, respuesta) {
	return modo === "voronoi" ? respuesta.cells.map((celda) => areaPoligono(celda).toFixed(1)).join(",") : resumirRespuesta(modo, respuesta);
}

/*
 * Area de un poligono con la formula del zapatero.
 * Parametros:
 *   poligono: arreglo de vertices [x, y].
 * Retorno: el area.
 */
function areaPoligono(poligono) {
	let suma = 0;
	poligono.forEach(([x, y], i) => {
		const [xs, ys] = poligono[(i + 1) % poligono.length];
		suma += x * ys - xs * y;
	});
	return Math.abs(suma) / 2;
}

/*
 * Explica en palabras el renglon de una tecnica.
 * Parametros:
 *   modo: nombre del modo.
 *   resultado: {tecnica, respuesta, segundos} o {tecnica, error}.
 *   referencia: resultado de la primera tecnica que no fallo.
 *   esPrincipal: true si es la tecnica que usa el autograder.
 *   nombreReferencia: nombre legible de la tecnica de referencia.
 * Retorno: texto para la columna "¿Que significa?".
 */
function explicarRenglon(modo, resultado, referencia, esPrincipal, nombreReferencia) {
	let texto = "";
	if (resultado.error) {
		texto = resultado.error.includes("20 centrales") ? "No aplica: demasiadas centrales para el método exacto." : `Error: ${resultado.error}`;
	} else if (resultado === referencia) {
		texto = esPrincipal ? "Técnica principal (la que usa el autograder)." : "Aproximada; se toma como referencia porque el método exacto no aplica.";
	} else if (valorComparable(modo, resultado.respuesta) === valorComparable(modo, referencia.respuesta)) {
		texto = `Mismo resultado; tarda ${(resultado.segundos / referencia.segundos).toFixed(1)} veces lo que ${nombreReferencia}.`;
	} else if (modo === "tsp" && referencia.tecnica === "held-karp") {
		texto = `Aproximada: ruta ${((resultado.respuesta.cost / referencia.respuesta.cost - 1) * 100).toFixed(1)} % más larga que la óptima.`;
	} else if (modo === "tsp") {
		const diferencia = (1 - resultado.respuesta.cost / referencia.respuesta.cost) * 100;
		texto = `Aproximada: ruta ${Math.abs(diferencia).toFixed(1)} % ${diferencia > 0 ? "más corta" : "más larga"} que la de ${nombreReferencia}.`;
	} else {
		texto = `Resultado distinto al de ${nombreReferencia}.`;
	}
	return texto;
}

/*
 * Prepara los datos de la comparacion y muestra la peticion.
 * Parametros: ninguno.
 * Retorno: el objeto "input" que se va a enviar.
 */
function prepararInstancia() {
	const modo = document.getElementById("labModo").value;
	const aleatoria = document.getElementById("labFuente").value === "aleatoria";
	document.getElementById("labCampoTamano").style.display = aleatoria ? "" : "none";
	const entrada = aleatoria ? generarInstancia(modo, Math.max(2, Number(document.getElementById("labTamano").value))) : construirEntrada(modo);
	const texto = JSON.stringify({ mode: modo, input: entrada });
	document.getElementById("labPeticion").textContent = texto.length <= MAXIMO_CARACTERES_VISIBLES ? texto : `${texto.slice(0, MAXIMO_CARACTERES_VISIBLES)}… (${texto.length.toLocaleString()} caracteres en total)`;
	return entrada;
}

/*
 * Cambia de problema: actualiza la explicacion y el tamano sugerido.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function cambiarProblema() {
	const problema = PROBLEMAS[document.getElementById("labModo").value];
	document.getElementById("labTamano").value = problema.tamano;
	document.getElementById("labDescripcion").textContent = problema.descripcion;
	document.querySelector("#labTabla tbody").innerHTML = `<tr><td colspan="4" class="ayuda">Presiona “Comparar”.</td></tr>`;
	prepararInstancia();
}

/*
 * Corre todas las tecnicas del problema sobre los mismos datos y muestra
 * tiempo, resultado y que significa cada renglon.
 * Parametros: ninguno.
 * Retorno: promesa que termina al mostrar los resultados.
 */
async function compararTecnicas() {
	const modo = document.getElementById("labModo").value;
	const nombres = PROBLEMAS[modo].tecnicas;
	const entrada = prepararInstancia();
	const cuerpoTabla = document.querySelector("#labTabla tbody");
	const boton = document.getElementById("labComparar");
	boton.disabled = true;
	cuerpoTabla.innerHTML = `<tr><td colspan="4">Calculando en el núcleo C++…</td></tr>`;
	const resultados = [];
	for (const tecnica of Object.keys(nombres)) {
		const resultado = await llamarNucleo(modo, { ...entrada, technique: tecnica }).catch((error) => ({ error: error.message }));
		resultados.push({ tecnica, ...resultado });
	}
	const referencia = resultados.find((r) => !r.error);
	cuerpoTabla.innerHTML = resultados.map((r, i) => `<tr><td><strong>${nombres[r.tecnica]}</strong></td><td>${r.error ? "-" : `${r.segundos.toFixed(3)} s`}</td><td>${r.error ? "-" : resumirRespuesta(modo, r.respuesta)}</td><td>${explicarRenglon(modo, r, referencia, i === 0, referencia ? nombres[referencia.tecnica] : "")}</td></tr>`).join("");
	const ultima = resultados[resultados.length - 1];
	const respuesta = ultima.error ? ultima.error : JSON.stringify(ultima.respuesta);
	document.getElementById("labRespuesta").textContent = respuesta.length > MAXIMO_CARACTERES_VISIBLES ? respuesta.slice(0, MAXIMO_CARACTERES_VISIBLES) + "…" : respuesta;
	boton.disabled = false;
}

/*
 * Conecta los controles de la vista.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function iniciarLaboratorio() {
	const selectorModo = document.getElementById("labModo");
	selectorModo.innerHTML = Object.entries(PROBLEMAS).map(([modo, p]) => `<option value="${modo}">${p.nombre}</option>`).join("");
	selectorModo.addEventListener("change", cambiarProblema);
	document.getElementById("labFuente").addEventListener("change", prepararInstancia);
	document.getElementById("labTamano").addEventListener("change", prepararInstancia);
	document.getElementById("labComparar").addEventListener("click", compararTecnicas);
	cambiarProblema();
}
