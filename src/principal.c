#include "gpio.h"
#include "tempo_hw.h"
#include "tempo_ms.h"

int main(void)
{
    enum{TENCODER = THW3};
    Tempo_inicializa();
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);
    Pin_configuraEntrada(PA6,PULL_UP);
    Pin_configuraEntrada(PA7,PULL_UP);
    TempoHW_configModoEncoder(TENCODER,ME_T1,10000,FE_LARGO,Polaridades_NN,2);
    TempoHW_enciendeContador(TENCODER);
    uint16_t pe0 = TempoHW_obtCuenta(TENCODER);
    int32_t tLED = 500;
    for(unsigned i=0;;++i){
        const uint16_t pe = TempoHW_obtCuenta(TENCODER);
        if (pe0 != pe){
            tLED += (int16_t)(pe-pe0); 
            if (tLED < 1 ) tLED = 1;
            pe0=pe;
        }
        Pin_conmuta(PIN_LED);
        Tempo_esperaMilisegundos(tLED);
    }
    return 0;
}
