/*
 * Descripcion: desafio 05 · Malware Signature Detection (busqueda de patrones).
 * Busca tres firmas en dos transmisiones y reporta la primera posicion
 * de cada coincidencia, contando desde la posicion uno.
 * Usa KMP: O(n + m) por firma, sin retroceder en la transmision.
 * Alternativa para comparar: busqueda ingenua, O(n * m) en el peor caso.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef FIRMAS_H
#define FIRMAS_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.h"
#include "transmision.h"

/*
 * Calcula la funcion de fallo de KMP: fallo[i] es la longitud del prefijo
 * propio mas largo de patron[0..i] que tambien es sufijo. O(m).
 * Parametros:
 *   patron: cadena a buscar.
 * Retorno: la tabla de fallo.
 */
inline std::vector<std::size_t> calcularFalloKmp(const std::string &patron) {
	std::vector<std::size_t> fallo(patron.size(), 0);
	std::size_t longitud = 0;
	for (std::size_t indice = 1; indice < patron.size(); indice++) {
		while (longitud > 0 && patron[indice] != patron[longitud]) {
			longitud = fallo[longitud - 1];
		}
		if (patron[indice] == patron[longitud]) {
			longitud++;
		}
		fallo[indice] = longitud;
	}
	return fallo;
}

/*
 * Busca la primera aparicion de un patron con KMP. O(n + m).
 * Parametros:
 *   texto: transmision donde se busca.
 *   patron: firma a buscar.
 * Retorno: la posicion inicial en base 1, o -1 si no aparece
 *          (un patron vacio aparece en la posicion 1).
 */
inline long long buscarPrimeraAparicion(const std::string &texto, const std::string &patron) {
	long long posicion = -1;
	if (patron.empty()) {
		posicion = 1;
	} else {
		std::vector<std::size_t> fallo = calcularFalloKmp(patron);
		std::size_t coincidencia = 0;
		std::size_t indice = 0;
		while (indice < texto.size() && posicion < 0) {
			while (coincidencia > 0 && texto[indice] != patron[coincidencia]) {
				coincidencia = fallo[coincidencia - 1];
			}
			if (texto[indice] == patron[coincidencia]) {
				coincidencia++;
			}
			if (coincidencia == patron.size()) {
				posicion = static_cast<long long>(indice + 2 - patron.size());
			}
			indice++;
		}
	}
	return posicion;
}

/*
 * Busca la primera aparicion de un patron probando cada posicion inicial.
 * O(n * m) en el peor caso (por ejemplo "AAAA...AB" en "AAAA...A").
 * Parametros:
 *   texto: transmision donde se busca.
 *   patron: firma a buscar.
 * Retorno: la posicion inicial en base 1, o -1 si no aparece.
 */
inline long long buscarIngenuo(const std::string &texto, const std::string &patron) {
	long long posicion = -1;
	std::size_t inicio = 0;
	while (posicion < 0 && inicio + patron.size() <= texto.size()) {
		std::size_t comparados = 0;
		while (comparados < patron.size() && texto[inicio + comparados] == patron[comparados]) {
			comparados++;
		}
		if (comparados == patron.size()) {
			posicion = static_cast<long long>(inicio + 1);
		}
		inicio++;
	}
	return posicion;
}

/*
 * Atiende el modo de deteccion de firmas.
 * Parametros:
 *   entrada: campo "input" con "transmissions", "patterns" y, opcional,
 *            "technique": "kmp" (por defecto) o "naive".
 * Retorno: objeto JSON {"matches": [[pos, ...] por transmision]}, con una
 *          posicion por firma (base 1, o -1 si no aparece).
 */
inline ValorJson resolverFirmas(const ValorJson &entrada) {
	std::vector<std::string> transmisiones = leerTransmisiones(entrada.obtenerCampo("transmissions"));
	std::vector<std::string> firmas = leerTransmisiones(entrada.obtenerCampo("patterns"));
	std::string tecnica = leerTecnica(entrada, "kmp");
	if (tecnica != "kmp" && tecnica != "naive") {
		throw std::runtime_error("tecnica de matching desconocida: " + tecnica);
	}
	ValorJson coincidencias = crearArreglo();
	for (const std::string &transmision : transmisiones) {
		ValorJson fila = crearArreglo();
		for (const std::string &firma : firmas) {
			long long posicion = tecnica == "kmp" ? buscarPrimeraAparicion(transmision, firma) : buscarIngenuo(transmision, firma);
			fila.agregarElemento(crearNumero(static_cast<double>(posicion)));
		}
		coincidencias.agregarElemento(fila);
	}
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("matches", coincidencias);
	return respuesta;
}

#endif
