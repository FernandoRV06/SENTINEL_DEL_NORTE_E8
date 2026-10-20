/*
 * Descripcion: desafio 06 · Mirrored Code Analysis (palindromo mas largo).
 * Encuentra en cada transmision el palindromo continuo mas largo, de
 * longitud par o impar, sobre caracteres hexadecimales.
 * Responsable: integrante 3; Tecnicas: Manacher (principal) y expansion
 * desde el centro.
 * Autores: Carlos Emiliano Nava Flores (A0XXXXXXX),
 *          Luis Enrique Castillo Carrillo (A0XXXXXXX),
 *          Desiderio Ivan Ortegon Morton (A0XXXXXXX),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
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
