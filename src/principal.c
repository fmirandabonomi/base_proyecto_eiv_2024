#include "gpio.h"
#include "tempo_ms.h"

enum {TIEMPO = 500};

int main(void)
{
    // Para el bus de 5 bits [PB12,PB13,PB14,PB15,PA8]
    // Bus_lee ~195 ciclos
    // Bus_escribe ~204 ciclos
    // Cambio de modo ~114 ciclos
    // Partiendo de modo escritura, Bus_lee seguido de Bus_escribe ~627 ciclos
    static Bus b1 = INICIALIZA_VARIABLE_BUS(PULL_UP,PUSH_PULL,V_BAJA,
        PB12,PB13,PB14,PB15,PA8);
    Tempo_inicializa();
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);
    unsigned aux;
    uint32_t t0 = Tempo_obtMilisegundos() - TIEMPO;
    for(;;){
        const uint32_t t = Tempo_obtMilisegundos();
        if(t-t0 >= TIEMPO){
            Bus_escribe(&b1,0x3C);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0xC3);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x3C);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0xC3);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x3C);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0xC3);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x3C);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0xC3);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_lee(&b1,&aux);
            Bus_escribe(&b1,0x0);
            Bus_escribe(&b1,0xFF);
            Bus_escribe(&b1,0x0);
           Pin_conmuta(PIN_LED);
            t0 = t;
        }
    }
    return 0;
}