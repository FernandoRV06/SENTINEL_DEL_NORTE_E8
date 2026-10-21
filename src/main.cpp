/*
 * SENTINEL Digital Twin · nucleo algoritmico.
 * Descripcion: lee UN objeto JSON de la entrada estandar con la forma
 * {"mode": "...", "input": {...}}, lo envia al modulo del desafio indicado
 * y escribe UN objeto JSON con la respuesta en la salida estandar.
 * Usa la libreria externa nlohmann/json para leer y escribir JSON.
 */

#include <exception>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "json.hpp"

#include "firmas.h"
#include "flujoMaximo.h"
#include "mst.h"
#include "palindromo.h"
#include "subcadenaComun.h"
#include "tsp.h"
#include "voronoi.h"

// Nombres de los modos.
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
Json despacharModo(const std::string &modo, const Json &entrada) {
	Json respuesta;
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
 * Punto de entrada: lee la peticion, la resuelve y escribe la respuesta.
 * Parametros: ninguno.
 * Retorno: 0 al terminar, aun si la peticion tuvo errores (se reportan en JSON).
 */
int main() {
	Json respuesta;
	try {
		Json peticion = Json::parse(leerEntradaCompleta());
		std::string modo = peticion.at("mode").get<std::string>();
		respuesta = despacharModo(modo, peticion.at("input"));
	} catch (const std::exception &error) {
		respuesta = {{"error", error.what()}};
	}
	std::cout << respuesta.dump() << "\n";
	return 0;
}
