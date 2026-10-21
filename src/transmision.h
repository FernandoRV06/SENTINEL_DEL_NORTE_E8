/*
 * Descripcion: utilerias compartidas por los tres desafios de ciberseguridad.
 * Normaliza las transmisiones: elimina saltos de linea y espacios para que
 * las posiciones (en base 1) se cuenten sobre la secuencia limpia.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef TRANSMISION_H
#define TRANSMISION_H

#include <string>

/*
 * Elimina saltos de linea (incluido el \r de Windows), espacios y
 * tabuladores de una transmision.
 * Parametros:
 *   textoOriginal: transmision tal como se leyo del archivo.
 * Retorno: la secuencia hexadecimal sin caracteres de separacion.
 */
inline std::string normalizarTransmision(const std::string &textoOriginal) {
	std::string limpio = "";
	for (char caracter : textoOriginal) {
		bool esSeparador = caracter == '\n' || caracter == '\r' || caracter == ' ' || caracter == '\t';
		if (!esSeparador) {
			limpio += caracter;
		}
	}
	return limpio;
}

#endif
