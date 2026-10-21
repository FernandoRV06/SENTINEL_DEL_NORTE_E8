/*
 * Descripcion: desafio 05 · Malware Signature Detection (busqueda de patrones).
 * Busca tres firmas en dos transmisiones y reporta la primera posicion
 * de cada coincidencia, contando desde la posicion uno.
 * Usa KMP: O(n + m) por firma, sin retroceder en la transmision.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef FIRMAS_H
#define FIRMAS_H

#include <cstddef>
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
 * Atiende el modo de deteccion de firmas.
 * Parametros:
 *   entrada: campo "input" con "transmissions" y "patterns".
 * Retorno: objeto JSON {"matches": [[pos, ...] por transmision]}, con una
 *          posicion por firma (base 1, o -1 si no aparece).
 */
inline ValorJson resolverFirmas(const ValorJson &entrada) {
	std::vector<std::string> transmisiones = leerTransmisiones(entrada.obtenerCampo("transmissions"));
	std::vector<std::string> firmas = leerTransmisiones(entrada.obtenerCampo("patterns"));
	ValorJson coincidencias = crearArreglo();
	for (const std::string &transmision : transmisiones) {
		ValorJson fila = crearArreglo();
		for (const std::string &firma : firmas) {
			fila.agregarElemento(crearNumero(static_cast<double>(buscarPrimeraAparicion(transmision, firma))));
		}
		coincidencias.agregarElemento(fila);
	}
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("matches", coincidencias);
	return respuesta;
}

#endif
