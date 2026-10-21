/*
 * Descripcion: desafio 07 · Transmission Similarity Analysis (subcadena comun mas larga).
 * Encuentra la subcadena continua mas larga presente en ambas
 * transmisiones y reporta sus posiciones en la primera.
 * Usa un arreglo de sufijos de "t1 + separador + t2" (duplicacion de
 * prefijos con ordenamiento por conteo, O(n log n)) y el arreglo LCP de
 * Kasai (O(n)); la respuesta es el mayor LCP entre un sufijo de t1 y uno de t2.
 * Alternativa para comparar: programacion dinamica O(n * m) con O(m) memoria.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef SUBCADENA_COMUN_H
#define SUBCADENA_COMUN_H

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.h"
#include "transmision.h"

// Simbolos reservados: el final va antes que todo y el separador despues;
// los caracteres reales se recorren dos lugares para no chocar con ellos.
const std::size_t SIMBOLO_FINAL = 0;
const std::size_t SIMBOLO_SEPARADOR = 1;
const std::size_t DESPLAZAMIENTO_CARACTER = 2;
const std::size_t TAMANO_ALFABETO = 258;

/*
 * Ordena por conteo los indices segun su clase (estable).
 * Parametros:
 *   indices: indices a ordenar.
 *   clase: clase de cada posicion.
 *   cantidadClases: numero de clases distintas.
 * Retorno: los indices ordenados por clase.
 */
inline std::vector<std::size_t> ordenarPorConteo(const std::vector<std::size_t> &indices, const std::vector<std::size_t> &clase, std::size_t cantidadClases) {
	std::vector<std::size_t> conteo(cantidadClases + 1, 0);
	for (std::size_t indice : indices) {
		conteo[clase[indice] + 1]++;
	}
	for (std::size_t valor = 1; valor <= cantidadClases; valor++) {
		conteo[valor] += conteo[valor - 1];
	}
	std::vector<std::size_t> ordenados(indices.size());
	for (std::size_t indice : indices) {
		ordenados[conteo[clase[indice]]] = indice;
		conteo[clase[indice]]++;
	}
	return ordenados;
}

/*
 * Construye el arreglo de sufijos ordenando desplazamientos ciclicos de
 * longitud 1, 2, 4, ...; como el texto termina en SIMBOLO_FINAL (unico y
 * menor), el orden ciclico coincide con el de los sufijos.
 * Parametros:
 *   texto: simbolos del texto, terminado en SIMBOLO_FINAL.
 * Retorno: las posiciones iniciales de los sufijos en orden lexicografico.
 */
inline std::vector<std::size_t> construirArregloSufijos(const std::vector<std::size_t> &texto) {
	std::size_t cantidad = texto.size();
	std::vector<std::size_t> orden(cantidad);
	for (std::size_t indice = 0; indice < cantidad; indice++) {
		orden[indice] = indice;
	}
	orden = ordenarPorConteo(orden, texto, TAMANO_ALFABETO);
	std::vector<std::size_t> clase(cantidad, 0);
	std::size_t cantidadClases = 1;
	for (std::size_t indice = 1; indice < cantidad; indice++) {
		if (texto[orden[indice]] != texto[orden[indice - 1]]) {
			cantidadClases++;
		}
		clase[orden[indice]] = cantidadClases - 1;
	}
	std::size_t longitud = 1;
	while (longitud < cantidad && cantidadClases < cantidad) {
		// Ordenar por (clase[i], clase[i + longitud]): los indices ya estan
		// ordenados por la segunda mitad si se recorren "longitud" lugares atras.
		std::vector<std::size_t> recorridos(cantidad);
		for (std::size_t indice = 0; indice < cantidad; indice++) {
			recorridos[indice] = (orden[indice] + cantidad - longitud) % cantidad;
		}
		orden = ordenarPorConteo(recorridos, clase, cantidadClases);
		std::vector<std::size_t> nuevaClase(cantidad, 0);
		cantidadClases = 1;
		for (std::size_t indice = 1; indice < cantidad; indice++) {
			std::size_t actual = orden[indice];
			std::size_t previo = orden[indice - 1];
			bool distinto = clase[actual] != clase[previo] || clase[(actual + longitud) % cantidad] != clase[(previo + longitud) % cantidad];
			if (distinto) {
				cantidadClases++;
			}
			nuevaClase[actual] = cantidadClases - 1;
		}
		clase = nuevaClase;
		longitud *= 2;
	}
	return orden;
}

/*
 * Calcula el arreglo LCP con Kasai: lcp[i] es el prefijo comun entre los
 * sufijos sufijos[i] y sufijos[i + 1]. O(n).
 * Parametros:
 *   texto: simbolos del texto.
 *   sufijos: arreglo de sufijos del texto.
 * Retorno: el arreglo LCP (el ultimo valor es 0).
 */
inline std::vector<std::size_t> construirLcp(const std::vector<std::size_t> &texto, const std::vector<std::size_t> &sufijos) {
	std::size_t cantidad = texto.size();
	std::vector<std::size_t> rango(cantidad, 0);
	for (std::size_t indice = 0; indice < cantidad; indice++) {
		rango[sufijos[indice]] = indice;
	}
	std::vector<std::size_t> lcp(cantidad, 0);
	std::size_t comun = 0;
	for (std::size_t posicion = 0; posicion < cantidad; posicion++) {
		if (rango[posicion] + 1 < cantidad) {
			std::size_t siguiente = sufijos[rango[posicion] + 1];
			while (posicion + comun < cantidad && siguiente + comun < cantidad && texto[posicion + comun] == texto[siguiente + comun]) {
				comun++;
			}
			lcp[rango[posicion]] = comun;
			if (comun > 0) {
				comun--;
			}
		} else {
			comun = 0;
		}
	}
	return lcp;
}

/*
 * Encuentra la subcadena comun mas larga; con empate gana la que empieza
 * antes en la primera transmision.
 * Parametros:
 *   primera, segunda: transmisiones normalizadas.
 * Retorno: par {inicio, longitud} con el inicio en base 0 (longitud 0 si no hay).
 */
inline std::vector<std::size_t> buscarSubcadenaComun(const std::string &primera, const std::string &segunda) {
	std::vector<std::size_t> texto;
	for (char caracter : primera) {
		texto.push_back(static_cast<unsigned char>(caracter) + DESPLAZAMIENTO_CARACTER);
	}
	texto.push_back(SIMBOLO_SEPARADOR);
	for (char caracter : segunda) {
		texto.push_back(static_cast<unsigned char>(caracter) + DESPLAZAMIENTO_CARACTER);
	}
	texto.push_back(SIMBOLO_FINAL);
	std::vector<std::size_t> sufijos = construirArregloSufijos(texto);
	std::vector<std::size_t> lcp = construirLcp(texto, sufijos);
	std::size_t cantidad = texto.size();
	std::size_t limitePrimera = primera.size();
	std::size_t limiteSegunda = primera.size() + 1 + segunda.size();
	// mejor[p]: prefijo comun mas largo del sufijo p de t1 con algun sufijo de t2.
	// Es el minimo de LCP hasta el sufijo de t2 mas cercano, hacia arriba o abajo.
	const std::size_t SIN_SEGUNDA = std::numeric_limits<std::size_t>::max();
	std::vector<std::size_t> mejor(limitePrimera, 0);
	std::size_t minimo = 0;
	for (std::size_t indice = 0; indice < cantidad; indice++) {
		if (indice > 0) {
			minimo = std::min(minimo, lcp[indice - 1]);
		}
		std::size_t posicion = sufijos[indice];
		if (posicion > limitePrimera && posicion < limiteSegunda) {
			minimo = SIN_SEGUNDA;
		} else if (posicion < limitePrimera && minimo != SIN_SEGUNDA) {
			mejor[posicion] = std::max(mejor[posicion], minimo);
		}
	}
	minimo = 0;
	for (std::size_t indice = cantidad; indice > 0; indice--) {
		if (indice < cantidad) {
			minimo = std::min(minimo, lcp[indice - 1]);
		}
		std::size_t posicion = sufijos[indice - 1];
		if (posicion > limitePrimera && posicion < limiteSegunda) {
			minimo = SIN_SEGUNDA;
		} else if (posicion < limitePrimera && minimo != SIN_SEGUNDA) {
			mejor[posicion] = std::max(mejor[posicion], minimo);
		}
	}
	std::vector<std::size_t> resultado = {0, 0};
	for (std::size_t posicion = 0; posicion < limitePrimera; posicion++) {
		if (mejor[posicion] > resultado[1]) {
			resultado[0] = posicion;
			resultado[1] = mejor[posicion];
		}
	}
	return resultado;
}

/*
 * Encuentra la subcadena comun mas larga con programacion dinamica:
 * largo[j] es la longitud del sufijo comun mas largo que termina en
 * primera[i] y segunda[j]. Solo se guarda la fila anterior. O(n * m).
 * Parametros:
 *   primera, segunda: transmisiones normalizadas.
 * Retorno: par {inicio, longitud} con el inicio en base 0 (longitud 0 si no hay).
 */
inline std::vector<std::size_t> buscarSubcadenaComunDinamica(const std::string &primera, const std::string &segunda) {
	std::vector<std::size_t> resultado = {0, 0};
	std::vector<std::size_t> anterior(segunda.size() + 1, 0);
	std::vector<std::size_t> actual(segunda.size() + 1, 0);
	for (std::size_t fila = 0; fila < primera.size(); fila++) {
		for (std::size_t columna = 0; columna < segunda.size(); columna++) {
			actual[columna + 1] = 0;
			if (primera[fila] == segunda[columna]) {
				actual[columna + 1] = anterior[columna] + 1;
			}
			// Las filas avanzan sobre t1: con la misma longitud, la primera que
			// aparece es la que empieza antes, asi que solo se acepta si es mas larga.
			if (actual[columna + 1] > resultado[1]) {
				resultado[1] = actual[columna + 1];
				resultado[0] = fila + 1 - resultado[1];
			}
		}
		std::swap(anterior, actual);
	}
	return resultado;
}

/*
 * Atiende el modo de subcadena comun mas larga.
 * Parametros:
 *   entrada: campo "input" con "transmissions" (dos cadenas) y, opcional,
 *            "technique": "suffix-array" (por defecto) o "dp".
 * Retorno: objeto JSON {"range": [inicio, fin]} en base 1 sobre la primera
 *          transmision, o [0, 0] si no comparten ningun caracter.
 */
inline ValorJson resolverSubcadenaComun(const ValorJson &entrada) {
	std::vector<std::string> transmisiones = leerTransmisiones(entrada.obtenerCampo("transmissions"));
	if (transmisiones.size() != 2) {
		throw std::runtime_error("se esperaban exactamente dos transmisiones");
	}
	std::string tecnica = leerTecnica(entrada, "suffix-array");
	std::vector<std::size_t> encontrada;
	if (tecnica == "suffix-array") {
		encontrada = buscarSubcadenaComun(transmisiones[0], transmisiones[1]);
	} else if (tecnica == "dp") {
		encontrada = buscarSubcadenaComunDinamica(transmisiones[0], transmisiones[1]);
	} else {
		throw std::runtime_error("tecnica de common desconocida: " + tecnica);
	}
	double inicio = 0.0;
	double fin = 0.0;
	if (encontrada[1] > 0) {
		inicio = static_cast<double>(encontrada[0] + 1);
		fin = static_cast<double>(encontrada[0] + encontrada[1]);
	}
	ValorJson rango = crearArreglo();
	rango.agregarElemento(crearNumero(inicio));
	rango.agregarElemento(crearNumero(fin));
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("range", rango);
	return respuesta;
}

#endif
