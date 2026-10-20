/*
 * Descripcion: desafio 04 · Geospatial Network Coverage (diagrama de Voronoi).
 * Divide el dominio en regiones segun la central mas cercana y responde
 * consultas de proximidad, sobre coordenadas planas.
 * Responsable: integrante 2; Tecnicas: recorte por semiplanos (principal) y
 * Fortune como alternativa analizada.
 * Autores: Nombre Apellido (A00843290), Nombre Apellido (A0XXXXXXX),
 *          Nombre Apellido (A0XXXXXXX), Nombre Apellido (A0XXXXXXX)
 * Fecha de creacion: 20/10/2026
 */

#ifndef VORONOI_H
#define VORONOI_H

#include "json.h"

/*
 * Atiende el modo "voronoi". Version provisional: solo confirma que
 * recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverVoronoi(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
