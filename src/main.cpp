/*
 * SENTINEL Digital Twin · nucleo algoritmico (Hackathon TC2038).
 * Descripcion: lee UN objeto JSON de la entrada estandar con la forma
 * {"mode": "...", "input": {...}}, lo envia al modulo del desafio indicado
 * y escribe UN objeto JSON con la respuesta en la salida estandar.
 * Autores: Nombre Apellido (A00843290), Nombre Apellido (A0XXXXXXX),
 *          Nombre Apellido (A0XXXXXXX), Nombre Apellido (A0XXXXXXX)
 * Fecha de creacion: 20/10/2026
 */

#include <exception>
#include <iostream>
#include <iterator>
#include <string>

#include "firmas.h"
#include "flujoMaximo.h"
#include "json.h"
#include "mst.h"
#include "palindromo.h"
#include "subcadenaComun.h"
#include "tsp.h"
#include "voronoi.h"

// Nombres de los modos, tal como los envia el autograder de la pagina.
const std::string MODO_MST = "mst";
const std::string MODO_TSP = "tsp";
const std::string MODO_FLUJO = "flow";
const std::string MODO_VORONOI = "voronoi";
const std::string MODO_FIRMAS = "matching";
const std::string MODO_PALINDROMO = "palindrome";
const std::string MODO_SUBCADENA = "common";

/*
 * Lee toda la entrada estandar hasta el final.
 * Parametros: ninguno.
 * Retorno: el texto completo recibido.
 */
std::string leerEntradaCompleta() {
	std::string entrada((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
	return entrada;
}

/*
 * Envia la entrada al modulo que corresponde al modo solicitado.
 * Parametros:
 *   modo: nombre del desafio a resolver.
 *   entrada: campo "input" de la peticion.
 * Retorno: la respuesta del modulo; lanza una excepcion si el modo no existe.
 */
ValorJson despacharModo(const std::string &modo, const ValorJson &entrada) {
	ValorJson respuesta;
	if (modo == MODO_MST) {
		respuesta = resolverMst(entrada);
	} else if (modo == MODO_TSP) {
		respuesta = resolverTsp(entrada);
	} else if (modo == MODO_FLUJO) {
		respuesta = resolverFlujoMaximo(entrada);
	} else if (modo == MODO_VORONOI) {
		respuesta = resolverVoronoi(entrada);
	} else if (modo == MODO_FIRMAS) {
		respuesta = resolverFirmas(entrada);
	} else if (modo == MODO_PALINDROMO) {
		respuesta = resolverPalindromo(entrada);
	} else if (modo == MODO_SUBCADENA) {
		respuesta = resolverSubcadenaComun(entrada);
	} else {
		throw std::runtime_error("modo desconocido: " + modo);
	}
	return respuesta;
}

/*
 * Crea la respuesta que se envia cuando la peticion no se puede atender.
 * Parametros:
 *   mensaje: descripcion del problema encontrado.
 * Retorno: objeto JSON con el campo "error".
 */
ValorJson crearRespuestaError(const std::string &mensaje) {
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("error", crearTexto(mensaje));
	return respuesta;
}

/*
 * Punto de entrada: lee la peticion, la resuelve y escribe la respuesta.
 * Parametros: ninguno.
 * Retorno: 0 al terminar, aun si la peticion tuvo errores (se reportan en JSON).
 */
int main() {
	ValorJson respuesta;
	try {
		LectorJson lector;
		ValorJson peticion = lector.leerDocumento(leerEntradaCompleta());
		std::string modo = peticion.obtenerCampo("mode").texto;
		respuesta = despacharModo(modo, peticion.obtenerCampo("input"));
	} catch (const std::exception &error) {
		respuesta = crearRespuestaError(error.what());
	}
	std::cout << escribirJson(respuesta) << "\n";
	return 0;
}
