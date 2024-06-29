#include <tempo_hw.h>
#include <stm32f1xx.h>
#include <stdbool.h>
#include <stddef.h>

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

static void enciendeRelojPeriferico(HTempoHW t)
{
    switch(t){
    case THW1:
        estableceBits(&RCC->APB2ENR,RCC_APB2ENR_TIM1EN);
    break;case THW2:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM2EN);
    break;case THW3:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM3EN);
    break;case THW4:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM4EN);
    break;default:
    break;
    }
}

static uint32_t relojTimer(const HTempoHW t)
{
    SystemCoreClockUpdate();
    uint32_t reloj = SystemCoreClock;
    const int posPpre = (t==THW1)? RCC_CFGR_PPRE2_Pos : RCC_CFGR_PPRE1_Pos;
    const uint32_t ppre = (RCC->CFGR >> posPpre)&0b111;
    if (ppre & (1<<2)){
        reloj >>= ((ppre&0b11)+1) - 1;
    }
    return reloj;
}

static TIM_TypeDef *registrosTimer(const HTempoHW t)
{
    switch(t){
    case THW1: return TIM1;
    case THW2: return TIM2;
    case THW3: return TIM3;
    case THW4: return TIM4;
    default: return NULL;
    }
}
uint32_t TempoHW_configReloj(const HTempoHW tempo,const uint32_t frec)
{
    enum{MAX_PSC = (uint32_t)1 << 16};
    enciendeRelojPeriferico(tempo);
    const uint32_t frecuenciaPrescaler = relojTimer(tempo);
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIG;
    uint32_t frecuenciaTimer;
    if (frecuenciaPrescaler <= frec){
        frecuenciaTimer = frecuenciaPrescaler;
        regTempo->PSC = 0;
    }else{
        uint32_t psc = (frecuenciaPrescaler + frec/2)/frec;
        if (psc > MAX_PSC) psc = MAX_PSC;
        frecuenciaTimer = frecuenciaPrescaler/psc;
        regTempo->PSC = psc-1;
    }
    return frecuenciaTimer;
}

static uint8_t exponenteDivisorMasCercano(const uint32_t divisorOptimo){
    const uint8_t exponente = 32 - __CLZ(divisorOptimo);
    const uint32_t v1 = (uint32_t)1 << exponente; 
    const uint32_t v2 = (uint32_t)1 << (exponente+1);
    const uint32_t err1 = divisorOptimo - v1;
    const uint32_t err2 = v2 - divisorOptimo;
    return (err1<err2) ? exponente : exponente + 1;
}

uint32_t TempoHW_configModoEncoder(
    HTempoHW tempo, ModoEncoder m,
    uint32_t fs, FiltroEntrada filt, Polaridades pol, uint16_t pasosPorCuenta)
{
    enum{
        CKD_DIV1 = 0b00 << TIM_CR1_CKD_Pos,
        CKD_DIV2 = 0b01 << TIM_CR1_CKD_Pos,
        CKD_DIV4 = 0b10 << TIM_CR1_CKD_Pos,
        MASCARA_HAB = TIM_CCER_CC1E_Msk | TIM_CCER_CC2E_Msk,
        MASCARA_HAB_Y_POL = TIM_CCER_CC1P_Msk | TIM_CCER_CC2P_Msk
    };
    const uint32_t frecuenciaBase = relojTimer(tempo);
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    uint32_t frecuenciaMuestreo;
    if (!regTempo | !pasosPorCuenta) return TempoHW_ERROR_CONFIG;
    enciendeRelojPeriferico(tempo);

    const uint32_t polaridades = (((pol>>1) & 1) << TIM_CCER_CC1P_Pos )
                                |((pol & 1) << TIM_CCER_CC2P_Pos);
    uint32_t expDivDts = 0;
    uint32_t expDivSamp = 0;
    
    const uint32_t divisorOptimo = (frecuenciaBase + fs/2)/fs;
    uint8_t exponente = exponenteDivisorMasCercano(divisorOptimo);
    if (exponente > 7) exponente = 7;

    if (filt == FE_NINGUNO){
        expDivDts = exponente < 2 ? exponente : 2;
        expDivSamp = 0;
    }else{
        if (exponente <= 2){
            expDivDts = 0;
            expDivSamp = exponente;
        }else{
            expDivDts = 2;
            expDivSamp = exponente - 2;
        }
    }
    unsigned bitsICF;
    switch(expDivSamp){
    case 0:
        bitsICF = (filt == FE_NINGUNO)? 0b0000:
                  (filt == FE_CORTO)?   0b0001:
                  (filt == FE_MEDIO)?   0b0010:
                                        0b0011;
    break;case 1:
        bitsICF = (filt == FE_CORTO)? 0b0100 : 0b0101;
    break;case 2:
        bitsICF = (filt == FE_CORTO)? 0b0110 : 0b0111;
    break;case 3:
        bitsICF = (filt == FE_CORTO)? 0b1000 : 0b1001;
    break;case 4:
        bitsICF = (filt == FE_CORTO)? 0b1010 :
                  (filt == FE_MEDIO)? 0b1011 :
                                      0b1100;
    break;default: // 5
        bitsICF = (filt == FE_CORTO)? 0b1101:
                  (filt == FE_MEDIO)? 0b1110:
                                      0b1111;
    }
    unsigned const bitsCKD = expDivDts;
    unsigned const bitsSMS = (m+1)&3; 

    regTempo->CR1 = (bitsCKD << TIM_CR1_CKD_Pos);
    regTempo->CR2 = 0;
    regTempo->SMCR = (bitsSMS << TIM_SMCR_SMS_Pos);
    regTempo->CCER = polaridades;
    regTempo->CCMR1 = (bitsICF << TIM_CCMR1_IC1F_Pos) 
                     |(bitsICF << TIM_CCMR1_IC2F_Pos)
                     |(0b01 << TIM_CCMR1_CC1S_Pos)
                     |(0b01 << TIM_CCMR1_CC2S_Pos);
    regTempo->PSC = pasosPorCuenta-1;
    regTempo->CCER |= MASCARA_HAB;
    frecuenciaMuestreo = frecuenciaBase >> (expDivDts + expDivSamp);
    return frecuenciaMuestreo;
}

uint32_t TempoHW_enciendeContador(HTempoHW tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIG;
    enciendeRelojPeriferico(tempo);
    estableceBits(&regTempo->CR1,TIM_CR1_CEN);
    return 0;
}

uint32_t TempoHW_obtCuenta(HTempoHW tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIG;
    return regTempo->CNT;
}