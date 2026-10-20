/*
 * Descripcion: desafio 02 · Optimal Maintenance Route (problema del viajante).
 * Visita cada central una vez y regresa al origen con la menor distancia;
 * entre paradas se puede pasar por centrales intermedias.
 * Responsable: integrante 1; Tecnicas: Floyd-Warshall para los caminos,
 * Held-Karp (exacto) y vecino mas cercano (aproximado).
 * Autores: Carlos Emiliano Nava Flores (A0XXXXXXX),
 *          Luis Enrique Castillo Carrillo (A0XXXXXXX),
 *          Desiderio Ivan Ortegon Morton (A0XXXXXXX),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
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
