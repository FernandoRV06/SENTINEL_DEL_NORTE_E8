/*
 * Descripcion: desafio 04 (Geospatial Network Coverage): diagrama de Voronoi.
 * Divide el rectangulo del dominio en regiones segun la central mas cercana,
 * sobre coordenadas planas. La celda de cada central es la interseccion del
 * rectangulo con los semiplanos "mas cerca de esta central que de la otra".
 * Usa recorte por semiplanos (O(n^2) en el peor caso, con poda por
 * distancia); Fortune queda como alternativa analizada. Para comparar se
 * puede pedir el mismo recorte sin poda (contra las n - 1 mediatrices).
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef VORONOI_H
#define VORONOI_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.h"

const double TOLERANCIA_VORONOI = 1e-9;

/*
 * Punto del plano con coordenadas reales.
 */
struct PuntoVoronoi {
	double x = 0.0;
	double y = 0.0;
};

/*
 * Lee un arreglo JSON [x, y] como punto.
 * Parametros:
 *   valor: arreglo JSON con dos numeros.
 * Retorno: el punto leido; lanza una excepcion si no tiene dos coordenadas.
 */
inline PuntoVoronoi leerPuntoVoronoi(const ValorJson &valor) {
	if (valor.elementos.size() < 2) {
		throw std::runtime_error("cada sitio debe tener coordenadas [x, y]");
	}
	PuntoVoronoi punto;
	punto.x = valor.elementos[0].numero;
	punto.y = valor.elementos[1].numero;
	return punto;
}

/*
 * Recorta un poligono convexo con el semiplano a*x + b*y <= c
 * (paso de Sutherland-Hodgman para una sola recta).
 * Parametros:
 *   poligono: vertices en orden.
 *   a, b, c: coeficientes del semiplano.
 * Retorno: los vertices del poligono recortado (vacio si no queda nada).
 */
inline std::vector<PuntoVoronoi> recortarSemiplano(const std::vector<PuntoVoronoi> &poligono, double a, double b, double c) {
	std::vector<PuntoVoronoi> recortado;
	for (std::size_t indice = 0; indice < poligono.size(); indice++) {
		const PuntoVoronoi &actual = poligono[indice];
		const PuntoVoronoi &siguiente = poligono[(indice + 1) % poligono.size()];
		double ladoActual = a * actual.x + b * actual.y - c;
		double ladoSiguiente = a * siguiente.x + b * siguiente.y - c;
		if (ladoActual <= TOLERANCIA_VORONOI) {
			recortado.push_back(actual);
		}
		bool cruzaHaciaFuera = ladoActual < -TOLERANCIA_VORONOI && ladoSiguiente > TOLERANCIA_VORONOI;
		bool cruzaHaciaDentro = ladoActual > TOLERANCIA_VORONOI && ladoSiguiente < -TOLERANCIA_VORONOI;
		if (cruzaHaciaFuera || cruzaHaciaDentro) {
			double proporcion = ladoActual / (ladoActual - ladoSiguiente);
			PuntoVoronoi cruce;
			cruce.x = actual.x + proporcion * (siguiente.x - actual.x);
			cruce.y = actual.y + proporcion * (siguiente.y - actual.y);
			recortado.push_back(cruce);
		}
	}
	return recortado;
}

/*
 * Calcula la mayor distancia al cuadrado entre un sitio y los vertices de su celda.
 * Parametros:
 *   sitio: centro de la celda.
 *   celda: vertices de la celda.
 * Retorno: el radio al cuadrado de la celda vista desde el sitio.
 */
inline double calcularRadioCuadrado(const PuntoVoronoi &sitio, const std::vector<PuntoVoronoi> &celda) {
	double radio = 0.0;
	for (const PuntoVoronoi &vertice : celda) {
		double dx = vertice.x - sitio.x;
		double dy = vertice.y - sitio.y;
		radio = std::max(radio, dx * dx + dy * dy);
	}
	return radio;
}

/*
 * Construye la celda de un sitio recortando el rectangulo con la mediatriz
 * de cada otro sitio, del mas cercano al mas lejano. Cuando un sitio esta a
 * mas del doble del radio de la celda, su mediatriz ya no puede tocarla, y
 * los siguientes tampoco (estan aun mas lejos), asi que se deja de recortar.
 * Parametros:
 *   sitios: todas las centrales.
 *   propio: indice de la central cuya celda se construye.
 *   rectangulo: vertices del dominio en sentido antihorario.
 *   podar: false recorta contra todos los sitios (version sin poda).
 * Retorno: los vertices de la celda.
 */
inline std::vector<PuntoVoronoi> construirCelda(const std::vector<PuntoVoronoi> &sitios, std::size_t propio, const std::vector<PuntoVoronoi> &rectangulo, bool podar) {
	const PuntoVoronoi &sitio = sitios[propio];
	std::vector<std::size_t> otros;
	for (std::size_t indice = 0; indice < sitios.size(); indice++) {
		if (indice != propio) {
			otros.push_back(indice);
		}
	}
	std::vector<double> distanciaCuadrada(sitios.size(), 0.0);
	for (std::size_t indice : otros) {
		double dx = sitios[indice].x - sitio.x;
		double dy = sitios[indice].y - sitio.y;
		distanciaCuadrada[indice] = dx * dx + dy * dy;
	}
	std::sort(otros.begin(), otros.end(), [&distanciaCuadrada](std::size_t izquierdo, std::size_t derecho) {
		return distanciaCuadrada[izquierdo] < distanciaCuadrada[derecho];
	});
	std::vector<PuntoVoronoi> celda = rectangulo;
	std::size_t posicion = 0;
	bool puedeRecortar = true;
	while (posicion < otros.size() && puedeRecortar && !celda.empty()) {
		const PuntoVoronoi &otro = sitios[otros[posicion]];

		if (podar && distanciaCuadrada[otros[posicion]] > 4.0 * calcularRadioCuadrado(sitio, celda) + TOLERANCIA_VORONOI) {
			puedeRecortar = false;
		} else {
			double a = 2.0 * (otro.x - sitio.x);
			double b = 2.0 * (otro.y - sitio.y);
			double c = otro.x * otro.x + otro.y * otro.y - sitio.x * sitio.x - sitio.y * sitio.y;
			celda = recortarSemiplano(celda, a, b, c);
			posicion++;
		}
	}
	return celda;
}

/*
 * Atiende el modo "voronoi": calcula una celda por sitio, en el mismo orden.
 * Parametros:
 *   entrada: campo "input" con "sites" ([[x, y], ...]) y
 *            "bounds" ([xMin, yMin, xMax, yMax]) y, opcional, "technique":
 *            "half-plane" (por defecto, con poda) o "half-plane-naive".
 * Retorno: objeto JSON {"cells": [[[x, y], ...], ...]}.
 */
inline ValorJson resolverVoronoi(const ValorJson &entrada) {
	const ValorJson &limites = entrada.obtenerCampo("bounds");
	if (limites.elementos.size() != 4) {
		throw std::runtime_error("bounds debe ser [xMin, yMin, xMax, yMax]");
	}
	double xMinimo = limites.elementos[0].numero;
	double yMinimo = limites.elementos[1].numero;
	double xMaximo = limites.elementos[2].numero;
	double yMaximo = limites.elementos[3].numero;
	std::vector<PuntoVoronoi> rectangulo(4);
	rectangulo[0].x = xMinimo;
	rectangulo[0].y = yMinimo;
	rectangulo[1].x = xMaximo;
	rectangulo[1].y = yMinimo;
	rectangulo[2].x = xMaximo;
	rectangulo[2].y = yMaximo;
	rectangulo[3].x = xMinimo;
	rectangulo[3].y = yMaximo;
	std::vector<PuntoVoronoi> sitios;
	for (const ValorJson &sitio : entrada.obtenerCampo("sites").elementos) {
		sitios.push_back(leerPuntoVoronoi(sitio));
	}
	std::string tecnica = leerTecnica(entrada, "half-plane");
	if (tecnica != "half-plane" && tecnica != "half-plane-naive") {
		throw std::runtime_error("tecnica de voronoi desconocida: " + tecnica);
	}
	ValorJson celdas = crearArreglo();
	for (std::size_t indice = 0; indice < sitios.size(); indice++) {
		ValorJson celdaJson = crearArreglo();
		for (const PuntoVoronoi &vertice : construirCelda(sitios, indice, rectangulo, tecnica == "half-plane")) {
			ValorJson verticeJson = crearArreglo();
			verticeJson.agregarElemento(crearNumero(vertice.x));
			verticeJson.agregarElemento(crearNumero(vertice.y));
			celdaJson.agregarElemento(verticeJson);
		}
		celdas.agregarElemento(celdaJson);
	}
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("cells", celdas);
	return respuesta;
}

#endif
