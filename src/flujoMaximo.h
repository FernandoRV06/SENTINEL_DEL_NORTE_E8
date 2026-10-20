/*
 * Descripcion: desafio 03 · Maximum Information Flow (flujo maximo).
 * Calcula la maxima informacion transportable entre dos nodos usando las
 * capacidades de los enlaces (no las demandas).
 * Responsable: integrante 2; Tecnicas: Edmonds-Karp (principal) y
 * Ford-Fulkerson con DFS.
 * Autores: Carlos Emiliano Nava Flores (A0XXXXXXX),
 *          Luis Enrique Castillo Carrillo (A0XXXXXXX),
 *          Desiderio Ivan Ortegon Morton (A0XXXXXXX),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
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
