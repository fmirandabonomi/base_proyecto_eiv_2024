#ifndef GPIO_H
#define GPIO_H
#include <stdbool.h>
#include <stdint.h>

// Nota: PA13 es SWDIO, PA14 es SWCLK, PB2 es BOOT1

typedef enum HPin{
    PA0 ,PA1 ,PA2 ,PA3 ,
    PA4 ,PA5 ,PA6 ,PA7 ,
    PA8 ,PA9 ,PA10,PA11,
    PA12,          PA15,
    PB0 ,PB1 ,     PB3 ,
    PB4 ,PB5 ,PB6 ,PB7 ,
    PB8 ,PB9 ,PB10,PB11,
    PB12,PB13,PB14,PB15,
    PC12,PC13,PC14,PC15,
    HPin_NO_VALIDO,
    HPin_NINGUNO,
    PIN_LED = PC13,
} HPin;

#define NUM_PUERTOS 3

typedef enum ModoEntrada{
    ANALOGICO,FLOTANTE,PULL_UP,PULL_DOWN
}ModoEntrada;

typedef enum ModoSalida{
    PUSH_PULL=0b0000,DRENADOR_ABIERTO=0b0100
}ModoSalida;
typedef enum VelocidadSalida{
    V_BAJA = 0b10,V_MEDIA=0b01,V_ALTA=0b11
}VelocidadSalida;

/**
 * @brief Configura un pin como entrada
 * 
 * @param p El pin
 * @param m El modo de entrada
 * @retval 0 si la configuración tuvo exito
 * @retval -1 si la configuración falló
 */
int Pin_configuraEntrada(HPin p,ModoEntrada m);
/**
 * @brief Configura un pin como salida
 * 
 * @param p El pin
 * @param m El modo de entrada
 * @retval 0 si la configuración tuvo exito
 * @retval -1 si la configuración falló
 */
int Pin_configuraSalida(HPin p,ModoSalida m,VelocidadSalida v);

/**
 * @brief Lee el buffer de entrada de un pin
 * 
 * @param p El pin
 * @retval true El potencial del pin corresponde al estado alto 
 * @retval false El potencial del pin corresponde al estado bajo
 */
bool Pin_lee(HPin p);
/**
 * @brief Lee el estado del buffer de salida de un pin
 * 
 * @param p El pin
 * @return true El buffer de salida está en alto
 * @return false El buffer de salida está en bajo
 */
bool Pin_obtEstado(HPin p);

/**
 * @brief Pon el buffer de salida de un pin en estado alto
 * 
 * @param p El pin
 */
void Pin_ponAlto(HPin p);
/**
 * @brief Pon el buffer de salida de un pin en estado bajo
 * 
 * @param p El pin
 */
void Pin_ponBajo(HPin p);
/**
 * @brief Conmuta el estado del buffer de salida de un pin entre alto y bajo
 * 
 * @param p El pin
 */
void Pin_conmuta(HPin p);
/**
 * @brief Escribe el buffer de salida de un pin con un valor
 * 
 * @param p El pin
 * @param valor El valor, true para alto y false para bajo
 */
void Pin_escribe(HPin p,bool valor);

/* Tiempos medidos:
 * Para el bus de 5 bits [PB12,PB13,PB14,PB15,PA8]
 * Bus_lee ~195 ciclos
 * Bus_escribe ~204 ciclos
 * Cambio de modo ~114 ciclos
 * Partiendo de modo escritura, Bus_lee seguido de Bus_escribe ~627 ciclos
 */

/**
 * @brief Variable que representa un bus paralelo. Siempre debe inicializarse
 * con la macro INICIALIZA_VARIABLE_BUS(<Modo de entrada>,<Modo de salida>,
 * <Velocidad de salida>,<Pines de mas a menos significativo>)
 * @note static Bus miBus = INICIALIZA_VARIABLE_BUS(PULL_UP,DRENADOR_ABIERTO,
 * S_LENTA,PB12,PB13,PB14,PA8);
 */
typedef struct Bus{
    uint32_t mascaraConfigL[NUM_PUERTOS];
    uint32_t mascaraConfigH[NUM_PUERTOS];
    uint32_t configEntrada;
    uint32_t configSalida;
    unsigned short id;
    unsigned short mascarasPines[NUM_PUERTOS];
    enum Bus_Direccion{D_NO_CONFIGURADA,D_ENTRADA,D_SALIDA} direccion;
    unsigned char numPines; 
    unsigned char pines[];
}Bus;

/**
 * @brief Inicializa una variable Bus. Es necesario usar esta macro como
 * inicializador de la variable Bus. La variable bus tiene que ser static
 * @note static Bus miBus = INICIALIZA_VARIABLE_BUS(PULL_UP,DRENADOR_ABIERTO,
 * S_LENTA,PB12,PB13,PB14,PA8); // lee ~251 ciclos, escribe ~231 ciclos,
 * alternado
 */
#define INICIALIZA_VARIABLE_BUS(modoEntrada_,modoSalida_,velocidadSalida_,...)\
    {.id            = 0xB005,\
     .configEntrada = (modoEntrada_),\
     .configSalida  = (modoSalida_) | (velocidadSalida_),\
     .direccion     = D_NO_CONFIGURADA,\
     .numPines      = sizeof((char[]){__VA_ARGS__}),\
     .pines         = {__VA_ARGS__}}


/**
 * @brief Escribe en bus paralelo, establece el modo salida si es necesario
 * 
 * @param b El bus
 * @param valor El valor a escribir
 * @retval 0 si tuvo éxito
 * @retval -1 si ocurrió un error al escribir 
 */
int Bus_escribe(Bus *b,unsigned valor);
/**
 * @brief Lee un bus paralelo, establece el modo entrada si es necesario
 * 
 * @param b El bus
 * @param destino Referencia a variable donde se escribirá el valor leido
 * @retval 0 si tuvo éxito
 * @retval -1 si ocurrió un error al leer, en este caso destino no fue
 * modificado
 */
int Bus_lee(Bus *b,unsigned *destino);

/**
 * @brief Configura bus paralelo como entrada
 * 
 * @param b El bus
 * @retval 0 si tuvo éxito 
 * @retval -1 si ocurrió un error al configurar 
 */
int Bus_ponModoEntrada(Bus *b);
/**
 * @brief Configura bus paralelo como salida
 * 
 * @param b El bus
 * @retval 0 si tuvo éxito 
 * @retval -1 si ocurrió un error al configurar 
 */
int Bus_ponModoSalida(Bus *b);

#endif //GPIO_H
