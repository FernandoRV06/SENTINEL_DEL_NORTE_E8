/*
 * Descripcion: desafio 01 (Optimal Network Deployment): arbol de expansion minima.
 * Conecta todas las centrales al menor costo de instalacion.
 */

#ifndef MST_H
#define MST_H

#include "json.h"

/*
 * Atiende el modo "mst". Version provisional: solo confirma que recibio
 * la entrada; se reemplaza por Kruskal.
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
