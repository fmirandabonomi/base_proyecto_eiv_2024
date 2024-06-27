#include "gpio.h"
#include <stm32f1xx.h>
#include <stddef.h>
#include <assert.h>

typedef enum HPuerto{PA,PB,PC,HPuerto_INVALIDO}HPuerto;
static_assert((int)HPuerto_INVALIDO == (int)NUM_PUERTOS,"Número de puertos incorrecto");
__attribute__((__always_inline__))
inline static void modificaBits(volatile uint32_t *registro,uint32_t mascara,uint32_t valor)
{
    bool fallo;
    do{
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW((anterior & ~mascara)|(valor & mascara),registro);
    }while(fallo);
}
__attribute__((__always_inline__))
inline static void estableceBits(volatile uint32_t *registro,uint32_t bits)
{
    bool fallo;
    do{
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW(anterior | bits,registro);
    }while(fallo);
}

__attribute__((__always_inline__))
inline static HPuerto obtHPuerto(HPin p)
{
  /*                          / <si> si <cond> != 0
    (<cond> ? <si> : <no>) = <
                              \ <no> si <cond> == 0
   */
    return (p <= PA15)? PA :
           (p <= PB15)? PB :
           (p <= PC15)? PC :
                        HPuerto_INVALIDO;
}

static void configuraPinesJtagComoGpio(void)
{
    estableceBits(&RCC->APB2ENR,RCC_APB2ENR_AFIOEN);
    modificaBits(&AFIO->MAPR,AFIO_MAPR_SWJ_CFG_Msk,AFIO_MAPR_SWJ_CFG_JTAGDISABLE);
}
static HPuerto iniciaPuerto(HPin p)
{
    volatile uint32_t *const APB2ENR = &RCC->APB2ENR;
    HPuerto const puerto = obtHPuerto(p);
    switch(puerto){
    case PA:
        estableceBits(APB2ENR,RCC_APB2ENR_IOPAEN);
        if (p == PA15) configuraPinesJtagComoGpio();
    break;case PB:
        estableceBits(APB2ENR,RCC_APB2ENR_IOPBEN);
        if ((p == PB3) || (p == PB4)) configuraPinesJtagComoGpio();
    break;case PC:
        estableceBits(APB2ENR,RCC_APB2ENR_IOPCEN);
    break;default:
    break;
    }
    return puerto;
}

__attribute__((__always_inline__))
inline static GPIO_TypeDef *obtPuerto(HPuerto p)
{
    switch (p)
    {
    case PA: return GPIOA;
    case PB: return GPIOB;
    case PC: return GPIOC;
    default: return NULL;
    }
}

__attribute__((__always_inline__))
inline static int obtNumero(HPin p)
{
    static const unsigned char numeroPin[]={
        [PA0 ] =  0,[PA1 ] =  1,[PA2 ] =  2,[PA3 ] =  3,
        [PA4 ] =  4,[PA5 ] =  5,[PA6 ] =  6,[PA7 ] =  7,
        [PA8 ] =  8,[PA9 ] =  9,[PA10] = 10,[PA11] = 11,
        [PA12] = 12,                        [PA15] = 15,
        [PB0 ] =  0,[PB1 ] =  1,            [PB3 ] =  3,
        [PB4 ] =  4,[PB5 ] =  5,[PB6 ] =  6,[PB7 ] =  7,
        [PB8 ] =  8,[PB9 ] =  9,[PB10] = 10,[PB11] = 11,
        [PB12] = 12,[PB13] = 13,[PB14] = 14,[PB15] = 15,

        [PC12] = 12,[PC13] = 13,[PC14] = 14,[PC15] = 15,
    };
    return (p < HPin_NO_VALIDO) ? numeroPin[p] : 0;
}

static void configuraPin(GPIO_TypeDef *puerto,int numero, int config)
{
    volatile uint32_t *const CRx = (numero <8)? &puerto->CRL : &puerto->CRH;
    const int posicion = (numero % 8)*4;
    modificaBits(CRx,0xf<<posicion,(0xf&config)<<posicion);
}

static int bitsConfigEntrada(ModoEntrada m)
{
    switch(m){
    case ANALOGICO: return 0b0000;
    case FLOTANTE:  return 0b0100;
    case PULL_UP:   return 0b1000;
    case PULL_DOWN: return 0b1000;
    default:        return -1;
    }    
} 
int Pin_configuraEntrada(HPin p,ModoEntrada m)
{
    GPIO_TypeDef *const puerto = obtPuerto(iniciaPuerto(p));
    if (!puerto) return -1;
    const int numero = obtNumero(p);
    const int config = bitsConfigEntrada(m);
    if (config < 0) return -1;
    configuraPin(puerto,numero,config);
    if (m == PULL_UP){
        puerto->BSRR = 1 << numero;
    }else if (m == PULL_DOWN){
        puerto->BRR = 1 << numero;
    }
    return 0;
}
static int bitsConfigSalida(ModoSalida m,VelocidadSalida v)
{
    if (!(v&3)) return -1;
    return (m|v)&0xf;
}
int Pin_configuraSalida(HPin p,ModoSalida m,VelocidadSalida v)
{
    GPIO_TypeDef *const puerto = obtPuerto(iniciaPuerto(p));
    const int config = bitsConfigSalida(m,v);
    if (!puerto || config<0) return -1;
    const int numero = obtNumero(p);
    configuraPin(puerto,numero,config);
    return 0;
}

bool Pin_lee(HPin p)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const int numero = obtNumero(p);
    if (!puerto) return 0;
    return puerto->IDR & (1<<numero);
}
bool Pin_obtEstado(HPin p)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const int numero = obtNumero(p);
    if (!puerto) return 0;
    return puerto->ODR & (1<<numero);
}

void Pin_ponAlto(HPin p)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const int numero = obtNumero(p);
    if (!puerto) return;
    puerto->BSRR = 1 << numero;
}

void Pin_ponBajo(HPin p)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const int numero = obtNumero(p);
    if (!puerto) return;
    puerto->BRR = 1 << numero;
}

void Pin_conmuta(HPin p)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const uint32_t mascara = 1 << obtNumero(p);
    if (!puerto) return;
    if (puerto->ODR & mascara){
        puerto->BRR = mascara;
    }else{
        puerto->BSRR = mascara;
    }
}

void Pin_escribe(HPin p,bool valor)
{
    GPIO_TypeDef *const puerto = obtPuerto(obtHPuerto(p));
    const uint32_t mascara = 1 << obtNumero(p);
    if (!puerto) return;
    if (valor){
        puerto->BRR = mascara;
    }else{
        puerto->BSRR = mascara;
    }

}


static uint32_t creaMascaraConfig(uint32_t mascara8Pines)
{
    mascara8Pines = ((mascara8Pines & 0xF0) << (16-4)) | (mascara8Pines & 0x0F); 
    mascara8Pines = ((mascara8Pines & 0x000C000C) << (8-2)) | (mascara8Pines & 0x00030003); 
    mascara8Pines = ((mascara8Pines & 0x02020202) << (4-1)) | (mascara8Pines & 0x01010101);
    mascara8Pines |= mascara8Pines << 1;
    mascara8Pines |= mascara8Pines << 2;
    return mascara8Pines;
}
static uint32_t extiendeConfig(uint32_t config)
{
    config &= 0b1111;
    config |= config << 4;
    config |= config << 8;
    config |= config << 16;
    return config;
}
static int Bus_inicializa(Bus *const b)
{
    for (int i=0;i<b->numPines;++i){
        const HPin pin = b->pines[i];
        const HPuerto puerto = obtHPuerto(pin);
        const int numero = obtNumero(pin);
        if(puerto == HPuerto_INVALIDO) return -1;
        iniciaPuerto(pin);
        b->mascarasPines[puerto] |= 1<<numero;
        b->pines[i] = (puerto << 4) | (numero & 0xF);
    }
    for (HPuerto i=0;i<HPuerto_INVALIDO;++i)
    {
        const uint32_t mascaraPines = b->mascarasPines[i];
        b->mascaraConfigL[i] = creaMascaraConfig(mascaraPines); 
        b->mascaraConfigH[i] = creaMascaraConfig(mascaraPines >> 8); 
    }
    const ModoEntrada mE = b->configEntrada;
    int bitsConfig = bitsConfigEntrada(mE);
    if (bitsConfig < 0) return -1;
    b->configEntrada = extiendeConfig(bitsConfig);
    if (mE == PULL_UP) b->configEntrada |= 1;
    if(!(b->configSalida & 0b11)) return -1;
    b->configSalida  = extiendeConfig(b->configSalida);
    return 0;
}

int Bus_escribe(Bus *b,unsigned valor)
{
    uint16_t valores[NUM_PUERTOS]={};
    int r = Bus_ponModoSalida(b);
    if (r) return r;
    unsigned mascaraValor = 1<<(b->numPines-1);
    // Indica los bits en uno para cada puerto
    for(int i=0;i<b->numPines;++i){
        const unsigned pin    = b->pines[i];
        const HPuerto hPuerto = pin >> 4;
        const int numero      = pin & 0xf;
        if (valor & mascaraValor){
            valores[hPuerto] |= 1 << numero;
        }
        mascaraValor>>=1;
    }
    // Actualiza los registros de salida
    for (HPuerto i=0; i<HPuerto_INVALIDO;++i){
        uint32_t ceros = b->mascarasPines[i];
        if (!ceros) continue;
        const uint32_t unos = valores[i];
        ceros &= ~unos;
        GPIO_TypeDef *const puerto = obtPuerto(i);
        puerto->BSRR = (ceros << 16) | unos;
    }
    return 0;
}

int Bus_lee(Bus *b,unsigned *destino)
{
    uint16_t valores[NUM_PUERTOS];
    int r = Bus_ponModoEntrada(b);
    if (r) return r;
    // Lee los registros de entrada
    for (HPuerto i=0; i<HPuerto_INVALIDO;++i){
        if (!b->mascarasPines[i]) continue;
        GPIO_TypeDef *const puerto = obtPuerto(i);
        valores[i] = puerto->IDR;
    }
    // Extrae los bits del bus de los registros de entrada
    unsigned aux = 0;
    unsigned mascaraValor = 1<<(b->numPines-1);
    for(int i=0;i<b->numPines;++i){
        const unsigned pin = b->pines[i];
        const HPuerto hPuerto = pin >> 4;
        const int numero      = pin & 0xf;
        if (valores[hPuerto] & (1<<numero)){
            aux |= mascaraValor;
        }
        mascaraValor >>= 1;
    }
    *destino = aux;
    return 0;
}

int Bus_ponModoEntrada(Bus *b)
{
    if(b->id!=0xB005) return -1;
    if (b->direccion == D_NO_CONFIGURADA){
        const int r = Bus_inicializa(b);
        if (r) return r;
    }
    if (b->direccion != D_ENTRADA){
        b->direccion = D_ENTRADA;
        for(int i=0;i<NUM_PUERTOS;i++){
            const uint32_t mascaraPines = b->mascarasPines[i];
            if (!mascaraPines) continue;
            const uint32_t mascaraL = b->mascaraConfigL[i];
            const uint32_t mascaraH = b->mascaraConfigH[i];
            GPIO_TypeDef *const puerto = obtPuerto(i);
            const uint32_t config = b->configEntrada;
            modificaBits(&puerto->CRL,mascaraL,config & ~(uint32_t)1);
            modificaBits(&puerto->CRH,mascaraH,config & ~(uint32_t)1);
            if (config&0b1000){
                if (config & 1){
                    puerto->BSRR = b->mascarasPines[i];
                }else{
                    puerto->BRR = b->mascarasPines[i];
                }
            }
        }
    }
    return 0;
}

int Bus_ponModoSalida(Bus *b)
{
    if(b->id!=0xB005) return -1;
    if (b->direccion == D_NO_CONFIGURADA){
        const int r = Bus_inicializa(b);
        if (r) return r;
    }
    if (b->direccion != D_SALIDA){
        b->direccion = D_SALIDA;
        for(int i=0;i<NUM_PUERTOS;i++){
            const uint32_t mascaraPines = b->mascarasPines[i];
            if (!mascaraPines) continue;
            const uint32_t mascaraL = b->mascaraConfigL[i];
            const uint32_t mascaraH = b->mascaraConfigH[i];
            GPIO_TypeDef *const puerto = obtPuerto(i);
            const uint32_t config = b->configSalida;
            modificaBits(&puerto->CRL,mascaraL,config & ~(uint32_t)1);
            modificaBits(&puerto->CRH,mascaraH,config & ~(uint32_t)1);
        }
    }
    return 0;
}
