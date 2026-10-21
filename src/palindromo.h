/*
 * Descripcion: desafio 06 · Mirrored Code Analysis (palindromo mas largo).
 * Encuentra en cada transmision el palindromo continuo mas largo, de
 * longitud par o impar, sobre caracteres hexadecimales.
 */

#ifndef PALINDROMO_H
#define PALINDROMO_H

#include "json.hpp"

using Json = nlohmann::json;

#include "transmision.h"

/*
 * Atiende el modo de palindromo mas largo. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline Json resolverPalindromo(const Json &entrada) {
	Json respuesta = {{"estado", "pendiente"}, {"camposRecibidos", entrada.size()}};
	return respuesta;
}

#endif
