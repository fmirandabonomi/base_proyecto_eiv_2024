#include <tempo_hw.h>
#include <stm32f1xx.h>
#include <stdbool.h>
#include <stddef.h>

enum{PERIODO_MAX = 65535};

typedef enum TempoHW_RelojMuestreo{
    THW_MUESTREO_FCK,
    THW_MUESTREO_DTS_DIV_2,
    THW_MUESTREO_DTS_DIV_4,
    THW_MUESTREO_DTS_DIV_8,
    THW_MUESTREO_DTS_DIV_16,
    THW_MUESTREO_DTS_DIV_32,
    TempoHW_RelojMuestreo_NO_VALIDO
}TempoHW_RelojMuestreo;

typedef enum TempoHW_ModoComparacion{
    THWMC_CONGELADO,
    THWMC_PON_ALTO_AL_COINCIDIR,
    THWMC_PON_BAJO_AL_COINCIDIR,
    THWMC_CONMUTA_AL_COINCIDIR,
    THWMC_FUERZA_BAJO,
    THWMC_FUERZA_ALTO,
    THWMC_PWM_MODO_1,
    THWMC_PWM_MODO_2,
    TempoHW_ModoComparacion_NO_VALIDO
}TempoHW_ModoComparacion;

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
inline static void limpiaBits(volatile uint32_t *registro,uint32_t bits)
{
    bool fallo;
    do{
        const uint32_t anterior = __LDREXW(registro);
        fallo = __STREXW(anterior & ~bits,registro);
    }while(fallo);
}

__attribute__((__always_inline__))
static inline void enciendeCanal(
    TIM_TypeDef *const regTempo,TempoHW_Canal const canal)
{
    estableceBits(&regTempo->CCER,1 << (4*canal));
}
__attribute__((__always_inline__))
static inline void apagaCanal(
    TIM_TypeDef *const regTempo,TempoHW_Canal const canal)
{
    limpiaBits(&regTempo->CCER,1 << (4*canal));
}

__attribute__((__always_inline__))
static inline void apagaContador(
    TIM_TypeDef *const regTempo)
{
    limpiaBits(&regTempo->CR1,TIM_CR1_CEN);
}
__attribute__((__always_inline__))
static inline void enciendeContador(
    TIM_TypeDef *const regTempo)
{
    estableceBits(&regTempo->CR1,TIM_CR1_CEN);
}


static void enciendeRelojPeriferico(TempoHW t);
static uint32_t relojTimer(const TempoHW t);
static TIM_TypeDef *registrosTimer(const TempoHW t);
static uint8_t exponenteDivisorMasCercano(const uint32_t divisorOptimo);
static void configuraCanal(
    TIM_TypeDef    *const regTempo,
    TempoHW_Canal   const canal,
    uint32_t        const config);
static uint32_t calculaConfigCaptura(
    TempoHW_FiltroEntrada       const filt, 
    TempoHW_RelojMuestreo       const relojMuestreo, 
    TempoHW_Entrada             const entrada,
    TempoHW_PrescalerCaptura    const prescaler);
static uint32_t calculaConfigCompara(
    bool                    const bajoConFlancoDeEtrFiltrada,
    TempoHW_ModoComparacion const modo,
    bool                    const cambiaValorComparacionSincronizadoConEvento,
    bool                    const efectoRapidoDisparoEnPWM);
static void configuraCuenta(
    TIM_TypeDef        *const regTempo,
    TempoHW_ModoCuenta  const modo,
    uint32_t            const limite);
static void configuraAccionCanal(
    TempoHW         const tempo,
    TempoHW_Canal   const canal,
    Accion         *const accion);


static void enciendeRelojPeriferico(TempoHW t)
{
    switch(t){
    case TempoHW_1:
        estableceBits(&RCC->APB2ENR,RCC_APB2ENR_TIM1EN);
    break;case TempoHW_2:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM2EN);
    break;case TempoHW_3:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM3EN);
    break;case TempoHW_4:
        estableceBits(&RCC->APB1ENR,RCC_APB1ENR_TIM4EN);
    break;default:
    break;
    }
}

/**
 * @brief Calcula la frecuencia de reloj de un periférico timer
 * 
 * @param t El timer
 * @return uint32_t La frecuencia en Hz
 */
static uint32_t relojTimer(const TempoHW t)
{
    SystemCoreClockUpdate();
    uint32_t reloj = SystemCoreClock;
    const int posPpre = (t==TempoHW_1)? RCC_CFGR_PPRE2_Pos : RCC_CFGR_PPRE1_Pos;
    const uint32_t ppre = (RCC->CFGR >> posPpre)&0b111;
    if (ppre & (1<<2)){
        reloj >>= ((ppre&0b11)+1) - 1;
    }
    return reloj;
}

static TIM_TypeDef *registrosTimer(const TempoHW t)
{
    switch(t){
    case TempoHW_1: return TIM1;
    case TempoHW_2: return TIM2;
    case TempoHW_3: return TIM3;
    case TempoHW_4: return TIM4;
    default: return NULL;
    }
}
uint32_t TempoHW_configReloj(const TempoHW tempo,const uint32_t frec)
{
    enum{MAX_PSC = (uint32_t)1 << 16};
    enciendeRelojPeriferico(tempo);
    const uint32_t frecuenciaPrescaler = relojTimer(tempo);
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIGURACION;
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

static uint8_t exponenteDivisorMasCercano(const uint32_t divisorOptimo)
{
    const uint8_t exponente = 31 - __CLZ(divisorOptimo);
    const uint32_t v1 = (uint32_t)1 << exponente; 
    const uint32_t v2 = (uint32_t)1 << (exponente+1);
    const uint32_t err1 = divisorOptimo - v1;
    const uint32_t err2 = v2 - divisorOptimo;
    return (err1<err2) ? exponente : exponente + 1;
}

uint32_t TempoHW_configModoEncoder(
    TempoHW tempo, TempoHW_ModoEncoder m,
    uint32_t fs, TempoHW_FiltroEntrada filt, TempoHW_PolaridadesEncoder pol, uint16_t pasosPorCuenta)
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
    if (!regTempo | !pasosPorCuenta) return TempoHW_ERROR_CONFIGURACION;
    enciendeRelojPeriferico(tempo);

    const uint32_t polaridades = (((pol>>1) & 1) << TIM_CCER_CC1P_Pos )
                                |((pol & 1) << TIM_CCER_CC2P_Pos);
    uint32_t expDivDts = 0;
    uint32_t expDivSamp = 0;
    
    const uint32_t divisorOptimo = (frecuenciaBase + fs/2)/fs;
    uint8_t exponente = exponenteDivisorMasCercano(divisorOptimo);
    if (exponente > 7) exponente = 7;

    if (filt == THWFiltroEntrada_NINGUNO){
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
        bitsICF = (filt == THWFiltroEntrada_NINGUNO)? 0b0000:
                  (filt == THWFiltroEntrada_CORTO)?   0b0001:
                  (filt == THWFiltroEntrada_MEDIO)?   0b0010:
                                        0b0011;
    break;case 1:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b0100 : 0b0101;
    break;case 2:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b0110 : 0b0111;
    break;case 3:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1000 : 0b1001;
    break;case 4:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1010 :
                  (filt == THWFiltroEntrada_MEDIO)? 0b1011 :
                                      0b1100;
    break;default: // 5
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1101:
                  (filt == THWFiltroEntrada_MEDIO)? 0b1110:
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

uint32_t TempoHW_enciendeContador(TempoHW tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIGURACION;
    enciendeRelojPeriferico(tempo);
    enciendeContador(regTempo);
    return 0;
}

uint32_t TempoHW_apagaContador(TempoHW tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIGURACION;
    apagaContador(regTempo);
    return 0;
}

uint32_t TempoHW_obtCuenta(TempoHW tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo) return TempoHW_ERROR_CONFIGURACION;
    return regTempo->CNT;
}

static void configuraCanal(
    TIM_TypeDef    *const regTempo,
    TempoHW_Canal   const canal,
    uint32_t        const config)
{
    volatile uint32_t *const CCMRx = (canal < THWCanal_3) ? &regTempo->CCMR1 :
                                                        &regTempo->CCMR2;
    enum{NBITS_MODO = 8UL, MASCARA_MODO = (1UL << NBITS_MODO) - 1};
    const int offsetModo = (canal & 1) * NBITS_MODO;
    modificaBits(CCMRx,MASCARA_MODO << offsetModo, config << offsetModo);
}


static uint32_t calculaConfigCaptura(
    TempoHW_FiltroEntrada       const filt, 
    TempoHW_RelojMuestreo       const relojMuestreo, 
    TempoHW_Entrada             const entrada,
    TempoHW_PrescalerCaptura    const prescaler)
{
    uint32_t bitsICF;
    switch(relojMuestreo){
    case THW_MUESTREO_FCK:
        bitsICF = (filt == THWFiltroEntrada_NINGUNO)? 0b0000:
                  (filt == THWFiltroEntrada_CORTO)?   0b0001:
                  (filt == THWFiltroEntrada_MEDIO)?   0b0010:
                                        0b0011;
    break;case THW_MUESTREO_DTS_DIV_2:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b0100 : 0b0101;
    break;case THW_MUESTREO_DTS_DIV_4:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b0110 : 0b0111;
    break;case THW_MUESTREO_DTS_DIV_8:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1000 : 0b1001;
    break;case THW_MUESTREO_DTS_DIV_16:
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1010 :
                  (filt == THWFiltroEntrada_MEDIO)? 0b1011 :
                                      0b1100;
    break;default: // THW_MUESTREO_DTS_DIV_32
        bitsICF = (filt == THWFiltroEntrada_CORTO)? 0b1101:
                  (filt == THWFiltroEntrada_MEDIO)? 0b1110:
                                      0b1111;
    }
    const uint32_t bitsCCS = (entrada + 1)&3;
    const uint32_t bitsPSC = prescaler;
    return  (bitsCCS << TIM_CCMR1_CC1S_Pos)
           |(bitsPSC << TIM_CCMR1_CC1S_Pos)
           |(bitsICF << TIM_CCMR1_IC1F_Pos);
}


static uint32_t calculaConfigCompara(
    bool                    const bajoConFlancoDeEtrFiltrada,
    TempoHW_ModoComparacion const modo,
    bool                    const cambiaValorComparacionSincronizadoConEvento,
    bool                    const efectoRapidoDisparoEnPWM)
{
    return (((uint32_t)modo & 7UL) << TIM_CCMR1_OC1M_Pos)
          |(bajoConFlancoDeEtrFiltrada ? TIM_CCMR1_OC1CE : 0)
          |(cambiaValorComparacionSincronizadoConEvento  ?
                                         TIM_CCMR1_OC1PE : 0)
          |(efectoRapidoDisparoEnPWM   ? TIM_CCMR1_OC1FE : 0);
}

static void configuraCuenta(
    TIM_TypeDef        *const regTempo,
    TempoHW_ModoCuenta  const modo,
    uint32_t            const limite)
{
    enum {MASCARA_CONFIG = TIM_CR1_CMS | TIM_CR1_DIR | TIM_CR1_CEN};
    apagaContador(regTempo);
    static const uint32_t config[]={
    [THWModoCuenta_ASCENDENTE]                      = 0 << TIM_CR1_DIR_Pos,
    [THWModoCuenta_DESCENDENTE]                     = 1 << TIM_CR1_DIR_Pos,
    [THWModoCuenta_ASCENDENTE_DESCENDENTE_ICC]      = 0b01 << TIM_CR1_CMS_Pos,
    [THWModoCuenta_ASCENDENTE_ICC_DESCENDENTE]      = 0b10 << TIM_CR1_CMS_Pos,
    [THWModoCuenta_ASCENDENTE_ICC_DESCENDENTE_ICC]  = 0b11 << TIM_CR1_CMS_Pos
    };
    regTempo->ARR = limite;
    modificaBits(&regTempo->CR1,MASCARA_CONFIG,config[modo] | TIM_CR1_CEN);
}

uint32_t TempoHW_configPwm(
    TempoHW             const tempo,
    TempoHW_Canal       const canal,
    uint32_t            const cuentaMaxima,
    uint32_t            const umbral,
    TempoHW_ModoPWM     const modoPwm,
    TempoHW_ModoCuenta  const modoCuenta,
    Accion             *const accion)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (   !regTempo 
        || canal >= THWCanal_NO_VALIDO
        || cuentaMaxima > PERIODO_MAX 
        || umbral > PERIODO_MAX
        || modoPwm >= THWModoPWM_NO_VALIDO
        || modoCuenta >= THWModoCuenta_NO_VALIDO)
        return TempoHW_ERROR_CONFIGURACION;
    volatile uint32_t *CCRs = &regTempo->CCR1;
    configuraCuenta(regTempo,modoCuenta,cuentaMaxima);    
    apagaCanal(regTempo,canal);
    configuraCanal(regTempo,canal,
        calculaConfigCompara(false, 
            (modoPwm == THWModoPWM_ACTIVO_IZQUIERDA) ? THWMC_PWM_MODO_1:
                                                     THWMC_PWM_MODO_2,
                            false,
                            false));
    CCRs[canal] = umbral;
    regTempo->ARR = cuentaMaxima;
    enciendeCanal(regTempo,canal);
    configuraAccionCanal(tempo,canal,accion);
    return 0;
}

uint32_t TempoHW_ponUmbralComparacion(
    TempoHW       const tempo,
    TempoHW_Canal const canal,
    uint32_t      const umbral)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (   !regTempo 
        || canal >= THWCanal_NO_VALIDO
        || umbral > PERIODO_MAX)
        return TempoHW_ERROR_CONFIGURACION;
    volatile uint32_t *CCRs = &regTempo->CCR1;
    CCRs[canal] = umbral;
    return umbral;
}


uint32_t TempoHW_configCaptura(
    TempoHW tempo,
    TempoHW_Canal canal,
    TempoHW_Entrada entrada,
    TempoHW_FiltroEntrada filtro,
    uint32_t frecMuestreoEntrada,
    TempoHW_Flanco flanco,
    TempoHW_PrescalerCaptura flancosPorCaptura,
    Accion* accion)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (   !regTempo
        || canal >= THWCanal_NO_VALIDO
        || entrada >= THWEntrada_NO_VALIDO
        || filtro >= THWFiltroEntrada_NO_VALIDO
        || flanco >= THWFlanco_NO_VALIDO
        || flancosPorCaptura >= THWPrescalerCaptura_NO_VALIDO)
        return TempoHW_ERROR_CONFIGURACION;
    apagaCanal(regTempo, canal);
    const uint32_t frecBase = relojTimer(tempo);
    const uint8_t expDiv = 
        exponenteDivisorMasCercano(frecBase/frecMuestreoEntrada);
    const uint8_t expDivDts = (regTempo->CR1 & TIM_CR1_CKD) >> TIM_CR1_CKD_Pos;
    int expDivRestante = expDiv - expDivDts;
    const TempoHW_RelojMuestreo relojMuestreo = 
        (expDivRestante < 1) ? THW_MUESTREO_FCK:
        (expDivRestante < 5) ? expDivRestante + THW_MUESTREO_FCK:
                               THW_MUESTREO_DTS_DIV_32;
    configuraCanal(regTempo,canal,
        calculaConfigCaptura(filtro,
                             relojMuestreo,
                             entrada,
                             flancosPorCaptura));
    configuraAccionCanal(tempo,canal,accion);
    enciendeCanal(regTempo,canal);
    return (relojMuestreo == THW_MUESTREO_FCK) ? frecBase :
                                                 frecBase 
                                                    >> (expDivDts 
                                                        + relojMuestreo);
}


uint32_t TempoHW_configuraCuenta(
    TempoHW             const tempo,
    TempoHW_ModoCuenta  const modo,
    uint32_t            const limite)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (   !regTempo
        || modo >= THWModoCuenta_NO_VALIDO
        || limite > PERIODO_MAX)
        return TempoHW_ERROR_CONFIGURACION;
    configuraCuenta(regTempo,modo,limite);
    return (   modo == THWModoCuenta_DESCENDENTE 
            || modo == THWModoCuenta_ASCENDENTE  )? limite+1 : 2*limite;
}


/* Atención de Interrupciones  */

static struct AccionesTempoHW{
    Accion *disparo;
    Accion *actualizacion;
    Accion *canal[THWCanal_NO_VALIDO];
}acciones[TempoHW_NO_VALIDO]={};

static IRQn_Type obtIrqCC(TempoHW tempo){
    switch (tempo)
    {
    case TempoHW_1: return TIM1_CC_IRQn;
    case TempoHW_2: return TIM2_IRQn;
    case TempoHW_3: return TIM3_IRQn;
    case TempoHW_4: return TIM4_IRQn;
    default: for(;;);
    }
}


static void configuraAccionCanal(
    TempoHW         const tempo,
    TempoHW_Canal   const canal,
    Accion         *const accion)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    if (!regTempo || canal >= THWCanal_NO_VALIDO) return;
    acciones[tempo].canal[canal] = accion;
    IRQn_Type irqn = obtIrqCC(tempo);
    if (!accion){
        limpiaBits(&regTempo->DIER,TIM_DIER_CC1IE << canal);
    }else{
        estableceBits(&regTempo->DIER,TIM_DIER_CC1IE << canal);
        NVIC_EnableIRQ(irqn);
    }
}

static void manejaInterrupcionesCC(TempoHW const tempo)
{
    TIM_TypeDef *const regTempo = registrosTimer(tempo);
    Accion **const accionCanal = acciones[tempo].canal;
    if (!regTempo) return; 
    for (TempoHW_Canal c = 0 ; c < THWCanal_NO_VALIDO;++c){
        const uint32_t bandera = TIM_SR_CC1IF << c;
        const bool peticion = regTempo->SR & bandera;
        if (!peticion) continue;
        else regTempo->SR = ~bandera;
        Accion *const accion = accionCanal[c];
        if (accion) Accion_ejecuta(accion);
    }
}

void TIM1_CC_IRQHandler(void)
{
    manejaInterrupcionesCC(TempoHW_1);
}

void TIM2_IRQHandler(void)
{
    manejaInterrupcionesCC(TempoHW_2);
}

void TIM3_IRQHandler(void)
{
    manejaInterrupcionesCC(TempoHW_3);
}

void TIM4_IRQHandler(void)
{
    manejaInterrupcionesCC(TempoHW_4);
}