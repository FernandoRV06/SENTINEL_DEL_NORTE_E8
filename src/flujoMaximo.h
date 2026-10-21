/*
 * Descripcion: desafio 03 · Maximum Information Flow (flujo maximo).
 * Calcula la maxima informacion transportable entre dos nodos usando las
 * capacidades de los enlaces (no las demandas). Los enlaces son dirigidos.
 * Usa Edmonds-Karp: busca caminos de aumento con BFS en la red residual,
 * siempre el de menos aristas. Tiempo O(V * E^2), memoria O(V + E).
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef FLUJO_MAXIMO_H
#define FLUJO_MAXIMO_H

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "json.h"

// Marca de "sin arista" en las listas de la red residual.
const std::size_t SIN_ARISTA_FLUJO = std::numeric_limits<std::size_t>::max();
// Capacidad residual minima para considerar que una arista sigue abierta.
const double TOLERANCIA_FLUJO = 1e-9;

/*
 * Red residual guardada como listas de adyacencia en arreglos: la arista
 * k y su reversa k ^ 1 se agregan juntas, asi que la reversa se encuentra
 * sin buscarla.
 */
class RedResidual {
public:
	/*
	 * Crea una red sin aristas.
	 * Parametros:
	 *   cantidad: numero de nodos.
	 */
	explicit RedResidual(std::size_t cantidad) : primera(cantidad, SIN_ARISTA_FLUJO) {
	}

	/*
	 * Agrega la arista dirigida origen -> destino y su reversa con capacidad 0.
	 * Parametros:
	 *   origen, llegada: extremos de la arista.
	 *   valor: capacidad de la arista.
	 * Retorno: ninguno.
	 */
	void agregarArista(std::size_t origen, std::size_t llegada, double valor) {
		agregarUnaArista(origen, llegada, valor);
		agregarUnaArista(llegada, origen, 0.0);
	}

	/*
	 * Calcula el flujo maximo con Edmonds-Karp.
	 * Parametros:
	 *   fuente, sumidero: nodos de inicio y fin.
	 * Retorno: el valor del flujo maximo.
	 */
	double calcularFlujoMaximo(std::size_t fuente, std::size_t sumidero) {
		double total = 0.0;
		std::vector<std::size_t> aristaLlegada(primera.size(), SIN_ARISTA_FLUJO);
		while (fuente != sumidero && buscarCamino(fuente, sumidero, aristaLlegada)) {
			// El cuello de botella es la menor capacidad residual del camino.
			double cuello = std::numeric_limits<double>::infinity();
			std::size_t nodo = sumidero;
			while (nodo != fuente) {
				std::size_t arista = aristaLlegada[nodo];
				cuello = std::min(cuello, capacidad[arista]);
				nodo = destino[arista ^ 1];
			}
			nodo = sumidero;
			while (nodo != fuente) {
				std::size_t arista = aristaLlegada[nodo];
				capacidad[arista] -= cuello;
				capacidad[arista ^ 1] += cuello;
				nodo = destino[arista ^ 1];
			}
			total += cuello;
		}
		return total;
	}

private:
	std::vector<std::size_t> primera;
	std::vector<std::size_t> siguiente;
	std::vector<std::size_t> destino;
	std::vector<double> capacidad;

	/*
	 * Agrega una sola arista al inicio de la lista de su origen.
	 * Parametros:
	 *   origen, llegada: extremos de la arista.
	 *   valor: capacidad de la arista.
	 * Retorno: ninguno.
	 */
	void agregarUnaArista(std::size_t origen, std::size_t llegada, double valor) {
		destino.push_back(llegada);
		capacidad.push_back(valor);
		siguiente.push_back(primera[origen]);
		primera[origen] = destino.size() - 1;
	}

	/*
	 * Busca con BFS el camino de aumento con menos aristas.
	 * Parametros:
	 *   fuente, sumidero: nodos de inicio y fin.
	 *   aristaLlegada: se llena con la arista por la que se llego a cada nodo.
	 * Retorno: true si el sumidero es alcanzable en la red residual.
	 */
	bool buscarCamino(std::size_t fuente, std::size_t sumidero, std::vector<std::size_t> &aristaLlegada) {
		std::vector<bool> visitado(primera.size(), false);
		std::vector<std::size_t> cola(1, fuente);
		visitado[fuente] = true;
		std::size_t frente = 0;
		while (frente < cola.size() && !visitado[sumidero]) {
			std::size_t nodo = cola[frente];
			frente++;
			for (std::size_t arista = primera[nodo]; arista != SIN_ARISTA_FLUJO; arista = siguiente[arista]) {
				std::size_t vecino = destino[arista];
				if (!visitado[vecino] && capacidad[arista] > TOLERANCIA_FLUJO) {
					visitado[vecino] = true;
					aristaLlegada[vecino] = arista;
					cola.push_back(vecino);
				}
			}
		}
		return visitado[sumidero];
	}
};

/*
 * Convierte un numero de la peticion en indice de nodo.
 * Parametros:
 *   valor: numero leido del JSON.
 *   cantidad: numero de nodos.
 * Retorno: el indice; lanza una excepcion si no esta en 0 .. n - 1.
 */
inline std::size_t leerNodoFlujo(const ValorJson &valor, std::size_t cantidad) {
	if (valor.numero < 0 || valor.numero >= static_cast<double>(cantidad)) {
		throw std::runtime_error("nodo fuera de rango en el modo flow");
	}
	return static_cast<std::size_t>(valor.numero);
}

/*
 * Atiende el modo de flujo maximo.
 * Parametros:
 *   entrada: campo "input" con "n", "edges" ([{"u", "v", "capacity"}, ...]),
 *            "source" y "sink".
 * Retorno: objeto JSON {"flow": valor}.
 */
inline ValorJson resolverFlujoMaximo(const ValorJson &entrada) {
	std::size_t cantidad = static_cast<std::size_t>(entrada.obtenerCampo("n").numero);
	RedResidual red(cantidad);
	for (const ValorJson &arista : entrada.obtenerCampo("edges").elementos) {
		std::size_t origen = leerNodoFlujo(arista.obtenerCampo("u"), cantidad);
		std::size_t destino = leerNodoFlujo(arista.obtenerCampo("v"), cantidad);
		red.agregarArista(origen, destino, arista.obtenerCampo("capacity").numero);
	}
	std::size_t fuente = leerNodoFlujo(entrada.obtenerCampo("source"), cantidad);
	std::size_t sumidero = leerNodoFlujo(entrada.obtenerCampo("sink"), cantidad);
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("flow", crearNumero(red.calcularFlujoMaximo(fuente, sumidero)));
	return respuesta;
}

#endif
