/*
 * Descripcion: desafio 05 (Malware Signature Detection): busqueda de patrones.
 * Busca tres firmas en dos transmisiones y reporta la primera posicion
 * de cada coincidencia, contando desde la posicion uno.
 */

#ifndef FIRMAS_H
#define FIRMAS_H

#include "json.hpp"

using Json = nlohmann::json;

#include "transmision.h"

/*
 * Atiende el modo de deteccion de firmas. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverFirmas(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
