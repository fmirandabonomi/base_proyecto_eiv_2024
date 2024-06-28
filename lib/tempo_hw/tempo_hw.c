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


uint32_t TempoHW_configModoEncoder(
    HTempoHW tempo, ModoEncoder m,
    uint32_t fs, FiltroEntrada filt, Polaridades pol)
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
    if (!regTempo) return TempoHW_ERROR_CONFIG;
    uint32_t polaridades = (((pol>>1) & 1) << TIM_CCER_CC1P_Pos )
                          |((pol & 1) << TIM_CCER_CC2P_Pos);
    modificaBits(&regTempo->CCER,MASCARA_HAB_Y_POL,polaridades);
    uint32_t div_dts = 0;
    uint32_t div_samp = 0;
    if (filt == FE_NINGUNO){
        if (frecuenciaBase <= fs){
            frecuenciaMuestreo = frecuenciaBase;
            div_dts = 1;
            div_samp = 1;
        }else{
            const uint32_t optimo = (frecuenciaBase + fs/2)/fs;
            
        }
    }
    estableceBits(&regTempo->CCER,MASCARA_HAB);
}