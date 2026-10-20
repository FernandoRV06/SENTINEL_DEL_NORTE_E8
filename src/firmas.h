/*
 * Descripcion: desafio 05 · Malware Signature Detection (busqueda de patrones).
 * Busca tres firmas en dos transmisiones y reporta la primera posicion
 * de cada coincidencia, contando desde la posicion uno.
 * Responsable: integrante 3; Tecnicas: KMP (principal) y busqueda ingenua.
 * Autores: Carlos Emiliano Nava Flores, Luis Enrique Castillo Carrillo,
 *          Desiderio Iván Ortegón Mortón, Fernando Robles Vidrio
 * Fecha de creacion: 20/10/2026
 */

#ifndef FIRMAS_H
#define FIRMAS_H

#include "json.h"
#include "transmision.h"

/*
 * Atiende el modo de deteccion de firmas. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverFirmas(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
