/*
 * Descripcion: desafio 07 · Transmission Similarity Analysis (subcadena comun mas larga).
 * Encuentra la subcadena continua mas larga presente en ambas
 * transmisiones y reporta sus posiciones en la primera.
 */

#ifndef SUBCADENA_COMUN_H
#define SUBCADENA_COMUN_H

#include "json.hpp"

using Json = nlohmann::json;

#include "transmision.h"

/*
 * Atiende el modo de subcadena comun mas larga. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverSubcadenaComun(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
