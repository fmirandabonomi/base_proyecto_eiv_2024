#ifndef TECLADO_H
#define TECLADO_H
#include <stdbool.h>

typedef enum CodigoTecla{
    CodigoTecla_0,
    CodigoTecla_1,
    CodigoTecla_2,
    CodigoTecla_3,
    CodigoTecla_4,
    CodigoTecla_5,
    CodigoTecla_6,
    CodigoTecla_7,
    CodigoTecla_8,
    CodigoTecla_9,
    CodigoTecla_ASTERISCO,
    CodigoTecla_NUMERAL,
    CodigoTecla_NO_VALIDO
}CodigoTecla;

typedef struct Teclado Teclado;

struct Teclado{
    unsigned fila;
    void (*seleccionaFila)(unsigned);
    unsigned (*leeColumnas)(void);
    unsigned char estadoColumnas[4];
    bool hayTeclaPendiente;
    CodigoTecla teclaPendiente;
};


/**
 * @brief Inicializa un teclado
 * 
 * @param teclado El teclado
 * @param seleccionaFila Puntero a función que acepta un número de fila (entre 
 * 0 y 3) y selecciona la fila indicada
 * @param leeColumnas Puntero a función que lee el estado de las columnas del
 * teclado. El estado debe retornarse como un entero de tres bit. El bit 3
 * corresponde a la columna izquierda.
 */
void Teclado_inicializa(
    Teclado *teclado,
    void (*seleccionaFila)(unsigned),
    unsigned (*leeColumnas)(void));

/**
 * @brief Rutina de proceso de la máquina de estado del teclado, llamar en el
 * lazo principal para que el teclado funcione correctamente
 * 
 * @param teclado El teclado
 */
void Teclado_procesa(Teclado *teclado);

/**
 * @brief Indica si hay una entrada pendiente en la cola del teclado
 * 
 * @param teclado El teclado
 * @retval true Hay entrada pendiente
 * @retval false No hay entrada pendiente
 */
bool Teclado_hayEntradaPendiente(Teclado *teclado);

/**
 * @brief Obtiene la siguiente entrada pendiente de un teclado y la escribe en
 * la variable destino
 * @note Remueve la entrada tomada de la cola del teclado
 * @param teclado El teclado
 * @retval CodigoTecla_NO_VALIDO No había entrada pendiente 
 * @retval CodigoTecla correspondiente a la entrada pendiente
 */
CodigoTecla Teclado_obtEntrada(Teclado *teclado);
#endif //TECLADO_H
