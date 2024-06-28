#include "gpio.h"
#include "tempo_ms.h"

enum {PERIODO_LOOP = 800, PERIODO_INT = 500};

static void Parpadeo_ejecuta(Accion *a);

static struct Parpadeo{
    Accion accion;
    uint32_t t0;
    uint32_t semiperiodo;
}parpadeo = {.accion.ejecuta=&Parpadeo_ejecuta, .semiperiodo = PERIODO_INT};

int main(void)
{
    static Bus b1 = INICIALIZA_VARIABLE_BUS(FLOTANTE,PUSH_PULL,V_BAJA,
        PA3,PA4,PA5,PA6);
    Tempo_inicializa();
    Tempo_ponAccionMilisegundo(&parpadeo.accion);
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);
    for(unsigned i=0;;++i){
        Tempo_esperaMilisegundos(PERIODO_LOOP);
        Pin_conmuta(PIN_LED);
        Bus_escribe(&b1,i);
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