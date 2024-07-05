#include "gpio.h"
#include "tempo_hw.h"
#include "tempo_ms.h"
#include "paso_a_paso.h"
#include "serie.h"

enum {PERIODO_INT = 800};

static void Parpadeo_ejecuta(AccionParam *a, void *tp);

static struct Parpadeo{
    AccionParam accion;
    uint32_t t0;
    uint32_t semiperiodo;
}parpadeo = {.accion.ejecuta=&Parpadeo_ejecuta, .semiperiodo = PERIODO_INT};


static void salidaPAP(int abcd);

int main(void)
{
    enum{T_ENCODER=TempoHW_3};
    PAP pap;
    Serie_init(9600);

    Tempo_inicializa();
    Tempo_ponAccionMilisegundo(&parpadeo.accion);

    PAP_inicializa(&pap,salidaPAP,true);
    
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);

    Pin_configuraEntrada(PA6,PULL_UP); // Timer 3 canal 1
    Pin_configuraEntrada(PA7,PULL_UP); // Timer 3 canal 2
    // Son posibles las frecuencias de muestreo que sean fracciones potencia de
    // dos del reloj del bus periférico (8 MHz en este caso) con exponentes
    // entre 0 y 7
    // 8000000, 4000000, 2000000, 1000000, 500000, 250000, 125000, 62500
    uint32_t frecuenciaMuestreo = TempoHW_configModoEncoder(
        /*temporizador*/            T_ENCODER,
        /*flancos detectados*/      THWModoEncoder_T1,
        /*frecuencia de muestreo*/  125000,
        /*filtro de entrada*/       THWFiltroEntrada_LARGO,
        /*polaridades del encoder*/ THWPolaridadesEncoder_NN,
        /*flancos por cuenta*/      2);
    Serie_enviaCadena("Frecuencia de muestreo encoder ");
    Serie_enviaEntero(frecuenciaMuestreo);
    Serie_enviaNuevaLinea();

    TempoHW_enciendeContador(T_ENCODER);
    
    uint16_t p0 = TempoHW_obtCuenta(T_ENCODER);
    uint32_t t0 = Tempo_obtMilisegundos();
    int diferencia = 0;
    for(;;){
        const uint16_t p = TempoHW_obtCuenta(T_ENCODER);
        const uint32_t t = Tempo_obtMilisegundos(); 
        if (p!= p0){
            diferencia += 5*(int16_t)(p-p0);
            p0=p;
        }
        if(t-t0 >= 10 && diferencia != 0){
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

static void Parpadeo_ejecuta(AccionParam *a,void *tp)
{
    struct Parpadeo *const p = (struct Parpadeo*)a;
    const uint32_t t = *(uint32_t*)tp;
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