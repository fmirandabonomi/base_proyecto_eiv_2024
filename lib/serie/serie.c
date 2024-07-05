#include "serie.h"
#include "gpio.h"
#include <stm32f1xx.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <stdatomic.h>
#include <acceso_esclusivo.h>


int Serie_init(int baudrate){
    int error = -1;
    SystemCoreClockUpdate();

    estableceBits(&RCC->APB2ENR,RCC_APB2ENR_USART1EN);    
    Pin_configuraSalida(PA9,PUSH_PULL | FUNCION_ALTERNATIVA,V_BAJA);
    Pin_configuraEntrada(PA10,PULL_UP);

    const uint32_t divisor = SystemCoreClock/baudrate;

    if (divisor > 0 && divisor < 0x10000){
        USART1->BRR = (SystemCoreClock + (baudrate+1)/2) / baudrate;
        error = 0;
    }else if(divisor){
        USART1->BRR = 0xFFFF;
    }else{
        USART1->BRR = 1;
    }
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    return error;
}



void Serie_enviaCaracter(int c){
    while(!(USART1->SR & USART_SR_TXE));
    USART1->DR = c&0xFF;
}

/**
 * @brief 
 * 
 * @param buffer 
 * @param tamano Tamaño de buffer, al menos 12
 * @param valor 
 * @return char* 
 */
static char* int32_t_reprBase10(char *buffer,int tamano,int32_t valor){
    int32_t a = valor>0 ? valor : -valor;
    char *p = buffer+tamano-1;
    *p = 0; // terminador
    while(a>0){
        p--;
        *p = '0' + a%10;
        a/=10;
        if(p == buffer) break;
    }
    if(valor < 0){
        if(p!=buffer) p--;
        *p = '-';
    }
    return p;
}

/**
 * @brief Envía la representación en texto del valor entero
 * 
 * @param valor 
 */
void Serie_enviaEntero(int32_t valor){
    enum{TAMANO_BUFFER = 12};
    char buffer[TAMANO_BUFFER];
    if (!valor){
        Serie_enviaCaracter('0');
    }else{
        Serie_enviaCadena(int32_t_reprBase10(buffer,TAMANO_BUFFER,valor));
    }
}

/**
 * @brief Envía una secuencia de caracteres de nueva línea (CR-LF)
 * 
 */
void Serie_enviaNuevaLinea(void){
    Serie_enviaCaracter(13);
    Serie_enviaCaracter(10);
}

/**
 * @brief Envía la representación textual de todos los enteros del arreglo
 * separados por espacios
 * 
 * @param base Dirección del arreglo
 * @param tamano Cantidad de elementos en el arreglo
 */
void Serie_enviaArreglo_int32(const int32_t *base,int tamano){
    const int32_t *const fin = base+tamano;
    for(const int32_t *p = base; p<fin ;++p){
        Serie_enviaEntero(*p);
        if(p+1 < fin) Serie_enviaCaracter(' ');
    }
}

/**
 * @brief Envía una cadena de caracteres terminada en cero
 * 
 * @param cadena Puntero a la cadena
 */
void Serie_enviaCadena(const char *cadena){
    for(const char*p=cadena; *p; ++p)
        Serie_enviaCaracter(*p);
}

int Serie_recibeCaracter(void){
    while(!(USART1->SR & USART_SR_RXNE));
    return USART1->DR;
}

/**
 * @brief Recibe e ignora cero o más espacios en blanco. Recibe y almacena hasta
 * tamanoBuffer-1 caracteres que no sean espacio en blanco. Al recibir el
 * próximo espacio en blanco lo descarta y retorna.
 * 
 * @param buffer Espacio de memoria donde guardar los caracteres recibidos
 * @param tamanoBuffer Tamaño del buffer
 * @return int cantidad de caracteres leidos
 */
int Serie_recibePalabra(char *buffer,int tamanoBuffer){
    char *const finCadena = buffer + tamanoBuffer - 1;
    char *p = buffer;
    int c = Serie_recibeCaracter();
    while(isspace(c)) c = Serie_recibeCaracter();
    while(!isspace(c)){
        if(p != finCadena){
            *p = c;
            ++p;
        }
        c = Serie_recibeCaracter();
    }
    *p = 0; // terminador
    return p - buffer;
}


void Serie_flush(void){
    while(!(USART1->SR & USART_SR_TC));
}


void Serie_deinit(void){
    Serie_flush();
    USART1->CR1 = 0;
    Pin_configuraEntrada(PA9,FLOTANTE);
    Pin_configuraEntrada(PA10,FLOTANTE);
}