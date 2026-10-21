/*
 * Descripcion: lectura y escritura de JSON para el contrato del autograder.
 * El ejecutable recibe un objeto JSON por la entrada estandar y responde
 * otro objeto JSON por la salida estandar; este archivo hace esa conversion.
 * Autores: Carlos Emiliano Nava Flores (A00842650),
 *          Luis Enrique Castillo Carrillo (A00842390),
 *          Desiderio Ivan Ortegon Morton (A00840591),
 *          Fernando Robles Vidrio (A00843290)
 * Fecha de creacion: 20/10/2026
 */

#ifndef JSON_H
#define JSON_H

#include <cmath>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class TipoJson { NULO, BOOLEANO, NUMERO, TEXTO, ARREGLO, OBJETO };

/*
 * Representa cualquier valor JSON: nulo, booleano, numero, texto,
 * arreglo u objeto. Los objetos guardan sus llaves y valores en dos
 * vectores paralelos, en el mismo orden en que se agregaron.
 */
class ValorJson {
public:
	TipoJson tipo = TipoJson::NULO;
	bool booleano = false;
	double numero = 0.0;
	std::string texto = "";
	std::vector<ValorJson> elementos;
	std::vector<std::string> llaves;
	std::vector<ValorJson> valores;

	/*
	 * Indica si el objeto contiene una llave.
	 * Parametros:
	 *   llave: nombre del campo a buscar.
	 * Retorno: true si el campo existe, false en caso contrario.
	 */
	bool tieneCampo(const std::string &llave) const {
		return buscarIndice(llave) < llaves.size();
	}

	/*
	 * Obtiene el valor de un campo del objeto.
	 * Parametros:
	 *   llave: nombre del campo a obtener.
	 * Retorno: referencia al valor del campo; lanza una excepcion si no existe.
	 */
	const ValorJson &obtenerCampo(const std::string &llave) const {
		std::size_t indice = buscarIndice(llave);
		if (indice >= llaves.size()) {
			throw std::runtime_error("falta el campo \"" + llave + "\"");
		}
		return valores[indice];
	}

	/*
	 * Agrega un campo al final del objeto.
	 * Parametros:
	 *   llave: nombre del campo.
	 *   valor: valor que se guarda en el campo.
	 * Retorno: ninguno.
	 */
	void agregarCampo(const std::string &llave, const ValorJson &valor) {
		llaves.push_back(llave);
		valores.push_back(valor);
	}

	/*
	 * Agrega un elemento al final del arreglo.
	 * Parametros:
	 *   valor: elemento que se agrega.
	 * Retorno: ninguno.
	 */
	void agregarElemento(const ValorJson &valor) {
		elementos.push_back(valor);
	}

private:
	/*
	 * Busca la posicion de una llave dentro del objeto.
	 * Parametros:
	 *   llave: nombre del campo a buscar.
	 * Retorno: posicion de la llave, o llaves.size() si no se encuentra.
	 */
	std::size_t buscarIndice(const std::string &llave) const {
		std::size_t indice = 0;
		bool encontrado = false;
		while (indice < llaves.size() && !encontrado) {
			if (llaves[indice] == llave) {
				encontrado = true;
			} else {
				indice++;
			}
		}
		return indice;
	}
};

/*
 * Lee el campo opcional "technique" de una peticion. Permite elegir la
 * tecnica alternativa de cada modo para los benchmarks; el autograder no
 * lo envia, asi que siempre se usa la tecnica principal.
 * Parametros:
 *   entrada: campo "input" de la peticion.
 *   porDefecto: nombre de la tecnica principal del modo.
 * Retorno: el nombre de la tecnica pedida, o porDefecto si no se indico.
 */
inline std::string leerTecnica(const ValorJson &entrada, const std::string &porDefecto) {
	std::string tecnica = porDefecto;
	if (entrada.tieneCampo("technique")) {
		tecnica = entrada.obtenerCampo("technique").texto;
	}
	return tecnica;
}

/*
 * Crea un valor JSON de tipo numero.
 * Parametros:
 *   numero: cantidad que se guarda.
 * Retorno: valor JSON con el numero.
 */
inline ValorJson crearNumero(double numero) {
	ValorJson valor;
	valor.tipo = TipoJson::NUMERO;
	valor.numero = numero;
	return valor;
}

/*
 * Crea un valor JSON de tipo texto.
 * Parametros:
 *   texto: cadena que se guarda.
 * Retorno: valor JSON con el texto.
 */
inline ValorJson crearTexto(const std::string &texto) {
	ValorJson valor;
	valor.tipo = TipoJson::TEXTO;
	valor.texto = texto;
	return valor;
}

/*
 * Crea un valor JSON de tipo booleano.
 * Parametros:
 *   booleano: valor logico que se guarda.
 * Retorno: valor JSON con el booleano.
 */
inline ValorJson crearBooleano(bool booleano) {
	ValorJson valor;
	valor.tipo = TipoJson::BOOLEANO;
	valor.booleano = booleano;
	return valor;
}

/*
 * Crea un arreglo JSON vacio.
 * Parametros: ninguno.
 * Retorno: valor JSON de tipo arreglo sin elementos.
 */
inline ValorJson crearArreglo() {
	ValorJson valor;
	valor.tipo = TipoJson::ARREGLO;
	return valor;
}

/*
 * Crea un objeto JSON vacio.
 * Parametros: ninguno.
 * Retorno: valor JSON de tipo objeto sin campos.
 */
inline ValorJson crearObjeto() {
	ValorJson valor;
	valor.tipo = TipoJson::OBJETO;
	return valor;
}

/*
 * Convierte texto JSON en un ValorJson recorriendolo caracter por caracter
 * (analizador descendente recursivo).
 */
class LectorJson {
public:
	/*
	 * Lee un documento completo y verifica que no sobre texto al final.
	 * Parametros:
	 *   textoEntrada: texto JSON recibido.
	 * Retorno: el valor JSON leido; lanza una excepcion si el texto es invalido.
	 */
	ValorJson leerDocumento(const std::string &textoEntrada) {
		texto = textoEntrada;
		posicion = 0;
		ValorJson valor = leerValor();
		saltarEspacios();
		if (posicion != texto.size()) {
			throw std::runtime_error("texto sobrante despues del JSON");
		}
		return valor;
	}

private:
	std::string texto = "";
	std::size_t posicion = 0;

	/*
	 * Avanza la posicion mientras haya espacios, tabuladores o saltos de linea.
	 * Parametros: ninguno.
	 * Retorno: ninguno.
	 */
	void saltarEspacios() {
		bool hayEspacio = true;
		while (posicion < texto.size() && hayEspacio) {
			char caracter = texto[posicion];
			hayEspacio = caracter == ' ' || caracter == '\t' || caracter == '\n' || caracter == '\r';
			if (hayEspacio) {
				posicion++;
			}
		}
	}

	/*
	 * Consulta el caracter en la posicion actual sin avanzar.
	 * Parametros: ninguno.
	 * Retorno: el caracter actual, o '\0' si ya se termino el texto.
	 */
	char caracterActual() const {
		char caracter = '\0';
		if (posicion < texto.size()) {
			caracter = texto[posicion];
		}
		return caracter;
	}

	/*
	 * Verifica que el caracter actual sea el esperado y avanza una posicion.
	 * Parametros:
	 *   esperado: caracter que debe aparecer.
	 * Retorno: ninguno; lanza una excepcion si el caracter no coincide.
	 */
	void esperarCaracter(char esperado) {
		if (caracterActual() != esperado) {
			throw std::runtime_error(std::string("se esperaba '") + esperado + "'");
		}
		posicion++;
	}

	/*
	 * Lee el siguiente valor segun su primer caracter.
	 * Parametros: ninguno.
	 * Retorno: el valor JSON leido.
	 */
	ValorJson leerValor() {
		ValorJson valor;
		saltarEspacios();
		char caracter = caracterActual();
		if (caracter == '{') {
			valor = leerObjeto();
		} else if (caracter == '[') {
			valor = leerArreglo();
		} else if (caracter == '"') {
			valor = crearTexto(leerTexto());
		} else if (caracter == '-' || (caracter >= '0' && caracter <= '9')) {
			valor = leerNumero();
		} else {
			valor = leerLiteral();
		}
		return valor;
	}

	/*
	 * Lee un objeto de la forma {"llave": valor, ...}.
	 * Parametros: ninguno.
	 * Retorno: el objeto JSON leido.
	 */
	ValorJson leerObjeto() {
		ValorJson objeto = crearObjeto();
		esperarCaracter('{');
		saltarEspacios();
		bool terminado = caracterActual() == '}';
		while (!terminado) {
			saltarEspacios();
			std::string llave = leerTexto();
			saltarEspacios();
			esperarCaracter(':');
			objeto.agregarCampo(llave, leerValor());
			saltarEspacios();
			if (caracterActual() == ',') {
				posicion++;
			} else {
				terminado = true;
			}
		}
		esperarCaracter('}');
		return objeto;
	}

	/*
	 * Lee un arreglo de la forma [valor, valor, ...].
	 * Parametros: ninguno.
	 * Retorno: el arreglo JSON leido.
	 */
	ValorJson leerArreglo() {
		ValorJson arreglo = crearArreglo();
		esperarCaracter('[');
		saltarEspacios();
		bool terminado = caracterActual() == ']';
		while (!terminado) {
			arreglo.agregarElemento(leerValor());
			saltarEspacios();
			if (caracterActual() == ',') {
				posicion++;
			} else {
				terminado = true;
			}
		}
		esperarCaracter(']');
		return arreglo;
	}

	/*
	 * Lee una cadena entre comillas y resuelve sus secuencias de escape.
	 * Parametros: ninguno.
	 * Retorno: el contenido de la cadena sin comillas.
	 */
	std::string leerTexto() {
		std::string resultado = "";
		esperarCaracter('"');
		bool terminado = false;
		while (!terminado) {
			char caracter = caracterActual();
			if (caracter == '\0') {
				throw std::runtime_error("cadena sin cerrar");
			}
			posicion++;
			if (caracter == '"') {
				terminado = true;
			} else if (caracter == '\\') {
				resultado += leerEscape();
			} else {
				resultado += caracter;
			}
		}
		return resultado;
	}

	/*
	 * Traduce la secuencia de escape que sigue a una diagonal invertida.
	 * Parametros: ninguno.
	 * Retorno: el texto que representa la secuencia.
	 */
	std::string leerEscape() {
		char codigo = caracterActual();
		std::string resultado = "";
		posicion++;
		if (codigo == 'n') {
			resultado = "\n";
		} else if (codigo == 't') {
			resultado = "\t";
		} else if (codigo == 'r') {
			resultado = "\r";
		} else if (codigo == 'b') {
			resultado = "\b";
		} else if (codigo == 'f') {
			resultado = "\f";
		} else if (codigo == 'u') {
			resultado = leerUnicode();
		} else {
			resultado = std::string(1, codigo);
		}
		return resultado;
	}

	/*
	 * Lee los cuatro digitos hexadecimales de un escape \uXXXX y los
	 * convierte a la codificacion UTF-8 de la cadena.
	 * Parametros: ninguno.
	 * Retorno: el caracter ya codificado.
	 */
	std::string leerUnicode() {
		const std::size_t DIGITOS_UNICODE = 4;
		if (posicion + DIGITOS_UNICODE > texto.size()) {
			throw std::runtime_error("escape unicode incompleto");
		}
		unsigned int codigo = static_cast<unsigned int>(std::stoul(texto.substr(posicion, DIGITOS_UNICODE), nullptr, 16));
		posicion += DIGITOS_UNICODE;
		std::string resultado = "";
		if (codigo < 0x80) {
			resultado += static_cast<char>(codigo);
		} else if (codigo < 0x800) {
			resultado += static_cast<char>(0xC0 | (codigo >> 6));
			resultado += static_cast<char>(0x80 | (codigo & 0x3F));
		} else {
			resultado += static_cast<char>(0xE0 | (codigo >> 12));
			resultado += static_cast<char>(0x80 | ((codigo >> 6) & 0x3F));
			resultado += static_cast<char>(0x80 | (codigo & 0x3F));
		}
		return resultado;
	}

	/*
	 * Lee un numero entero o decimal, con signo y exponente opcionales.
	 * Parametros: ninguno.
	 * Retorno: valor JSON de tipo numero.
	 */
	ValorJson leerNumero() {
		const std::string CARACTERES_NUMERO = "+-0123456789eE.";
		std::size_t inicio = posicion;
		while (posicion < texto.size() && CARACTERES_NUMERO.find(texto[posicion]) != std::string::npos) {
			posicion++;
		}
		return crearNumero(std::stod(texto.substr(inicio, posicion - inicio)));
	}

	/*
	 * Lee una de las palabras fijas de JSON: true, false o null.
	 * Parametros: ninguno.
	 * Retorno: el valor JSON correspondiente; lanza una excepcion si no coincide.
	 */
	ValorJson leerLiteral() {
		ValorJson valor;
		if (texto.compare(posicion, 4, "true") == 0) {
			valor = crearBooleano(true);
			posicion += 4;
		} else if (texto.compare(posicion, 5, "false") == 0) {
			valor = crearBooleano(false);
			posicion += 5;
		} else if (texto.compare(posicion, 4, "null") == 0) {
			posicion += 4;
		} else {
			throw std::runtime_error("valor JSON no reconocido");
		}
		return valor;
	}
};

/*
 * Convierte una cadena a su forma JSON, con comillas y escapes.
 * Parametros:
 *   texto: cadena original.
 * Retorno: la cadena lista para escribirse en JSON.
 */
inline std::string escribirTextoJson(const std::string &texto) {
	std::string resultado = "\"";
	for (char caracter : texto) {
		if (caracter == '"') {
			resultado += "\\\"";
		} else if (caracter == '\\') {
			resultado += "\\\\";
		} else if (caracter == '\n') {
			resultado += "\\n";
		} else if (caracter == '\t') {
			resultado += "\\t";
		} else if (caracter == '\r') {
			resultado += "\\r";
		} else {
			resultado += caracter;
		}
	}
	return resultado + "\"";
}

/*
 * Convierte un numero a texto: sin decimales si es entero, y con hasta
 * 15 cifras significativas si tiene parte decimal.
 * Parametros:
 *   numero: cantidad a escribir.
 * Retorno: el numero como texto.
 */
inline std::string escribirNumeroJson(double numero) {
	const double LIMITE_ENTERO = 1.0e15;
	const int CIFRAS_SIGNIFICATIVAS = 15;
	std::ostringstream salida;
	if (std::floor(numero) == numero && std::fabs(numero) < LIMITE_ENTERO) {
		salida << static_cast<long long>(numero);
	} else {
		salida.precision(CIFRAS_SIGNIFICATIVAS);
		salida << numero;
	}
	return salida.str();
}

/*
 * Convierte un ValorJson completo a texto JSON en una sola linea.
 * Parametros:
 *   valor: valor que se desea escribir.
 * Retorno: el texto JSON equivalente.
 */
inline std::string escribirJson(const ValorJson &valor) {
	std::string resultado = "null";
	if (valor.tipo == TipoJson::BOOLEANO) {
		resultado = valor.booleano ? "true" : "false";
	} else if (valor.tipo == TipoJson::NUMERO) {
		resultado = escribirNumeroJson(valor.numero);
	} else if (valor.tipo == TipoJson::TEXTO) {
		resultado = escribirTextoJson(valor.texto);
	} else if (valor.tipo == TipoJson::ARREGLO) {
		resultado = "[";
		for (std::size_t indice = 0; indice < valor.elementos.size(); indice++) {
			if (indice > 0) {
				resultado += ",";
			}
			resultado += escribirJson(valor.elementos[indice]);
		}
		resultado += "]";
	} else if (valor.tipo == TipoJson::OBJETO) {
		resultado = "{";
		for (std::size_t indice = 0; indice < valor.llaves.size(); indice++) {
			if (indice > 0) {
				resultado += ",";
			}
			resultado += escribirTextoJson(valor.llaves[indice]) + ":" + escribirJson(valor.valores[indice]);
		}
		resultado += "}";
	}
	return resultado;
}

#endif
