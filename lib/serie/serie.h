#ifndef SERIE_H
#define SERIE_H

#include <stdint.h>

/**
 * @brief Inicializa puerto serie 8-n-1 con el baudrate especificado
 * 
 * @param baudrate 
 * @return int 0 si tuvo exito, -1 si el baudrate no es soportado
 */
int Serie_init(int baudrate);

/**
 * @brief Bloquea hasta completar toda transmisión pendiente
 * 
 */
void Serie_flush(void);

/**
 * @brief Apaga puerto serie
 * 
 */
void Serie_deinit(void);

/**
 * @brief Envía un caracter por el puerto serie
 * 
 * @param c caracter a enviar
 */
void Serie_enviaCaracter(int c);

/**
 * @brief Envía la representación en texto del valor entero
 * 
 * @param valor 
 */
void Serie_enviaEntero(int32_t valor);

/**
 * @brief Envía una secuencia de caracteres de nueva línea (CR-LF)
 * 
 */
void Serie_enviaNuevaLinea(void);

/**
 * @brief Envía la representación textual de todos los enteros del arreglo
 * separados por espacios
 * 
 * @param base Dirección del arreglo
 * @param tamano Cantidad de elementos en el arreglo
 */
void Serie_enviaArreglo_int32(const int32_t *base,int tamano);


/**
 * @brief Envía una cadena de caracteres terminada en cero
 * 
 * @param cadena Puntero a la cadena
 */
void Serie_enviaCadena(const char *cadena);

/**
 * @brief Recibe un caracter
 * 
 * @return int valor recibido
 */
int Serie_recibeCaracter(void);

/**
 * @brief Recibe e ignora cero o más espacios en blanco. Recibe y almacena hasta
 * tamanoBuffer-1 caracteres que no sean espacio en blanco. Al recibir el
 * próximo espacio en blanco lo descarta y retorna.
 * 
 * @param buffer Espacio de memoria donde guardar los caracteres recibidos
 * @param tamanoBuffer Tamaño del buffer
 * @return int cantidad de caracteres leidos
 */
int Serie_recibePalabra(char *buffer,int tamanoBuffer);

#endif
