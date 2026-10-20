/*
 * Descripcion: desafio 02 · Optimal Maintenance Route (problema del viajante).
 * Visita cada central una vez y regresa al origen con la menor distancia;
 * entre paradas se puede pasar por centrales intermedias.
 * Responsable: integrante 1; Tecnicas: Floyd-Warshall para los caminos,
 * Held-Karp (exacto) y vecino mas cercano (aproximado).
 * Autores: Nombre Apellido (A00843290), Nombre Apellido (A0XXXXXXX),
 *          Nombre Apellido (A0XXXXXXX), Nombre Apellido (A0XXXXXXX)
 * Fecha de creacion: 20/10/2026
 */

#ifndef TSP_H
#define TSP_H

#include "json.h"

/*
 * Atiende el modo "tsp". Version provisional: solo confirma que recibio
 * la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverTsp(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
