/*
 * Descripcion: desafio 03 (Maximum Information Flow): flujo maximo.
 * Calcula la maxima informacion transportable entre dos nodos usando las
 * capacidades de los enlaces (no las demandas).
 */

#ifndef FLUJO_MAXIMO_H
#define FLUJO_MAXIMO_H

#include "json.h"

/*
 * Atiende el modo de flujo maximo. Version provisional: solo confirma que
 * recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverFlujoMaximo(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
