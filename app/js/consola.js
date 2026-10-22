/*
 * Descripcion: vista Cybersecurity Console. Envia las transmisiones y las
 * firmas al nucleo en C++ (modos matching, palindrome y common), muestra
 * los resultados en palabras, resalta sobre cada transmision lo que
 * encontro y escribe la salida con el formato de texto del caso.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 21/10/2026
 */

const MAXIMO_CARACTERES_RESALTADOS = 20000;

/*
 * Quita saltos de linea y espacios, igual que transmision.h en el nucleo,
 * para que las posiciones resaltadas coincidan con las del C++.
 * Parametros:
 *   texto: transmision tal como se escribio o se cargo.
 * Retorno: la secuencia limpia.
 */
function normalizarTexto(texto) {
	return texto.replace(/\s+/g, "");
}

/*
 * Escapa un texto para insertarlo en HTML.
 * Parametros:
 *   texto: cualquier texto.
 * Retorno: el texto seguro para HTML.
 */
function escaparHtml(texto) {
	const reemplazos = { "&": "&amp;", "<": "&lt;", ">": "&gt;", "\"": "&quot;" };
	return [...texto].map((caracter) => reemplazos[caracter] || caracter).join("");
}

/*
 * Llena la consola con las transmisiones, firmas y asociacion del escenario.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function cargarConsola() {
	const escenario = estado.escenario;
	document.getElementById("conT1").value = escenario.transmisiones[0];
	document.getElementById("conT2").value = escenario.transmisiones[1];
	["conF1", "conF2", "conF3"].forEach((id, i) => {
		document.getElementById(id).value = escenario.firmas[i] || "";
	});
	const asociacion = escenario.asociacion.map((fila, i) => `la transmisión ${i + 1} viaja por ${fila.enlace.join("–")} (${fila.escenario.charAt(0).toLowerCase()}${fila.escenario.slice(1)})`).join(" y ");
	const aviso = asociacion ? `En el caso, ${asociacion}. Esa asociación es simulada, así que encontrar una firma indica algo sospechoso, no confirma un ataque. ` : "";
	document.getElementById("conAviso").textContent = `${aviso}Origen de estas transmisiones: ${escenario.origenTransmisiones}.`;
	document.getElementById("conResultados").innerHTML = `<p class="ayuda">Presiona “Analizar”.</p>`;
	document.getElementById("conSalida").textContent = "";
}

/*
 * Marca un intervalo de posiciones (base 1, inclusivo) con una clase.
 * Parametros:
 *   marcas: arreglo de conjuntos de clases, uno por caracter.
 *   inicio, fin: posiciones en base 1.
 *   clase: clase CSS que se agrega.
 * Retorno: ninguno.
 */
function marcarIntervalo(marcas, inicio, fin, clase) {
	for (let i = inicio - 1; i < fin && i < marcas.length; i++) {
		marcas[i].add(clase);
	}
}

/*
 * Convierte una transmision y sus marcas en HTML, agrupando caracteres
 * seguidos que tienen las mismas marcas.
 * Parametros:
 *   texto: transmision normalizada.
 *   marcas: conjunto de clases de cada caracter.
 * Retorno: HTML de la transmision resaltada.
 */
function pintarTransmision(texto, marcas) {
	const limite = Math.min(texto.length, MAXIMO_CARACTERES_RESALTADOS);
	let html = "";
	let inicio = 0;
	while (inicio < limite) {
		const clases = [...marcas[inicio]].sort().join(" ");
		let fin = inicio + 1;
		while (fin < limite && [...marcas[fin]].sort().join(" ") === clases) {
			fin++;
		}
		const fragmento = escaparHtml(texto.slice(inicio, fin));
		html += clases ? `<span class="${clases}">${fragmento}</span>` : fragmento;
		inicio = fin;
	}
	if (texto.length > limite) {
		html += `<span class="ayuda"> … (se muestran los primeros ${limite.toLocaleString()} de ${texto.length.toLocaleString()} caracteres)</span>`;
	}
	return html;
}

/*
 * Envia las transmisiones al nucleo y muestra los tres analisis.
 * Parametros: ninguno.
 * Retorno: promesa que termina al mostrar los resultados.
 */
async function analizarTransmisiones() {
	const boton = document.getElementById("conAnalizar");
	const transmisiones = [document.getElementById("conT1").value, document.getElementById("conT2").value];
	const firmas = ["conF1", "conF2", "conF3"].map((id) => document.getElementById(id).value.trim()).filter((firma) => firma.length > 0);
	estado.escenario.transmisiones = transmisiones;
	estado.escenario.firmas = firmas;
	boton.disabled = true;
	const respuestas = await Promise.all([
		llamarNucleo("matching", construirEntrada("matching")),
		llamarNucleo("palindrome", construirEntrada("palindrome")),
		llamarNucleo("common", construirEntrada("common"))
	]).catch((error) => {
		document.getElementById("conResultados").innerHTML = `<p class="no">${escaparHtml(error.message)}</p>`;
		return null;
	});
	boton.disabled = false;
	if (respuestas !== null) {
		mostrarAnalisis(transmisiones.map(normalizarTexto), firmas, respuestas.map((r) => r.respuesta));
	}
}

/*
 * Muestra los resultados en palabras, las transmisiones resaltadas y la
 * salida en el formato de texto del caso.
 * Parametros:
 *   textos: transmisiones normalizadas.
 *   firmas: firmas buscadas.
 *   respuestas: respuestas del nucleo [matching, palindrome, common].
 * Retorno: ninguno.
 */
function mostrarAnalisis(textos, firmas, respuestas) {
	const [firmasResultado, palindromo, comun] = respuestas;
	const posiciones = firmasResultado.matches;
	const rangos = palindromo.ranges;
	const [comunInicio, comunFin] = comun.range;
	const subcadena = comunFin > 0 ? textos[0].slice(comunInicio - 1, comunFin) : "";
	const marcas = textos.map((texto) => Array.from({ length: texto.length }, () => new Set()));
	textos.forEach((texto, t) => {
		firmas.forEach((firma, f) => {
			if (posiciones[t][f] > 0) {
				marcarIntervalo(marcas[t], posiciones[t][f], posiciones[t][f] + firma.length - 1, "marca-firma");
			}
		});
		if (rangos[t][1] > 0) {
			marcarIntervalo(marcas[t], rangos[t][0], rangos[t][1], "marca-palindromo");
		}
	});
	if (subcadena.length > 0) {
		marcarIntervalo(marcas[0], comunInicio, comunFin, "marca-comun");
		const enSegunda = textos[1].indexOf(subcadena) + 1;
		marcarIntervalo(marcas[1], enSegunda, enSegunda + subcadena.length - 1, "marca-comun");
	}
	const celda = (pos) => (pos > 0 ? `<span class="si">Sí</span>, en la posición ${pos}` : `<span class="no">No aparece</span>`);
	const filasFirmas = firmas.map((firma, f) => `<tr><td><code>${escaparHtml(firma)}</code></td><td>${celda(posiciones[0][f])}</td><td>${celda(posiciones[1][f])}</td></tr>`).join("");
	const palindromoTexto = (t) => (rangos[t][1] > 0 ? `posiciones ${rangos[t][0]} a ${rangos[t][1]}: <code>${escaparHtml(textos[t].slice(rangos[t][0] - 1, rangos[t][1]))}</code>` : "ninguno");
	const recorte = (texto) => (texto.length > 60 ? `${texto.slice(0, 60)}…` : texto);
	document.getElementById("conResultados").innerHTML = `
		<h2>1 · ¿Aparecen las firmas de malware?</h2>
		<table class="tabla"><thead><tr><th>Firma</th><th>Transmisión 1</th><th>Transmisión 2</th></tr></thead><tbody>${filasFirmas}</tbody></table>
		<h2>2 · Palíndromo más largo <span class="ayuda">(se lee igual al derecho y al revés)</span></h2>
		<p>Transmisión 1: ${palindromoTexto(0)}<br>Transmisión 2: ${palindromoTexto(1)}</p>
		<h2>3 · Fragmento más largo que comparten las dos</h2>
		<p>${subcadena ? `Posiciones ${comunInicio} a ${comunFin} de la transmisión 1 (${subcadena.length} caracteres): <code>${escaparHtml(recorte(subcadena))}</code>` : "No comparten ningún fragmento."}</p>
		<h2>Las transmisiones resaltadas</h2>
		<div class="leyenda-consola"><span class="marca-firma">Firma</span><span class="marca-palindromo">Palíndromo</span><span class="marca-comun">Fragmento común</span></div>
		<p class="ayuda">Transmisión 1</p><div class="transmision">${pintarTransmision(textos[0], marcas[0])}</div>
		<p class="ayuda">Transmisión 2</p><div class="transmision">${pintarTransmision(textos[1], marcas[1])}</div>`;
	const lineas = [];
	posiciones.forEach((fila) => fila.forEach((pos) => lineas.push(pos > 0 ? `true ${pos}` : "false")));
	rangos.forEach((rango) => lineas.push(rango.join(" ")));
	lineas.push(`${comunInicio} ${comunFin}`);
	document.getElementById("conSalida").textContent = lineas.join("\n");
}

/*
 * Conecta los controles de la consola.
 * Parametros: ninguno.
 * Retorno: ninguno.
 */
function iniciarConsola() {
	document.getElementById("conAnalizar").addEventListener("click", analizarTransmisiones);
	document.querySelectorAll("input.archivo[data-destino]").forEach((selector) => {
		selector.addEventListener("change", async () => {
			const archivo = selector.files[0];
			if (archivo) {
				document.getElementById(selector.dataset.destino).value = await archivo.text();
			}
		});
	});
}
