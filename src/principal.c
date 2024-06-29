#include "gpio.h"
#include "tempo_hw.h"
#include "tempo_ms.h"
#include "paso_a_paso.h"

enum {PERIODO_INT = 800};

static void Parpadeo_ejecuta(Accion *a);

static struct Parpadeo{
    Accion accion;
    uint32_t t0;
    uint32_t semiperiodo;
}parpadeo = {.accion.ejecuta=&Parpadeo_ejecuta, .semiperiodo = PERIODO_INT};


static void salidaPAP(int abcd);

int main(void)
{
    enum{T_ENCODER=THW3};
    PAP pap;

    Tempo_inicializa();
    Tempo_ponAccionMilisegundo(&parpadeo.accion);

    PAP_inicializa(&pap,salidaPAP,true);
    
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);

    Pin_configuraEntrada(PA6,PULL_UP); // Timer 3 canal 1
    Pin_configuraEntrada(PA7,PULL_UP); // Timer 3 canal 2
    TempoHW_configModoEncoder(T_ENCODER,ME_T1,10000,FE_LARGO,Polaridades_NN,2);
    TempoHW_enciendeContador(T_ENCODER);
    
    uint16_t p0 = TempoHW_obtCuenta(T_ENCODER);
    uint32_t t0 = Tempo_obtMilisegundos();
    int diferencia = 0;
    for(;;){
        const uint16_t p = TempoHW_obtCuenta(T_ENCODER);
        const uint32_t t = Tempo_obtMilisegundos(); 
        if (p!= p0){
            diferencia += 16*(int16_t)(p-p0);
            p0=p;
        }
        if(t-t0 >= 5 && diferencia != 0){
            if (diferencia > 0){
                PAP_avanza(&pap);
                --diferencia;
            }else{
                PAP_retrocede(&pap);
                ++diferencia;
            }
            t0=t;
        }
    }
    return 0;
}

static void Parpadeo_ejecuta(Accion *a)
{
    struct Parpadeo *const p = (struct Parpadeo*)a; 
    const uint32_t t = Tempo_obtMilisegundos();
    if ( t - p->t0 >= p->semiperiodo){
        Pin_conmuta(PIN_LED);
        p->t0 = t;
    }
}


static void salidaPAP(int abcd)
{
    static Bus conexionMotor = INICIALIZA_VARIABLE_BUS(PULL_UP,PUSH_PULL,V_BAJA,
                    PB6,PB7,PB8,PB9);

    Bus_escribe(&conexionMotor,abcd);
}