# SENTINEL Digital Twin · Sentinela del Norte

Hackathon integrador de TC2038 (Análisis y Diseño de Algoritmos Avanzados). Plataforma que integra 7 algoritmos de infraestructura de red y ciberseguridad sobre la red GÉANT y transmisiones derivadas de CIC-IDS2017.

## Estructura

```
src/        Núcleo en C++: un único main.cpp más un .h por desafío
app/        Aplicación web SENTINEL Digital Twin (servidor en Python y página)
tests/      Casos de prueba
results/    Mediciones de los benchmarks
```

| Archivo | Contenido | Técnica principal | Alternativa (`technique`) |
| --- | --- | --- | --- |
| `src/main.cpp` | Lee la petición JSON, la envía al módulo del modo y escribe la respuesta | | |
| `src/json.h` | Lectura y escritura de JSON (sin librerías externas) | | |
| `src/mst.h` | Desafío 01 · árbol de expansión mínima | Kruskal (`kruskal`) | Prim con montículo (`prim`) |
| `src/tsp.h` | Desafío 02 · ruta de mantenimiento | Held-Karp exacto hasta 16 nodos, si no vecino más cercano + 2-opt (`auto`) | `held-karp`, `nearest-neighbor`, `2-opt` |
| `src/flujoMaximo.h` | Desafío 03 · flujo máximo | Edmonds-Karp (`edmonds-karp`) | Ford-Fulkerson con DFS (`ford-fulkerson`) |
| `src/voronoi.h` | Desafío 04 · regiones de Voronoi | Recorte por semiplanos con poda (`half-plane`) | Sin poda (`half-plane-naive`); Fortune solo se analiza |
| `src/transmision.h` | Limpieza y lectura de transmisiones (compartido por 05, 06 y 07) | | |
| `src/firmas.h` | Desafío 05 · detección de firmas | KMP (`kmp`) | Búsqueda ingenua (`naive`) |
| `src/palindromo.h` | Desafío 06 · palíndromo más largo | Manacher (`manacher`) | Expansión desde el centro (`center`) |
| `src/subcadenaComun.h` | Desafío 07 · subcadena común más larga | Arreglo de sufijos + LCP de Kasai (`suffix-array`) | Programación dinámica (`dp`) |

## Compilación

Mismas opciones que el autograder:

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
```

(Con Clang: cambiar `g++` por `clang++`.)

## Ejecución

El ejecutable lee un objeto JSON por la entrada estándar y escribe un objeto JSON por la salida estándar:

```
./sentinel < tests/peticiones/mst.json
```

Los modos son `mst`, `tsp`, `flow`, `voronoi`, `matching`, `palindrome` y `common`. En `tests/peticiones/` hay una petición de ejemplo por modo y en `tests/esperados.md` su respuesta esperada.

Dentro de `input` se puede agregar el campo opcional `"technique"` para usar la técnica alternativa de la tabla anterior (sirve para los benchmarks). Si no se envía, se usa la técnica principal, que es lo que recibe el autograder:

```
echo '{"mode": "mst", "input": {"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 1, "weight": 4}, {"u": 1, "v": 2, "weight": 1}], "technique": "prim"}}' | ./sentinel
```

## Aplicación

```
python3 app/servidor.py
```

Abre `http://localhost:8000` en el navegador. Si `sentinel` no existe o es más viejo que `src/`, el servidor lo compila primero con las mismas opciones del autograder. Requiere internet solo para el mapa de fondo de OpenStreetMap; sin conexión, la red y los resultados se dibujan sobre fondo gris.

| Vista | Qué hace |
| --- | --- |
| Network Explorer | Mapa de la red. Se elige un algoritmo a la vez (MST, ruta TSP, flujo por enlace o regiones de Voronoi) y el panel explica qué se ve y da su resultado. Con Voronoi, un clic en el mapa ubica a un cliente nuevo en su región. |
| Algorithm Laboratory | Corre todas las técnicas de uno de los siete problemas sobre los mismos datos (la red del escenario o una instancia aleatoria de tamaño `n`) y compara su tiempo y su resultado. El JSON enviado y recibido se puede consultar. |
| Cybersecurity Console | Busca las firmas, el palíndromo y la subcadena común en las transmisiones (escritas o cargadas desde `.txt`), explica los resultados, los resalta y muestra la salida en el formato de texto del caso. Indica la asociación simulada de `mapping.csv`. |
| Scenario Manager | Cambia de escenario, elige el origen y destino del flujo, simula la caída de enlaces, y carga o descarga escenarios en `.json`. |

Todos los resultados vienen del núcleo en C++ en el momento: la página no guarda resultados precalculados.

## Pruebas

Requieren Python 3 (solo biblioteca estándar) y el ejecutable ya compilado:

```
python3 tests/probarTspVoronoi.py      # TSP y Voronoi
python3 tests/probarModulos.py         # MST, flujo, firmas, palíndromo y subcadena común
```

Cada script compara las respuestas contra oráculos escritos aparte en Python e incluye casos borde, casos aleatorios con el formato de la página, pruebas de estrés de los perfiles `lite`, `normal` y `huge`, y las técnicas alternativas. Se puede pasar una semilla como argumento para cambiar los casos aleatorios. Detalle de los casos en `tests/esperados.md`.

## Benchmarks

```
python3 results/benchmarks.py           # 3 repeticiones por medición (unos 3 minutos)
python3 results/benchmarks.py --rapido  # 1 repetición
```

Genera instancias chicas, medianas y grandes con semilla fija, corre todas las técnicas sobre la misma instancia y escribe `results/benchmarks.csv` y `results/benchmarks.md` con tiempo, memoria, exponente empírico contra la complejidad teórica, coincidencia entre técnicas y los datos del entorno (procesador, sistema, compilador y banderas).

## Arquitectura

Núcleo en C++ (`src/`):

```
stdin (JSON) -> main.cpp -> LectorJson (json.h) -> despacharModo -> resolverX() del módulo -> escribirJson -> stdout (JSON)
```

- `main.cpp` solo lee la petición, elige el módulo por el campo `mode` y escribe la respuesta; si la entrada es inválida responde `{"error": "..."}`.
- Cada desafío vive en su propio `.h` con una función `resolverX(const ValorJson &entrada)` que lee su campo `input`, elige la técnica y arma la respuesta. Los tres desafíos de cadenas comparten `transmision.h` para normalizar las transmisiones.
- No hay estado global: cada ejecución atiende una sola petición, así que la aplicación puede llamar al ejecutable por cada consulta.

Capa de datos: los archivos del caso (sección [Datos](#datos)) se convierten en peticiones JSON con el formato de cada modo. El núcleo no lee archivos: todo lo recibe en `input`, de modo que el autograder, las pruebas y la aplicación lo usan exactamente igual.

Aplicación (`app/`):

```
navegador (index.html + js/) --fetch POST /api/ejecutar--> servidor.py --stdin JSON--> ./sentinel --stdout JSON--> servidor.py --> navegador
```

- `app/servidor.py` sirve los archivos de la página y, por cada consulta, ejecuta `./sentinel` con la petición; mide el tiempo y devuelve la respuesta. No calcula nada por su cuenta.
- `app/js/api.js` convierte el escenario activo en la entrada de cada modo. Para el TSP calcula antes la matriz de distancias mínimas entre centrales siguiendo los enlaces (Floyd-Warshall, 14 nodos), porque no todas tienen enlace directo; así la ruta se dibuja por los enlaces reales.
- `app/js/explorador.js`, `laboratorio.js`, `consola.js` y `escenarios.js` implementan una vista cada uno; `principal.js` las conecta.
- `app/datos/red.json` es la red de demostración y `app/datos/escenarios.json` los escenarios iniciales.
- El modo `flow` acepta el campo opcional `"detail": true`, que agrega `"edgeFlows"` (flujo de cada arista) para dibujar el flujo por enlace; sin ese campo responde igual que siempre.

## Dependencias

Núcleo: solo la biblioteca estándar de C++17. El autograder de la página compila sin soporte de excepciones, por eso convierte cada `throw` en un `abort` y elimina los `try` y `catch`. El código solo lanza errores ante entradas inválidas, y no debe escribir esas palabras en comentarios. Pruebas, benchmarks y servidor de la aplicación: Python 3 con su biblioteca estándar. Página: [Leaflet](https://leafletjs.com) 1.9.4 (licencia BSD-2) incluida en `app/vendor/leaflet/`, y el mapa de fondo de [OpenStreetMap](https://www.openstreetmap.org/copyright).

## Datos

El caso combina dos fuentes públicas que provienen de **entornos independientes**:

| Fuente | Qué aporta | Archivos del caso | Módulo |
| --- | --- | --- | --- |
| [SNDlib](https://sndlib.put.poznan.pl/networks.overview.action), red GÉANT | Topología, coordenadas, costos, capacidades y demandas de la red troncal europea | `nodes.csv`, `edges.csv`, `capacities.csv`, `demands.csv`, `scenarios.json` | I · infraestructura |
| [CIC-IDS2017](https://www.unb.ca/cic/datasets/ids-2017.html) | Tráfico de red real, benigno y de ataque, capturado en archivos PCAP | `transmission1.txt`, `transmission2.txt`, `mcode1.txt`, `mcode2.txt`, `mcode3.txt` | II · ciberseguridad |

**Real y simulado.** `mapping.csv` asocia cada transmisión con un enlace de la red (`transmission1.txt` con Austria–Suiza como tráfico benigno y `transmission2.txt` con Austria–Alemania como tráfico DDoS). Esa asociación es una **construcción de simulación** del caso: las capturas de CIC-IDS2017 no ocurrieron en la red GÉANT. Por eso encontrar una firma en una transmisión indica un patrón sospechoso, pero no confirma una infección en ese enlace.

**Transformaciones aplicadas.**

- Las transmisiones son hexadecimal en mayúsculas derivado de los PCAP. Antes de procesarlas se eliminan saltos de línea (incluido `\r`), espacios y tabuladores (`transmision.h`), y todas las posiciones se reportan en base 1 sobre esa secuencia limpia.
- El palíndromo y la subcadena común se buscan sobre caracteres hexadecimales, no sobre bits.
- En Voronoi las coordenadas se tratan como puntos en el plano y las regiones se recortan al dominio indicado en `bounds`.
- El flujo máximo usa las capacidades de los enlaces; las demandas no intervienen en ese cálculo.
- El TSP trabaja sobre la matriz de distancias que recibe: el costo reportado es la suma de `distances[ruta[i]][ruta[i + 1]]`.

**Lo que no se incluye.** Los PCAP completos de CIC-IDS2017 no forman parte del repositorio; solo se documentan su fuente y las transformaciones.

**Estado.** La página del caso lista los archivos de entrada, pero sin enlace de descarga. Mientras tanto:

- La aplicación usa `app/datos/red.json`: la topología GÉANT del laboratorio de la página (14 centrales con sus coordenadas, 24 enlaces y capacidades de 10, 40 o 100 Gbps). La distancia de cada enlace se calcula con la fórmula de haversine y se usa también como costo de instalación.
- Las transmisiones de los escenarios son el ejemplo de la página o cadenas sintéticas generadas con semilla 2038, marcadas como tales; no provienen de CIC-IDS2017.
- Las pruebas y benchmarks usan instancias generadas con semilla fija y el formato del autograder.

Cuando lleguen los archivos oficiales basta con convertirlos al formato de `red.json` y de un escenario, y cargarlos desde el Scenario Manager.
