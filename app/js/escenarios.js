/*
 * Descripcion: vista Scenario Manager. Lista los escenarios, permite
 * editar el activo (origen y destino del flujo, enlaces caidos), cargar
 * un escenario desde un archivo JSON, descargar el actual y recalcular
 * todos los resultados con el nucleo en C++.
 * Formato de un escenario: {"nombre", "descripcion", "flujo": {"origen",
 * "destino"}, "enlacesCaidos": [["AT", "DE"]], "transmisiones": [t1, t2],
 * "origenTransmisiones", "firmas": [f1, f2, f3], "asociacion": [...]} y,
 * opcional, "red": {"nodos", "enlaces"} para usar otra red.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

let redBase = null;
let indiceEscenarioActivo = 0;

/*
 * Dibuja las tarjetas de los escenarios disponibles.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function listarEscenarios() {
	const lista = document.getElementById("listaEscenarios");
	lista.innerHTML = estado.escenarios.map((escenario, i) => `<button class="tarjeta-escenario${i === indiceEscenarioActivo ? " activa" : ""}" data-escenario="${i}"><strong>${escenario.nombre}</strong>${escenario.descripcion || ""}</button>`).join("");
	lista.querySelectorAll("[data-escenario]").forEach((tarjeta) => {
		tarjeta.addEventListener("click", () => activarEscenario(Number(tarjeta.dataset.escenario)));
	});
}

/*
 * Llena el editor con los datos del escenario activo.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function llenarEditor() {
	const opciones = estado.red.nodos.map((nodo) => `<option value="${nodo.codigo}">${nodo.codigo} · ${nodo.nombre}</option>`).join("");
	const origen = document.getElementById("escOrigen");
	const destino = document.getElementById("escDestino");
	origen.innerHTML = opciones;
	destino.innerHTML = opciones;
	origen.value = estado.escenario.flujo.origen;
	destino.value = estado.escenario.flujo.destino;
	document.getElementById("escEnlaces").innerHTML = estado.red.enlaces.map((enlace, i) => `<label><input type="checkbox" data-enlace="${i}"${enlaceCaido(enlace) ? "" : " checked"}> ${claveEnlace(enlace.u, enlace.v)} <span class="ayuda">${enlace.distanciaKm} km · ${enlace.capacidadGbps} Gbps</span></label>`).join("");
}

/*
 * Toma los cambios del editor y los guarda en el escenario activo.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function leerEditor() {
	estado.escenario.flujo = { origen: document.getElementById("escOrigen").value, destino: document.getElementById("escDestino").value };
	estado.escenario.enlacesCaidos = [...document.querySelectorAll("[data-enlace]")]
		.filter((casilla) => !casilla.checked)
		.map((casilla) => {
			const enlace = estado.red.enlaces[Number(casilla.dataset.enlace)];
			return [estado.red.nodos[enlace.u].codigo, estado.red.nodos[enlace.v].codigo];
		});
}

/*
 * Recalcula con el nucleo los cuatro algoritmos de infraestructura y
 * actualiza todas las vistas.
 * Parametros: ninguno.
 * Retorno: promesa que termina al actualizar las vistas.
 */
async function recalcular() {
	const aviso = document.getElementById("escEstado");
	const boton = document.getElementById("escRecalcular");
	leerEditor();
	boton.disabled = true;
	aviso.textContent = "Calculando…";
	const inicio = performance.now();
	await calcularInfraestructura();
	actualizarExplorador();
	prepararInstancia();
	const errores = Object.entries(estado.resultados).filter(([, r]) => r.error).map(([modo, r]) => `${modo}: ${r.error}`);
	aviso.textContent = errores.length ? `Atención: ${errores.join("; ")}` : `Listo. Todo se recalculó en ${((performance.now() - inicio) / 1000).toFixed(2)} s; revisa el mapa en Network Explorer.`;
	boton.disabled = false;
}

/*
 * Activa un escenario: copia sus datos, actualiza las vistas y recalcula.
 * Parametros:
 *   indice: posicion del escenario en la lista.
 * Retorno: promesa que termina al recalcular.
 */
async function activarEscenario(indice) {
	indiceEscenarioActivo = indice;
	estado.escenario = JSON.parse(JSON.stringify(estado.escenarios[indice]));
	estado.red = estado.escenario.red || redBase;
	document.getElementById("nombreEscenario").textContent = estado.escenario.nombre;
	listarEscenarios();
	llenarEditor();
	cargarConsola();
	await recalcular();
}

/*
 * Revisa que un escenario cargado tenga los campos necesarios.
 * Parametros:
 *   escenario: objeto leido del archivo.
 * Retorno: texto con el problema encontrado, o "" si es valido.
 */
function validarEscenario(escenario) {
	let problema = "";
	if (!escenario || typeof escenario.nombre !== "string") {
		problema = "falta \"nombre\"";
	} else if (!escenario.flujo || !escenario.flujo.origen || !escenario.flujo.destino) {
		problema = "falta \"flujo\" con \"origen\" y \"destino\"";
	} else if (!Array.isArray(escenario.transmisiones) || escenario.transmisiones.length !== 2) {
		problema = "\"transmisiones\" debe tener dos cadenas";
	} else if (!Array.isArray(escenario.firmas)) {
		problema = "falta \"firmas\"";
	}
	return problema;
}

/*
 * Carga un escenario desde un archivo JSON elegido por el usuario.
 * Parametros:
 *   evento: cambio del selector de archivo.
 * Retorno: promesa que termina al activar el escenario.
 */
async function cargarEscenarioArchivo(evento) {
	const archivo = evento.target.files[0];
	const aviso = document.getElementById("escEstado");
	if (archivo) {
		let escenario = null;
		try {
			escenario = JSON.parse(await archivo.text());
		} catch (error) {
			escenario = null;
		}
		const problema = validarEscenario(escenario);
		if (problema) {
			aviso.textContent = `No se pudo cargar ${archivo.name}: ${problema}.`;
		} else {
			escenario.enlacesCaidos = escenario.enlacesCaidos || [];
			escenario.asociacion = escenario.asociacion || [];
			estado.escenarios.push(escenario);
			await activarEscenario(estado.escenarios.length - 1);
		}
		evento.target.value = "";
	}
}

/*
 * Descarga el escenario activo (con su red) como archivo JSON.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function descargarEscenario() {
	leerEditor();
	const datos = { ...estado.escenario, red: estado.red };
	const enlace = document.createElement("a");
	enlace.href = URL.createObjectURL(new Blob([JSON.stringify(datos, null, 1)], { type: "application/json" }));
	enlace.download = "escenario.json";
	enlace.click();
	URL.revokeObjectURL(enlace.href);
}

/*
 * Conecta los controles de la vista.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function iniciarEscenarios() {
	document.getElementById("escRecalcular").addEventListener("click", recalcular);
	document.getElementById("escArchivo").addEventListener("change", cargarEscenarioArchivo);
	document.getElementById("escDescargar").addEventListener("click", descargarEscenario);
}
