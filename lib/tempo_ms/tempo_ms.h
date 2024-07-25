#ifndef TEMPORIZACION_SW_H
#define TEMPORIZACION_SW_H

#include <stdint.h>
#include <accion.h>
#include <stdbool.h>

/**
 * @brief Inicializa la temporización de milisegundo. A partir del llamado a
 * esta función podrán utilizarse las demás funciones Tempo_...
 * 
 */
void Tempo_inicializa(void);

/**
 * @brief Obtiene la cuenta de milisegundos actual. El valor se incrementa cada
 * milisegundo a partir del llamado a Tempo_inicializa
 * 
 * @return uint32_t La cuenta de milisegundos
 */
uint32_t Tempo_obtMilisegundos(void);

/**
 * @brief Bloquea la ejecución durante un tiempo especificado en milisegundos
 * 
 * @param tiempo El tiempo
 */
void Tempo_esperaMilisegundos(uint32_t tiempo);


/**
 * @brief Configura una acción que se ejecutará cada milisegudo. Nota: La acción
 * se ejecutará en modo Handler. Solo puede haber una acción configurada por vez,
 * Tempo_eliminaAccionMilisegundo puede quitar la acción actual. El parámetro de
 * la acción es un puntero al valor de cuenta de milisegundos tipo uint32_t
 * 
 * @param accion La accion
 * @retval 0 Se ha configurado la acción
 * @retval -1 Error al configurar la acción 
 */
int Tempo_ponAccionMilisegundo(AccionParam *accion);

/**
 * @brief Quita la acción a ejecutar cada milisegundo.
 * 
 */
void Tempo_eliminaAccionMilisegundo(void);

void Tempo_esperaMicrosegundos(uint32_t t);
#endif // TEMPORIZACION_SW_H
