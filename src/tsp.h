/*
 * Descripcion: desafio 02 · Optimal Maintenance Route (problema del viajante).
 * Visita cada central una vez y regresa al origen con la menor distancia;
 * entre paradas se puede pasar por centrales intermedias.
 */

#ifndef TSP_H
#define TSP_H

#include "json.hpp"

using Json = nlohmann::json;

/*
 * Atiende el modo "tsp". Version provisional: solo confirma que recibio
 * la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverTsp(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
