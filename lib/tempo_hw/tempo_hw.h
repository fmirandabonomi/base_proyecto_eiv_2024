#ifndef TEMPO_HW_H
#define TEMPO_HW_H
#include <stdint.h>
#include "accion.h"

enum{TempoHW_ERROR_CONFIGURACION = UINT32_MAX};

typedef enum TempoHW{
    TempoHW_1,
    TempoHW_2,
    TempoHW_3,
    TempoHW_4,
    TempoHW_NO_VALIDO
}TempoHW;

typedef enum TempoHW_FiltroEntrada{
    /// @brief Sin filtro de entrada
    THWFiltroEntrada_NINGUNO,
    /// @brief El filtro de entrada más corto para la frecuencia
    THWFiltroEntrada_CORTO,
    /// @brief El filtro de entrada intermedio para la frecuencia, o el largo si
    /// solo hay dos opciones
    THWFiltroEntrada_MEDIO,
    /// @brief El filtro de entrada más largo para la frecuencia
    THWFiltroEntrada_LARGO,
    THWFiltroEntrada_NO_VALIDO
}TempoHW_FiltroEntrada;

typedef enum TempoHW_PolaridadesEncoder{
    THWPolaridadesEncoder_PP = 0b00,
    THWPolaridadesEncoder_PN = 0b01,
    THWPolaridadesEncoder_NP = 0b10,
    THWPolaridadesEncoder_NN = 0b11,
    THWPolaridadesEncoder_NO_VALIDO = 0b100
}TempoHW_PolaridadesEncoder;

typedef enum TempoHW_ModoEncoder{
    /// @brief Cuenta un paso con cada flanco del primer canal del encoder
    THWModoEncoder_T1,
    /// @brief Cuenta un paso con cada flanco del segundo canal del encoder
    THWModoEncoder_T2,
    /**
     * @brief Cuenta un paso con cada flanco de cada canal del encoder,
     * contará los pasos de THWModoEncoder_T1 y los de THWModoEncoder_T2 resultando en el doble de
     * pasos
     */
    THWModoEncoder_T12,
    THWModoEncoder_NO_VALIDO
}TempoHW_ModoEncoder;

typedef enum TempoHW_Canal{
    THWCanal_1,
    THWCanal_2,
    THWCanal_3,
    THWCanal_4,
    THWCanal_NO_VALIDO
}TempoHW_Canal;

typedef enum TempoHW_Flanco{
    THWFlanco_ASCENDENTE,
    THWFlanco_DESCENDENTE,
    THWFlanco_ASCENDENTE_DESCENDENTE,
    THWFlanco_NO_VALIDO
}TempoHW_Flanco;

typedef enum TempoHW_ModoPWM{
    THWModoPWM_ACTIVO_IZQUIERDA,
    THWModoPWM_ACTIVO_DERECHA,
    THWModoPWM_NO_VALIDO
}TempoHW_ModoPWM;

typedef enum TempoHW_ModoCuenta{
    THWModoCuenta_ASCENDENTE,
    THWModoCuenta_DESCENDENTE,
    THWModoCuenta_ASCENDENTE_ICC_DESCENDENTE,
    THWModoCuenta_ASCENDENTE_DESCENDENTE_ICC,
    THWModoCuenta_ASCENDENTE_ICC_DESCENDENTE_ICC,
    THWModoCuenta_NO_VALIDO
}TempoHW_ModoCuenta;

typedef enum TempoHW_PrescalerCaptura{
    THWPrescalerCaptura_DIV1,
    THWPrescalerCaptura_DIV2,
    THWPrescalerCaptura_DIV4,
    THWPrescalerCaptura_DIV8,
    THWPrescalerCaptura_NO_VALIDO
}TempoHW_PrescalerCaptura;

typedef enum TempoHW_Entrada{
    /// @brief C1->I1 C2->I2 ...
    THWEntrada_DIRECTA,
    /// @brief C1->I2 C2->I1 C3->I4 C4->I3
    THWEntrada_CRUZADA,
    THWEntrada_DISPARO_INTERNO,
    THWEntrada_NO_VALIDO
}TempoHW_Entrada;


/**
 * @brief Configura la frecuencia de reloj de un temporizador. Si el contador
 * estaba encendido continúa encendido, caso contrario debe encenderse con
 * TempoHW_enciendeContador
 * 
 * @param tempo El temporizador
 * @param frec La frecuencia en Hz
 * @retval TempoHW_ERROR_CONFIGURACION si no pudo configurar el reloj
 * @retval Frecuencia configurada (puede diferir de la solicitada)
 */
uint32_t TempoHW_configReloj(TempoHW tempo,uint32_t frec);

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
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error de configuración 
 * @retval La frecuencia de muestreo efectiva si la configuración fue
 * correcta
 */
uint32_t TempoHW_configModoEncoder(TempoHW tempo,TempoHW_ModoEncoder m,uint32_t fs, TempoHW_FiltroEntrada filt, TempoHW_PolaridadesEncoder pol,uint16_t pasosPorCuenta);


/**
 * @brief Enciende el contador del periférico temporizador
 * 
 * @param tempo El temporizador
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error 
 * @retval 0 si fue encendido
 */
uint32_t TempoHW_enciendeContador(TempoHW tempo);

/**
 * @brief Apaga el contador de un periférico temporizador
 * 
 * @param tempo El temporizador
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error
 * @retval 0 si fue apagado 
 */
uint32_t TempoHW_apagaContador(TempoHW tempo);


/**
 * @brief Obtiene la cuenta actual del contador de un temporizador
 * 
 * @param tempo El temporizador
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error
 * @retval Número de 0 a 65535, cuenta actual del temporizador (16 bit) 
 */
uint32_t TempoHW_obtCuenta(TempoHW tempo);

/**
 * @brief Configura un canal como PWM en un temporizador
 * 
 * @param tempo El temporizador
 * @param canal El canal
 * @param cuentaMaxima Cuenta máxima del contador, es el periodo (o semiperiodo
 * para modo ascendente-descendente) de la señal PWM
 * @param umbral Umbral (debe ser menor a la cuentaMaxima)
 * @param modoPwm Modo de pwm
 * @param modoCuenta Modo de cuenta del contador
 * @param accion Accion a ejecutar 
 * @return uint32_t 
 */
uint32_t TempoHW_configPwm(
    TempoHW tempo,
    TempoHW_Canal canal,
    uint32_t cuentaMaxima,
    uint32_t umbral,
    TempoHW_ModoPWM modoPwm,
    TempoHW_ModoCuenta modoCuenta,
    Accion * accion);


/**
 * @brief Configura el umbral de comparación de un canal en un temporizador
 * 
 * @param tempo El temporizador
 * @param canal El canal
 * @param umbral El valor umbral
 * @return uint32_t 
 */
uint32_t TempoHW_ponUmbralComparacion(
    TempoHW tempo,
    TempoHW_Canal canal,
    uint32_t umbral);

/**
 * @brief Configura un canal de captura en un temporizador
 * 
 * @param tempo El temporizador
 * @param canal El canal
 * @param entrada La entrada tomada por el canal
 * @param filtro El filtro aplicado a la entrada
 * @param frecMuestreoEntrada Frecuencia con que se desea muestrear la entrada
 * @param flanco El flanco capturado
 * @param flancosPorCaptura Cantidad de flancos que generan una captura
 * @param accion Accion disparada al efectuar captura (NULL si ninguna)
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error
 * @retval uint32_t Frecuencia de muestreo obtenida
 */
uint32_t TempoHW_configCaptura(
    TempoHW tempo,
    TempoHW_Canal canal,
    TempoHW_Entrada entrada,
    TempoHW_FiltroEntrada filtro,
    uint32_t frecMuestreoEntrada,
    TempoHW_Flanco flanco,
    TempoHW_PrescalerCaptura flancosPorCaptura,
    Accion* accion);

/**
 * @brief Cambia la configuración de modo y límite de un temporizador
 * 
 * @param tempo El temporizador
 * @param modo El modo
 * @param limite El valor límite superior
 * @retval TempoHW_ERROR_CONFIGURACION si ocurrió un error (parámetros inválidos)
 * @retval uint32_t La cantidad de cuentas en un periodo del contador
 */
uint32_t TempoHW_configuraCuenta(
    TempoHW             const tempo,
    TempoHW_ModoCuenta  const modo,
    uint32_t            const limite);

#endif // TEMPO_HW_H
