/*
 * Descripcion: desafio 04 · Geospatial Network Coverage (diagrama de Voronoi).
 * Divide el dominio en regiones segun la central mas cercana y responde
 * consultas de proximidad, sobre coordenadas planas.
 * Responsable: integrante 2; Tecnicas: recorte por semiplanos (principal) y
 * Fortune como alternativa analizada.
 * Autores: Carlos Emiliano Nava Flores (A0XXXXXXX),
 *          Luis Enrique Castillo Carrillo (A0XXXXXXX),
 *          Desiderio Ivan Ortegon Morton (A0XXXXXXX),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef VORONOI_H
#define VORONOI_H

#include "json.hpp"

using Json = nlohmann::json;

/*
 * Atiende el modo "voronoi". Version provisional: solo confirma que
 * recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverVoronoi(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
