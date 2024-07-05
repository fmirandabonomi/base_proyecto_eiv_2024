#ifndef ACCESO_EXCUSIVO_H
#define ACCESO_EXCUSIVO_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Modifica un registro usando las instrucciones de acceso exclusivo
 * 
 * @param registro 
 * @param mascara 
 * @param valor 
 */
__attribute__((__always_inline__))
inline static void modificaBits(volatile uint32_t *registro,uint32_t mascara,uint32_t valor)
{
    bool fallo;
    do{
        __DMB();
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW((anterior & ~mascara)|(valor & mascara),registro);
    }while(fallo);
    __DMB();
}
__attribute__((__always_inline__))
inline static void estableceBits(volatile uint32_t *registro,uint32_t bits)
{
    bool fallo;
    do{
        __DMB();
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW(anterior | bits,registro);
    }while(fallo);
    __DMB();
}
__attribute__((__always_inline__))
inline static void limpiaBits(volatile uint32_t *registro,uint32_t bits)
{
    bool fallo;
    do{
        __DMB();
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW(anterior & ~bits,registro);
    }while(fallo);
    __DMB();
}

#endif //ACCESO_EXCUSIVO_H
