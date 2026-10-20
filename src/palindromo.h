/*
 * Descripcion: desafio 06 · Mirrored Code Analysis (palindromo mas largo).
 * Encuentra en cada transmision el palindromo continuo mas largo, de
 * longitud par o impar, sobre caracteres hexadecimales.
 * Responsable: integrante 3; Tecnicas: Manacher (principal) y expansion
 * desde el centro.
 * Autores: Nombre Apellido (A00843290), Nombre Apellido (A0XXXXXXX),
 *          Nombre Apellido (A0XXXXXXX), Nombre Apellido (A0XXXXXXX)
 * Fecha de creacion: 20/10/2026
 */

#ifndef PALINDROMO_H
#define PALINDROMO_H

#include "json.h"
#include "transmision.h"

/*
 * Atiende el modo de palindromo mas largo. Version provisional: solo
 * confirma que recibio la entrada.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 * Retorno: objeto JSON con la respuesta del modo.
 */
inline ValorJson resolverPalindromo(const ValorJson &entrada) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("estado", crearTexto("pendiente"));
	respuesta.agregarCampo("camposRecibidos", crearNumero(static_cast<double>(entrada.llaves.size())));
	return respuesta;
}

#endif
