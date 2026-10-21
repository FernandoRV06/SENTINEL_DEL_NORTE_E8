/*
 * Descripcion: desafio 02 (Optimal Maintenance Route): problema del viajante.
 * Visita cada central una vez, empezando y terminando en la central 0,
 * con la menor distancia total segun la matriz de distancias recibida.
 * Usa Held-Karp (exacto, hasta LIMITE_EXACTO_TSP centrales) y vecino mas
 * cercano mejorado con 2-opt (aproximado, para instancias grandes).
 * El costo se mide sobre la matriz tal cual: el autograder suma
 * distances[ruta[i]][ruta[i + 1]], asi que no se acortan caminos con
 * centrales intermedias (Floyd-Warshall) porque el costo ya no coincidiria.
 */

#ifndef TSP_H
#define TSP_H

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "json.h"

// Mayor numero de centrales que se resuelve de forma exacta: Held-Karp usa
// O(2^n * n) memoria y O(2^n * n^2) tiempo (16 centrales: ~16 millones de pasos).
const std::size_t LIMITE_EXACTO_TSP = 16;
// Mejora minima para aceptar un cambio 2-opt (evita ciclos por redondeo).
const double TOLERANCIA_TSP = 1e-9;
// Tope de pasadas completas de 2-opt para acotar el tiempo en casos grandes.
const int MAXIMO_PASADAS_2OPT = 1000;

/*
 * Lee la matriz de distancias y verifica que sea cuadrada.
 * Parametros:
 *   entrada: campo "input" de la peticion, con el campo "distances".
 * Retorno: la matriz como vector de vectores; lanza una excepcion si no es cuadrada.
 */
inline std::vector<std::vector<double>> leerDistanciasTsp(const ValorJson &entrada) {
	const ValorJson &filas = entrada.obtenerCampo("distances");
	std::size_t cantidad = filas.elementos.size();
	std::vector<std::vector<double>> distancias(cantidad, std::vector<double>(cantidad, 0.0));
	for (std::size_t origen = 0; origen < cantidad; origen++) {
		if (filas.elementos[origen].elementos.size() != cantidad) {
			throw std::runtime_error("la matriz de distancias no es cuadrada");
		}
		for (std::size_t destino = 0; destino < cantidad; destino++) {
			distancias[origen][destino] = filas.elementos[origen].elementos[destino].numero;
		}
	}
	return distancias;
}

/*
 * Suma la distancia de una ruta cerrada (el ultimo nodo regresa al primero).
 * Parametros:
 *   distancias: matriz de distancias.
 *   ruta: orden de visita sin repetir el origen al final.
 * Retorno: la distancia total del ciclo.
 */
inline double calcularCostoRutaTsp(const std::vector<std::vector<double>> &distancias, const std::vector<std::size_t> &ruta) {
	double costo = 0.0;
	for (std::size_t posicion = 0; posicion < ruta.size(); posicion++) {
		costo += distancias[ruta[posicion]][ruta[(posicion + 1) % ruta.size()]];
	}
	return costo;
}

/*
 * Resuelve el viajante de forma exacta con programacion dinamica (Held-Karp).
 * costo[mascara * n + ultimo] es la menor distancia para salir de 0, visitar
 * exactamente las centrales de la mascara y quedar en "ultimo".
 * Parametros:
 *   distancias: matriz de distancias (al menos 2 centrales).
 * Retorno: el orden de visita optimo, empezando en 0 y sin repetirlo al final.
 */
inline std::vector<std::size_t> resolverTspExacto(const std::vector<std::vector<double>> &distancias) {
	const double INFINITO = std::numeric_limits<double>::infinity();
	std::size_t cantidad = distancias.size();
	std::size_t totalMascaras = static_cast<std::size_t>(1) << cantidad;
	std::vector<double> costo(totalMascaras * cantidad, INFINITO);
	std::vector<std::size_t> anterior(totalMascaras * cantidad, 0);
	costo[1 * cantidad + 0] = 0.0;
	// Solo las mascaras impares contienen al origen; crecen en orden, asi que
	// cada estado ya esta resuelto cuando se expande.
	for (std::size_t mascara = 1; mascara < totalMascaras; mascara += 2) {
		for (std::size_t ultimo = 0; ultimo < cantidad; ultimo++) {
			double actual = costo[mascara * cantidad + ultimo];
			if (actual != INFINITO) {
				for (std::size_t siguiente = 1; siguiente < cantidad; siguiente++) {
					std::size_t bit = static_cast<std::size_t>(1) << siguiente;
					if ((mascara & bit) == 0) {
						std::size_t estado = (mascara | bit) * cantidad + siguiente;
						double candidato = actual + distancias[ultimo][siguiente];
						if (candidato < costo[estado]) {
							costo[estado] = candidato;
							anterior[estado] = ultimo;
						}
					}
				}
			}
		}
	}
	std::size_t completa = totalMascaras - 1;
	std::size_t mejorUltimo = 1;
	for (std::size_t ultimo = 2; ultimo < cantidad; ultimo++) {
		double candidato = costo[completa * cantidad + ultimo] + distancias[ultimo][0];
		double mejor = costo[completa * cantidad + mejorUltimo] + distancias[mejorUltimo][0];
		if (candidato < mejor) {
			mejorUltimo = ultimo;
		}
	}
	// Se reconstruye la ruta de atras hacia adelante siguiendo "anterior".
	std::vector<std::size_t> ruta;
	std::size_t mascara = completa;
	std::size_t nodo = mejorUltimo;
	while (nodo != 0) {
		ruta.push_back(nodo);
		std::size_t previo = anterior[mascara * cantidad + nodo];
		mascara ^= static_cast<std::size_t>(1) << nodo;
		nodo = previo;
	}
	ruta.push_back(0);
	std::reverse(ruta.begin(), ruta.end());
	return ruta;
}

/*
 * Construye una ruta con la heuristica del vecino mas cercano: desde la
 * central actual se va a la central no visitada mas cercana. O(n^2).
 * Parametros:
 *   distancias: matriz de distancias.
 * Retorno: el orden de visita, empezando en 0 y sin repetirlo al final.
 */
inline std::vector<std::size_t> construirVecinoMasCercano(const std::vector<std::vector<double>> &distancias) {
	std::size_t cantidad = distancias.size();
	std::vector<bool> visitado(cantidad, false);
	std::vector<std::size_t> ruta;
	std::size_t actual = 0;
	visitado[0] = true;
	ruta.push_back(0);
	while (ruta.size() < cantidad) {
		std::size_t elegido = cantidad;
		for (std::size_t candidato = 0; candidato < cantidad; candidato++) {
			bool esMejor = elegido == cantidad || distancias[actual][candidato] < distancias[actual][elegido];
			if (!visitado[candidato] && esMejor) {
				elegido = candidato;
			}
		}
		visitado[elegido] = true;
		ruta.push_back(elegido);
		actual = elegido;
	}
	return ruta;
}

/*
 * Indica si la matriz es simetrica; 2-opt invierte tramos de la ruta y solo
 * conserva el costo de esos tramos cuando d[a][b] == d[b][a].
 * Parametros:
 *   distancias: matriz de distancias.
 * Retorno: true si la matriz es simetrica.
 */
inline bool esMatrizSimetrica(const std::vector<std::vector<double>> &distancias) {
	bool simetrica = true;
	for (std::size_t origen = 0; origen < distancias.size(); origen++) {
		for (std::size_t destino = origen + 1; destino < distancias.size(); destino++) {
			if (distancias[origen][destino] != distancias[destino][origen]) {
				simetrica = false;
			}
		}
	}
	return simetrica;
}

/*
 * Mejora una ruta con 2-opt: cambia dos aristas (a,b) y (c,d) por (a,c) y
 * (b,d) invirtiendo el tramo intermedio, mientras eso reduzca la distancia.
 * La posicion 0 nunca se mueve, asi que la ruta sigue empezando en 0.
 * Parametros:
 *   distancias: matriz de distancias simetrica.
 *   ruta: orden de visita que se modifica en el lugar.
 * Retorno: ninguno.
 */
inline void mejorarConDosOpt(const std::vector<std::vector<double>> &distancias, std::vector<std::size_t> &ruta) {
	std::size_t cantidad = ruta.size();
	bool huboMejora = true;
	int pasadas = 0;
	while (huboMejora && pasadas < MAXIMO_PASADAS_2OPT) {
		huboMejora = false;
		pasadas++;
		for (std::size_t primero = 0; primero + 2 < cantidad; primero++) {
			// Si primero == 0, la ultima arista comparte el nodo 0 y no se cruza.
			std::size_t limite = primero == 0 ? cantidad - 1 : cantidad;
			for (std::size_t segundo = primero + 2; segundo < limite; segundo++) {
				std::size_t a = ruta[primero];
				std::size_t b = ruta[primero + 1];
				std::size_t c = ruta[segundo];
				std::size_t d = ruta[(segundo + 1) % cantidad];
				double cambio = distancias[a][c] + distancias[b][d] - distancias[a][b] - distancias[c][d];
				if (cambio < -TOLERANCIA_TSP) {
					std::reverse(ruta.begin() + static_cast<std::ptrdiff_t>(primero + 1), ruta.begin() + static_cast<std::ptrdiff_t>(segundo + 1));
					huboMejora = true;
				}
			}
		}
	}
}

/*
 * Resuelve el viajante de forma aproximada: vecino mas cercano y, si la
 * matriz es simetrica, mejora local con 2-opt.
 * Parametros:
 *   distancias: matriz de distancias.
 * Retorno: el orden de visita, empezando en 0 y sin repetirlo al final.
 */
inline std::vector<std::size_t> resolverTspAproximado(const std::vector<std::vector<double>> &distancias) {
	std::vector<std::size_t> ruta = construirVecinoMasCercano(distancias);
	if (esMatrizSimetrica(distancias)) {
		mejorarConDosOpt(distancias, ruta);
	}
	return ruta;
}

/*
 * Atiende el modo "tsp": elige el metodo exacto para instancias chicas y el
 * aproximado para las grandes.
 * Parametros:
 *   entrada: campo "input" de la peticion, con la matriz "distances".
 * Retorno: objeto JSON {"cost": distancia, "route": [0, ..., 0]}.
 */
inline ValorJson resolverTsp(const ValorJson &entrada) {
	std::vector<std::vector<double>> distancias = leerDistanciasTsp(entrada);
	// Sin centrales la ruta queda como [0] (n + 1 = 1 nodo); con una, [0, 0].
	std::vector<std::size_t> ruta;
	if (distancias.size() > LIMITE_EXACTO_TSP) {
		ruta = resolverTspAproximado(distancias);
	} else if (distancias.size() >= 2) {
		ruta = resolverTspExacto(distancias);
	} else if (distancias.size() == 1) {
		ruta.push_back(0);
	}
	double costo = calcularCostoRutaTsp(distancias, ruta);
	ValorJson rutaJson = crearArreglo();
	for (std::size_t nodo : ruta) {
		rutaJson.agregarElemento(crearNumero(static_cast<double>(nodo)));
	}
	rutaJson.agregarElemento(crearNumero(0.0));
	ValorJson respuesta = crearObjeto();
	respuesta.agregarCampo("cost", crearNumero(costo));
	respuesta.agregarCampo("route", rutaJson);
	return respuesta;
}

#endif
