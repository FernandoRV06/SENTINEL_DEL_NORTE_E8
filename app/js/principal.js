/*
 * Descripcion: punto de entrada de la pagina. Cambia entre las cuatro
 * vistas, revisa que el servidor (y por lo tanto el nucleo en C++) este
 * disponible, carga la red y los escenarios, y activa el primero.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

/*
 * Muestra una vista y oculta las demas.
 * Parametros:
 *   nombre: "explorador", "laboratorio", "consola" o "escenarios".
 * Retorno: ninguno.
 */
function cambiarVista(nombre) {
	document.querySelectorAll(".pestana").forEach((pestana) => pestana.classList.toggle("activa", pestana.dataset.vista === nombre));
	document.querySelectorAll(".vista").forEach((vista) => vista.classList.toggle("activa", vista.id === `vista-${nombre}`));
	if (nombre === "explorador") {
		mapa.invalidateSize();
	}
}

/*
 * Revisa que el servidor responda.
 * Parametros: ninguno.
 * Retorno: promesa con true si el servidor esta disponible.
 */
async function revisarServidor() {
	const indicador = document.getElementById("estadoServidor");
	const disponible = await fetch("/api/estado").then((r) => r.ok).catch(() => false);
	indicador.textContent = disponible ? "Núcleo C++ conectado" : "Sin servidor: ejecuta python3 app/servidor.py";
	indicador.classList.toggle("ok", disponible);
	indicador.classList.toggle("error", !disponible);
	return disponible;
}

/*
 * Inicia la aplicacion.
 * Parametros: ninguno.
 * Retorno: promesa que termina cuando el primer escenario esta calculado.
 */
async function iniciar() {
	document.querySelectorAll(".pestana").forEach((pestana) => {
		pestana.addEventListener("click", () => cambiarVista(pestana.dataset.vista));
	});
	iniciarMapa();
	if (await revisarServidor()) {
		redBase = await cargarJson("datos/red.json");
		estado.escenarios = await cargarJson("datos/escenarios.json");
		estado.red = redBase;
		estado.escenario = JSON.parse(JSON.stringify(estado.escenarios[0]));
		iniciarLaboratorio();
		iniciarConsola();
		iniciarEscenarios();
		await activarEscenario(0);
	}
}

iniciar();
