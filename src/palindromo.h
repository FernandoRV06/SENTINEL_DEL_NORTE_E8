/*
 * Descripcion: desafio 06 · Mirrored Code Analysis (palindromo mas largo).
 * Encuentra en cada transmision el palindromo continuo mas largo, de
 * longitud par o impar, sobre caracteres hexadecimales.
 * Usa Manacher: O(n) tiempo y memoria por transmision.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef PALINDROMO_H
#define PALINDROMO_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "json.h"
#include "transmision.h"

/*
 * Rango de un palindromo en posiciones base 1 (inicio y fin incluidos).
 */
struct RangoPalindromo {
	std::size_t inicio = 0;
	std::size_t fin = 0;
};

/*
 * Calcula los radios de Manacher. impar[i] es cuantos caracteres mide la
 * mitad (con el centro) del palindromo impar mas largo centrado en i;
 * par[i] es la mitad del palindromo par mas largo cuyo centro derecho es i.
 * Reutiliza el palindromo que llega mas a la derecha para no comparar de
 * nuevo, asi que el total es O(n).
 * Parametros:
 *   texto: cadena a analizar.
 *   esPar: false para los radios impares, true para los pares.
 * Retorno: el radio de cada posicion.
 */
inline std::vector<std::size_t> calcularRadiosManacher(const std::string &texto, bool esPar) {
	std::size_t cantidad = texto.size();
	// Con radio k, el impar cubre [i - k + 1, i + k - 1] y el par [i - k, i + k - 1];
	// "ajuste" es esa diferencia de una posicion en el extremo izquierdo.
	std::size_t ajuste = esPar ? 1 : 0;
	std::vector<std::size_t> radio(cantidad, 0);
	// [izquierda, derecha) es el palindromo encontrado que llega mas a la derecha.
	std::size_t izquierda = 0;
	std::size_t derecha = 0;
	for (std::size_t centro = 0; centro < cantidad; centro++) {
		std::size_t actual = 1 - ajuste;
		if (centro < derecha) {
			std::size_t espejo = izquierda + derecha - centro - 1 + ajuste;
			actual = std::min(radio[espejo], derecha - centro);
		}
		bool crece = true;
		while (crece) {
			bool dentro = centro + actual < cantidad && centro >= actual + ajuste;
			crece = dentro && texto[centro + actual] == texto[centro - actual - ajuste];
			if (crece) {
				actual++;
			}
		}
		radio[centro] = actual;
		if (centro + actual > derecha) {
			izquierda = centro + 1 - ajuste - actual;
			derecha = centro + actual;
		}
	}
	return radio;
}

/*
 * Encuentra el palindromo continuo mas largo; con empate gana el que
 * empieza antes.
 * Parametros:
 *   texto: transmision normalizada.
 * Retorno: su rango en base 1, o [0, 0] si el texto esta vacio.
 */
inline RangoPalindromo buscarPalindromoMasLargo(const std::string &texto) {
	RangoPalindromo mejor;
	std::size_t mejorLongitud = 0;
	std::vector<std::size_t> impar = calcularRadiosManacher(texto, false);
	std::vector<std::size_t> par = calcularRadiosManacher(texto, true);
	for (std::size_t centro = 0; centro < texto.size(); centro++) {
		std::size_t longitudes[2] = {2 * impar[centro] - 1, 2 * par[centro]};
		std::size_t inicios[2] = {centro + 1 - impar[centro], centro - par[centro]};
		for (std::size_t opcion = 0; opcion < 2; opcion++) {
			bool esMejor = longitudes[opcion] > mejorLongitud || (longitudes[opcion] == mejorLongitud && inicios[opcion] + 1 < mejor.inicio);
			if (longitudes[opcion] > 0 && esMejor) {
				mejorLongitud = longitudes[opcion];
				mejor.inicio = inicios[opcion] + 1;
				mejor.fin = inicios[opcion] + longitudes[opcion];
			}
		}
	}
	return mejor;
}

/*
 * Atiende el modo de palindromo mas largo.
 * Parametros:
 *   entrada: campo "input" con "transmissions".
 * Retorno: objeto JSON {"ranges": [[inicio, fin] por transmision]} en base 1.
 */
inline ValorJson resolverPalindromo(const ValorJson &entrada) {
	ValorJson rangos = crearArreglo();
	for (const std::string &transmision : leerTransmisiones(entrada.obtenerCampo("transmissions"))) {
		RangoPalindromo rango = buscarPalindromoMasLargo(transmision);
		ValorJson rangoJson = crearArreglo();
		rangoJson.agregarElemento(crearNumero(static_cast<double>(rango.inicio)));
		rangoJson.agregarElemento(crearNumero(static_cast<double>(rango.fin)));
		rangos.agregarElemento(rangoJson);
	}
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("ranges", rangos);
	return respuesta;
}

#endif
