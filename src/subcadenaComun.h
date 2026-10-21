/*
 * Descripcion: desafio 07 (Transmission Similarity Analysis): subcadena comun mas larga.
 * Encuentra la subcadena continua mas larga presente en ambas
 * transmisiones y reporta sus posiciones en la primera.
 */

#ifndef SUBCADENA_COMUN_H
#define SUBCADENA_COMUN_H

#include "json.h"

#include "transmision.h"

/*
 * Atiende el modo de subcadena comun mas larga. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverSubcadenaComun(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
