/*
 * Descripcion: desafio 01 · Optimal Network Deployment (arbol de expansion minima).
 * Conecta todas las centrales al menor costo de instalacion.
 * Usa Kruskal: ordena las aristas por peso y agrega cada una si une dos
 * componentes distintas (conjuntos disjuntos con compresion de caminos y
 * union por rango). Tiempo O(E log E), memoria O(V + E).
 * Alternativa para comparar: Prim con monticulo binario, O(E log V).
 * Equipo: Sentinela del Norte (Equipo 8)
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef MST_H
#define MST_H

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.h"

/*
 * Arista no dirigida con su peso.
 */
struct AristaMst {
	std::size_t origen = 0;
	std::size_t destino = 0;
	double peso = 0.0;
};

/*
 * Conjuntos disjuntos (union-find) con compresion de caminos y union por rango.
 */
class ConjuntosDisjuntos {
public:
	/*
	 * Crea un conjunto por cada elemento.
	 * Parametros:
	 *   cantidad: numero de elementos.
	 */
	explicit ConjuntosDisjuntos(std::size_t cantidad) : padre(cantidad), rango(cantidad, 0) {
		for (std::size_t indice = 0; indice < cantidad; indice++) {
			padre[indice] = indice;
		}
	}

	/*
	 * Busca el representante del conjunto de un elemento y acorta el camino.
	 * Parametros:
	 *   elemento: elemento a buscar.
	 * Retorno: el representante de su conjunto.
	 */
	std::size_t buscar(std::size_t elemento) {
		std::size_t raiz = elemento;
		while (padre[raiz] != raiz) {
			raiz = padre[raiz];
		}
		while (padre[elemento] != raiz) {
			std::size_t siguiente = padre[elemento];
			padre[elemento] = raiz;
			elemento = siguiente;
		}
		return raiz;
	}

	/*
	 * Une los conjuntos de dos elementos.
	 * Parametros:
	 *   primero, segundo: elementos a unir.
	 * Retorno: true si estaban en conjuntos distintos.
	 */
	bool unir(std::size_t primero, std::size_t segundo) {
		std::size_t raizPrimero = buscar(primero);
		std::size_t raizSegundo = buscar(segundo);
		bool distintos = raizPrimero != raizSegundo;
		if (distintos) {
			if (rango[raizPrimero] < rango[raizSegundo]) {
				std::swap(raizPrimero, raizSegundo);
			}
			padre[raizSegundo] = raizPrimero;
			if (rango[raizPrimero] == rango[raizSegundo]) {
				rango[raizPrimero]++;
			}
		}
		return distintos;
	}

private:
	std::vector<std::size_t> padre;
	std::vector<int> rango;
};

/*
 * Lee las aristas de la peticion y verifica que sus nodos existan.
 * Parametros:
 *   entrada: campo "input" con "edges" ([{"u", "v", "weight"}, ...]).
 *   cantidadNodos: numero de nodos (0 .. n - 1).
 * Retorno: las aristas leidas; lanza una excepcion si un nodo no existe.
 */
inline std::vector<AristaMst> leerAristasMst(const ValorJson &entrada, std::size_t cantidadNodos) {
	std::vector<AristaMst> aristas;
	for (const ValorJson &aristaJson : entrada.obtenerCampo("edges").elementos) {
		double origen = aristaJson.obtenerCampo("u").numero;
		double destino = aristaJson.obtenerCampo("v").numero;
		bool fueraDeRango = origen < 0 || destino < 0 || origen >= static_cast<double>(cantidadNodos) || destino >= static_cast<double>(cantidadNodos);
		if (fueraDeRango) {
			throw std::runtime_error("arista con un nodo inexistente");
		}
		AristaMst arista;
		arista.origen = static_cast<std::size_t>(origen);
		arista.destino = static_cast<std::size_t>(destino);
		arista.peso = aristaJson.obtenerCampo("weight").numero;
		aristas.push_back(arista);
	}
	return aristas;
}

/*
 * Calcula el arbol de expansion minima con Kruskal. Si el grafo no es
 * conexo, devuelve el bosque minimo (menos de n - 1 aristas).
 * Parametros:
 *   cantidadNodos: numero de nodos.
 *   aristas: aristas del grafo.
 * Retorno: las aristas elegidas, en el orden en que se agregaron.
 */
inline std::vector<AristaMst> calcularKruskal(std::size_t cantidadNodos, std::vector<AristaMst> aristas) {
	std::stable_sort(aristas.begin(), aristas.end(), [](const AristaMst &izquierda, const AristaMst &derecha) {
		return izquierda.peso < derecha.peso;
	});
	ConjuntosDisjuntos conjuntos(cantidadNodos);
	std::vector<AristaMst> arbol;
	std::size_t indice = 0;
	while (indice < aristas.size() && arbol.size() + 1 < cantidadNodos) {
		if (conjuntos.unir(aristas[indice].origen, aristas[indice].destino)) {
			arbol.push_back(aristas[indice]);
		}
		indice++;
	}
	return arbol;
}

/*
 * Calcula el arbol de expansion minima con Prim y un monticulo binario:
 * crece un solo arbol desde el nodo 0 tomando siempre la arista mas barata
 * que sale de el. O(E log V). Si el grafo no es conexo, solo cubre la
 * componente del nodo 0.
 * Parametros:
 *   cantidadNodos: numero de nodos.
 *   aristas: aristas del grafo.
 * Retorno: las aristas elegidas, en el orden en que se agregaron.
 */
inline std::vector<AristaMst> calcularPrim(std::size_t cantidadNodos, const std::vector<AristaMst> &aristas) {
	std::vector<std::vector<AristaMst>> vecinos(cantidadNodos);
	for (const AristaMst &arista : aristas) {
		vecinos[arista.origen].push_back(arista);
		AristaMst reversa = arista;
		std::swap(reversa.origen, reversa.destino);
		vecinos[arista.destino].push_back(reversa);
	}

	std::vector<AristaMst> candidatas;
	std::vector<std::pair<double, std::size_t>> monticulo;
	auto mayorPrimero = [](const std::pair<double, std::size_t> &izquierda, const std::pair<double, std::size_t> &derecha) {
		return izquierda > derecha;
	};
	std::vector<bool> enArbol(cantidadNodos, false);
	std::vector<AristaMst> arbol;
	if (cantidadNodos > 0) {
		enArbol[0] = true;
		for (const AristaMst &arista : vecinos[0]) {
			candidatas.push_back(arista);
			monticulo.push_back(std::make_pair(arista.peso, candidatas.size() - 1));
			std::push_heap(monticulo.begin(), monticulo.end(), mayorPrimero);
		}
	}
	while (!monticulo.empty() && arbol.size() + 1 < cantidadNodos) {
		std::pop_heap(monticulo.begin(), monticulo.end(), mayorPrimero);
		AristaMst elegida = candidatas[monticulo.back().second];
		monticulo.pop_back();
		if (!enArbol[elegida.destino]) {
			enArbol[elegida.destino] = true;
			arbol.push_back(elegida);
			for (const AristaMst &arista : vecinos[elegida.destino]) {
				if (!enArbol[arista.destino]) {
					candidatas.push_back(arista);
					monticulo.push_back(std::make_pair(arista.peso, candidatas.size() - 1));
					std::push_heap(monticulo.begin(), monticulo.end(), mayorPrimero);
				}
			}
		}
	}
	return arbol;
}

/*
 * Atiende el modo "mst".
 * Parametros:
 *   entrada: campo "input" con "nodes", "edges" y, opcional,
 *            "technique": "kruskal" (por defecto) o "prim".
 * Retorno: objeto JSON {"cost": costo, "edges": [[u, v, peso], ...]}.
 */
inline ValorJson resolverMst(const ValorJson &entrada) {
	std::size_t cantidadNodos = entrada.obtenerCampo("nodes").elementos.size();
	std::vector<AristaMst> aristasEntrada = leerAristasMst(entrada, cantidadNodos);
	std::string tecnica = leerTecnica(entrada, "kruskal");
	std::vector<AristaMst> arbol;
	if (tecnica == "kruskal") {
		arbol = calcularKruskal(cantidadNodos, aristasEntrada);
	} else if (tecnica == "prim") {
		arbol = calcularPrim(cantidadNodos, aristasEntrada);
	} else {
		throw std::runtime_error("tecnica de mst desconocida: " + tecnica);
	}
	double costo = 0.0;
	ValorJson aristasJson = crearArreglo();
	for (const AristaMst &arista : arbol) {
		costo += arista.peso;
		ValorJson aristaJson = crearArreglo();
		aristaJson.agregarElemento(crearNumero(static_cast<double>(arista.origen)));
		aristaJson.agregarElemento(crearNumero(static_cast<double>(arista.destino)));
		aristaJson.agregarElemento(crearNumero(arista.peso));
		aristasJson.agregarElemento(aristaJson);
	}
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("cost", crearNumero(costo));
	respuesta.agregarCampo("edges", aristasJson);
	return respuesta;
}

#endif
