/*
 * Descripcion: desafio 03 · Maximum Information Flow (flujo maximo).
 * Calcula la maxima informacion transportable entre dos nodos usando las
 * capacidades de los enlaces (no las demandas).
 */

#ifndef FLUJO_MAXIMO_H
#define FLUJO_MAXIMO_H

#include "json.hpp"

using Json = nlohmann::json;

/*
 * Atiende el modo de flujo maximo. Version provisional: solo confirma que
 * recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverFlujoMaximo(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
