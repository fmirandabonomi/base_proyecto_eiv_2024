#ifndef TEMPO_HW_H
#define TEMPO_HW_H
#include <stdint.h>

typedef enum HTempoHW{
    THW1,THW2,THW3,THW4,HTempoHW_NO_VALIDO
}HTempoHW;

#define TempoHW_ERROR_CONFIG UINT32_MAX

/**
 * @brief Configura la frecuencia de reloj de un temporizador. Si el contador
 * estaba encendido continúa encendido, caso contrario debe encenderse con
 * TempoHW_enciendeContador
 * 
 * @param tempo El temporizador
 * @param frec La frecuencia en Hz
 * @retval TempoHW_ERROR_CONFIG si no pudo configurar el reloj
 * @retval Frecuencia configurada (puede diferir de la solicitada)
 */
uint32_t TempoHW_configReloj(HTempoHW tempo,uint32_t frec);

typedef enum ModoEncoder{
    /// @brief Cuenta un paso con cada flanco del primer canal del encoder
    ME_T1,
    /// @brief Cuenta un paso con cada flanco del segundo canal del encoder
    ME_T2,
    /**
     * @brief Cuenta un paso con cada flanco de cada canal del encoder,
     * contará los pasos de ME_T1 y los de ME_T2 resultando en el doble de
     * pasos
     */
    ME_T12,
    ModoEncoder_NO_VALIDO
}ModoEncoder;

typedef enum FiltroEntrada{
    /// @brief Sin filtro de entrada
    FE_NINGUNO,
    /// @brief El filtro de entrada más corto para la frecuencia
    FE_CORTO,
    /// @brief El filtro de entrada intermedio para la frecuencia, o el largo si
    /// solo hay dos opciones
    FE_MEDIO,
    /// @brief El filtro de entrada más largo para la frecuencia
    FE_LARGO,
    FiltroEntrada_NO_VALIDO
}FiltroEntrada;
typedef enum Polaridades{
    Polaridades_PP = 0b00,
    Polaridades_PN = 0b01,
    Polaridades_NP = 0b10,
    Polaridades_NN = 0b11,
    Polaridades_NO_VALIDO = 0b100
}Polaridades;

/**
 * @brief Configura un temporizador en modo de lectura de encoder de cuadratura.
 * Implica configurar como entrada los dos primeros canales de
 * captura/comparación. Luego debe activarse el timer con
 * TempoHW_enciendeContador
 * 
 * @param tempo El temporizador 
 * @param m El modo de conteo
 * @param fs La frecuencia de muestreo deseada para las señales del encoder (es
 * indicativa, la frecuencia realmente obtenida será retornada por la función si
 * la configuración fue exitosa)
 * @param filt El filtro de entrada a utilizar
 * @param pol Polaridades de las señales del encoder
 * @retval TempoHW_ERROR_CONFIG si ocurrió un error de configuración 
 * @retval La frecuencia de muestreo efectiva si la configuración fue
 * correcta
 */
uint32_t TempoHW_configModoEncoder(HTempoHW tempo,ModoEncoder m,uint32_t fs, FiltroEntrada filt, Polaridades pol,uint16_t pasosPorCuenta);


/**
 * @brief Enciende el contador del periférico temporizador
 * 
 * @param tempo El temporizador
 * @retval TempoHW_ERROR_CONFIG si ocurrió un error 
 * @retval 0 si fue encendido
 */
uint32_t TempoHW_enciendeContador(HTempoHW tempo);


/**
 * @brief Obtiene la cuenta actual del contador de un temporizador
 * 
 * @param tempo El temporizador
 * @retval TempoHW_ERROR_CONFIG si ocurrió un error
 * @retval Número de 0 a 65535, cuenta actual del temporizador (16 bit) 
 */
uint32_t TempoHW_obtCuenta(HTempoHW tempo);



#endif // TEMPO_HW_H
