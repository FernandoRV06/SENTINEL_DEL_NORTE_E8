/*
 * Descripcion: desafio 01 · Optimal Network Deployment (arbol de expansion minima).
 * Conecta todas las centrales al menor costo de instalacion.
 */

#ifndef MST_H
#define MST_H

#if __has_include("json.hpp")
#include "json.hpp"
#elif __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#else
#error "No se encontro json.hpp; agregue la ruta de nlohmann/json a includePath."
#endif

using Json = nlohmann::json;

/*
 * Atiende el modo "mst". Version provisional: solo confirma que recibio
 * la entrada; se reemplaza por Kruskal cuando se conozca el formato.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverMst(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
