#ifndef PASO_A_PASO_H
#define PASO_A_PASO_H
#include <stdbool.h>

/**
 * @brief Actualiza los pines de salida que controlan los bobinados del
 * motor paso a paso (bits 3..0 : A,B,C,D) donde A=1 corresponde a fase 1
 * polarización directa, B=1 corresponde a fase 2 polarización directa,
 * C=1 corresponde a fase 1 polarización inversa y D=1 corresponde a fase 2
 * polarización inversa. No pueden ser simultáneamente 1 A y C ni B y D. 
 * 
 * @param abcd nuevos valores de salida
 */
typedef void (*PAP_ActualizaSalidas)(int abcd);

/**
 * @brief Estado de controlador de motor paso a paso
 * 
 */
typedef struct PAP{
    PAP_ActualizaSalidas actualiza;
    unsigned cuenta;
    bool medioPaso;
}PAP;

/**
 * @brief Inicializa un controlador de motor paso a paso
 * 
 * @param pap El controlador
 * @param actualiza Función de actualización de salidas 
 * @param medioPaso True para usar secuencia de medio paso (doble resolución)
 * @retval 0 si configuación correcta
 * @retval -1 si hubo un error en la configuración
 */
int PAP_inicializa(PAP *pap, PAP_ActualizaSalidas actualiza, bool medioPaso);

/**
 * @brief Avanza un motor paso a paso en un paso
 * 
 * @param pap El controlador
 */
void PAP_retrocede(PAP *pap);

/**
 * @brief Retrocede un motor paso a paso en un paso
 * 
 * @param pap El controlador
 */
void PAP_avanza(PAP *pap);


#endif //PASO_A_PASO_H
