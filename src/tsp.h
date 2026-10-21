/*
 * Descripcion: desafio 02 · Optimal Maintenance Route (problema del viajante).
 * Visita cada central una vez y regresa al origen con la menor distancia;
 * entre paradas se puede pasar por centrales intermedias.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
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
