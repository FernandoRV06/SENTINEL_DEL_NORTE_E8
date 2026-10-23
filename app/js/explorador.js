/*
 * Descripcion: vista Network Explorer. Dibuja la red sobre un mapa real
 * con la libreria Leaflet (mapa de OpenStreetMap) y, encima, uno a la vez,
 * los resultados del nucleo en C++: MST, ruta TSP, flujo maximo por enlace
 * o regiones de Voronoi. Con Voronoi, un clic en el mapa ubica a un
 * cliente nuevo en la region (calculada por el C++) donde cae.
 * Equipo: Sentinela del Norte (Equipo 8)
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

const COLORES = { red: "#5d6577", mst: "#138a4f", tsp: "#d9730d", flujo: "#1f5fd6", caido: "#b3261e" };

let mapa = null;
let capaVisible = "red";
let textoConsulta = "";
const capas = {};
let capaConsulta = null;

/*
 * Crea el mapa, su fondo de OpenStreetMap y una capa vacia por resultado.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function iniciarMapa() {
	mapa = L.map("mapa", { zoomSnap: 0.5 });
	mapa.fitBounds([[LIMITES_VORONOI[1], LIMITES_VORONOI[0]], [LIMITES_VORONOI[3], LIMITES_VORONOI[2]]]);
	L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
		maxZoom: 9,
		attribution: "&copy; colaboradores de OpenStreetMap"
	}).addTo(mapa);
	for (const nombre of ["voronoi", "mst", "flujo", "tsp", "red"]) {
		capas[nombre] = L.layerGroup();
	}
	capas.red.addTo(mapa);
	capaConsulta = L.layerGroup().addTo(mapa);
	document.querySelectorAll("input[name=capa]").forEach((opcion) => {
		opcion.addEventListener("change", () => mostrarCapa(opcion.value));
	});
	mapa.on("click", consultarCobertura);
}

/*
 * Deja visible solo la capa elegida (la red siempre se ve debajo).
 * Parametros:
 *   nombre: "red", "mst", "tsp", "flujo" o "voronoi".
 * Retorno: ninguno.
 */
function mostrarCapa(nombre) {
	capaVisible = nombre;
	textoConsulta = "";
	capaConsulta.clearLayers();
	for (const otra of ["mst", "tsp", "flujo", "voronoi"]) {
		if (otra === nombre) {
			capas[otra].addTo(mapa);
		} else {
			capas[otra].remove();
		}
	}
	capas.red.remove();
	capas.red.addTo(mapa);
	escribirExplicacion();
}

/*
 * Convierte el indice de una central en su posicion [lat, lon].
 * Parametros:
 *   indice: indice de la central.
 * Retorno: arreglo [lat, lon] para Leaflet.
 */
function posicion(indice) {
	const nodo = estado.red.nodos[indice];
	return [nodo.lat, nodo.lon];
}

/*
 * Dibuja centrales y enlaces. Los enlaces caidos del escenario se
 * muestran punteados en rojo.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function dibujarRed() {
	capas.red.clearLayers();
	for (const enlace of estado.red.enlaces) {
		const caido = enlaceCaido(enlace);
		L.polyline([posicion(enlace.u), posicion(enlace.v)], {
			color: caido ? COLORES.caido : COLORES.red,
			weight: 2,
			opacity: caido ? 0.9 : 0.5,
			dashArray: caido ? "6 6" : null
		}).bindTooltip(`${claveEnlace(enlace.u, enlace.v)}: ${enlace.distanciaKm} km, ${enlace.capacidadGbps} Gbps${caido ? " (caído)" : ""}`).addTo(capas.red);
	}
	estado.red.nodos.forEach((nodo) => {
		L.circleMarker([nodo.lat, nodo.lon], { radius: 6, color: "#ffffff", weight: 2, fillColor: "#0f1b33", fillOpacity: 1 })
			.bindTooltip(nodo.codigo, { permanent: true, direction: "top", className: "etiqueta-mapa", offset: [0, -6] })
			.addTo(capas.red);
	});
}

/*
 * Dibuja las aristas que eligio el MST.
 * Parametros:
 *   respuesta: {"cost", "edges": [[u, v, peso], ...]} del nucleo.
 * Retorno: ninguno.
 */
function dibujarMst(respuesta) {
	for (const [u, v, peso] of respuesta.edges) {
		L.polyline([posicion(u), posicion(v)], { color: COLORES.mst, weight: 6, opacity: 0.85 })
			.bindTooltip(`${claveEnlace(u, v)}: costo ${peso}`).addTo(capas.mst);
	}
}

/*
 * Dibuja la ruta del TSP siguiendo los caminos reales entre paradas, con
 * el numero de orden de cada parada.
 * Parametros:
 *   respuesta: {"route": [0, ..., 0], "cost"} del nucleo.
 * Retorno: ninguno.
 */
function dibujarTsp(respuesta) {
	const puntos = [];
	for (let i = 0; i + 1 < respuesta.route.length; i++) {
		caminoEntre(respuesta.route[i], respuesta.route[i + 1]).forEach((indice) => puntos.push(posicion(indice)));
	}
	L.polyline(puntos, { color: COLORES.tsp, weight: 4 }).addTo(capas.tsp);
	respuesta.route.slice(0, -1).forEach((indice, orden) => {
		L.marker(posicion(indice), { icon: L.divIcon({ className: "numero-tsp", html: `${orden + 1}`, iconSize: [20, 16], iconAnchor: [-8, -6] }) }).addTo(capas.tsp);
	});
}

/*
 * Dibuja el flujo de cada enlace con grosor proporcional a lo que lleva.
 * Las aristas se enviaron en pares (u -> v, v -> u) por enlace activo, asi
 * que el flujo neto del enlace es la diferencia del par.
 * Parametros:
 *   respuesta: {"flow", "edgeFlows"} del nucleo.
 * Retorno: ninguno.
 */
function dibujarFlujo(respuesta) {
	enlacesActivos().forEach((enlace, k) => {
		const neto = respuesta.edgeFlows[2 * k] - respuesta.edgeFlows[2 * k + 1];
		if (Math.abs(neto) > 0) {
			const desde = neto > 0 ? enlace.u : enlace.v;
			const hacia = neto > 0 ? enlace.v : enlace.u;
			L.polyline([posicion(desde), posicion(hacia)], { color: COLORES.flujo, weight: 3 + 9 * Math.abs(neto) / 100, opacity: 0.8 })
				.bindTooltip(`${estado.red.nodos[desde].codigo} → ${estado.red.nodos[hacia].codigo}: lleva ${Math.abs(neto)} de ${enlace.capacidadGbps} Gbps`).addTo(capas.flujo);
		}
	});
	for (const codigo of [estado.escenario.flujo.origen, estado.escenario.flujo.destino]) {
		L.circleMarker(posicion(indicePorCodigo(codigo)), { radius: 11, color: COLORES.flujo, weight: 3, fill: false }).addTo(capas.flujo);
	}
}

/*
 * Dibuja las regiones de Voronoi. El nucleo trabaja en el plano
 * (x = longitud, y = latitud); Leaflet espera [lat, lon].
 * Parametros:
 *   respuesta: {"cells": [poligono por central]} del nucleo.
 * Retorno: ninguno.
 */
function dibujarVoronoi(respuesta) {
	respuesta.cells.forEach((celda, indice) => {
		const tono = (indice * 137) % 360;
		L.polygon(celda.map(([x, y]) => [y, x]), {
			color: `hsl(${tono}, 55%, 40%)`,
			weight: 1,
			fillColor: `hsl(${tono}, 70%, 60%)`,
			fillOpacity: 0.3,
			interactive: false
		}).addTo(capas.voronoi);
	});
}

/*
 * Indica si un punto esta dentro de un poligono (conteo de cruces).
 * Parametros:
 *   x, y: punto en el plano.
 *   poligono: arreglo de vertices [x, y].
 * Retorno: true si el punto esta dentro.
 */
function puntoEnPoligono(x, y, poligono) {
	let dentro = false;
	for (let i = 0, j = poligono.length - 1; i < poligono.length; j = i++) {
		const [xi, yi] = poligono[i];
		const [xj, yj] = poligono[j];
		const cruza = (yi > y) !== (yj > y) && x < (xj - xi) * (y - yi) / (yj - yi) + xi;
		if (cruza) {
			dentro = !dentro;
		}
	}
	return dentro;
}

/*
 * Con Voronoi visible, ubica a un cliente nuevo: busca la region (resultado
 * del nucleo) que contiene el punto donde se hizo clic.
 * Parametros:
 *   evento: clic de Leaflet con la posicion.
 * Retorno: ninguno.
 */
function consultarCobertura(evento) {
	const resultado = estado.resultados.voronoi;
	if (capaVisible === "voronoi" && resultado && resultado.respuesta) {
		const { lat, lng } = evento.latlng;
		const indice = resultado.respuesta.cells.findIndex((celda) => puntoEnPoligono(lng, lat, celda));
		capaConsulta.clearLayers();
		if (indice < 0) {
			textoConsulta = "Ese punto queda fuera del área de cobertura.";
		} else {
			const nodo = estado.red.nodos[indice];
			L.circleMarker([lat, lng], { radius: 7, color: "#000", fillColor: "#ffd166", fillOpacity: 1 }).addTo(capaConsulta);
			L.polyline([[lat, lng], posicion(indice)], { color: "#000", weight: 1.5, dashArray: "4 4" }).addTo(capaConsulta);
			textoConsulta = `El cliente se conecta a <strong>${nodo.nombre} (${nodo.codigo})</strong>.`;
		}
		escribirExplicacion();
	}
}

/*
 * Escribe en el panel que muestra la capa elegida y su resultado.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function escribirExplicacion() {
	const r = estado.resultados;
	const nodos = estado.red.nodos;
	const flujo = estado.escenario.flujo;
	const nombreDe = (codigo) => nodos[indicePorCodigo(codigo)].nombre;
	const textos = {
		red: ["La red", `${nodos.length} centrales unidas por ${enlacesActivos().length} enlaces de fibra. Pasa el mouse sobre una línea para ver su distancia y capacidad.`, () => `${nodos.length} centrales`],
		mst: ["Despliegue al menor costo", "Los enlaces verdes conectan todas las centrales gastando lo menos posible y sin formar ciclos. Algoritmo: Kruskal.", () => `Costo total: ${r.mst.respuesta.cost.toLocaleString()}`],
		tsp: ["Ruta del técnico", `El recorrido más corto para visitar cada central una vez y regresar a ${nodos[0].nombre}. Los números son el orden de visita. Algoritmo: Held-Karp.`, () => `${r.tsp.respuesta.cost.toLocaleString()} km`],
		flujo: ["Flujo máximo", `Lo máximo que puede viajar de ${nombreDe(flujo.origen)} a ${nombreDe(flujo.destino)} sin rebasar la capacidad de ningún enlace. Entre más gruesa la línea, más tráfico lleva. Algoritmo: Edmonds-Karp.`, () => `${r.flow.respuesta.flow} Gbps`],
		voronoi: ["Zona de cada central", "Cada color es la zona que atiende una central: los puntos más cercanos a ella. Haz clic en el mapa para ubicar a un cliente nuevo. Algoritmo: recorte por semiplanos.", () => textoConsulta || "Haz clic en el mapa."]
	};
	const modos = { mst: "mst", tsp: "tsp", flujo: "flow", voronoi: "voronoi" };
	const [titulo, descripcion, resultado] = textos[capaVisible];
	const error = modos[capaVisible] && r[modos[capaVisible]] ? r[modos[capaVisible]].error : null;
	document.getElementById("explicacion").innerHTML = `<h2>${titulo}</h2><p>${descripcion}</p><p class="dato">${error ? `<span class="no">No se pudo calcular: ${error}</span>` : resultado()}</p>`;
}

/*
 * Redibuja la red y los resultados del escenario activo.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function actualizarExplorador() {
	const r = estado.resultados;
	dibujarRed();
	for (const nombre of ["mst", "tsp", "flujo", "voronoi"]) {
		capas[nombre].clearLayers();
	}
	if (!r.mst.error) {
		dibujarMst(r.mst.respuesta);
	}
	if (!r.tsp.error) {
		dibujarTsp(r.tsp.respuesta);
	}
	if (!r.flow.error) {
		dibujarFlujo(r.flow.respuesta);
	}
	if (!r.voronoi.error) {
		dibujarVoronoi(r.voronoi.respuesta);
	}
	mostrarCapa(capaVisible);
}
