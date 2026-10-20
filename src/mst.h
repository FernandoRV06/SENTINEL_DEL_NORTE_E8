/*
 * Descripcion: desafio 01 · Optimal Network Deployment (arbol de expansion minima).
 * Conecta todas las centrales al menor costo de instalacion.
 * Responsable: integrante 1; Tecnicas: Kruskal (principal) y Prim.
 * Autores: Carlos Emiliano Nava Flores, Luis Enrique Castillo Carrillo,
 *          Desiderio Iván Ortegón Mortón, Fernando Robles Vidrio
 * Fecha de creacion: 20/10/2026
 */

#ifndef MST_H
#define MST_H

#include "json.h"

/*
 * Atiende el modo "mst". Version provisional: solo confirma que recibio
 * la entrada; se reemplaza por Kruskal cuando se conozca el formato.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverMst(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
