#include "gpio.h"
#include "tempo_ms.h"

enum {PERIODO_LOOP = 1300, PERIODO_INT = 800};

static void Parpadeo_ejecuta(Accion *a);

static struct Parpadeo{
    Accion accion;
    uint32_t t0;
    uint32_t semiperiodo;
}parpadeo = {.accion.ejecuta=&Parpadeo_ejecuta, .semiperiodo = PERIODO_INT};


typedef struct PasoAPaso{
    Bus *conexion;
    unsigned cuenta;
    bool medioPaso;
}PasoAPaso;

/**
 * @brief Inicializa control de motor paso a paso
 * 
 * @param pap El motor
 * @param bus El bus de conexion (4 bit)
 * @param medioPaso Secuencia de medio paso si verdadero, paso completo si falso
 */
void PAP_inicia(PasoAPaso *pap, Bus *bus, bool medioPaso);
/**
 * @brief Avanza un motor paso a paso en un paso
 * 
 * @param pap El motor
 */
void PAP_retrocede(PasoAPaso *pap);
/**
 * @brief Retrocede un motor paso a paso en un paso
 * 
 * @param pap El motor
 */
void PAP_avanza(PasoAPaso *pap);

int main(void)
{
    static Bus conexionMotor = INICIALIZA_VARIABLE_BUS(PULL_UP,PUSH_PULL,V_BAJA,
                      PB6,PB7,PB8,PB9);
    static Bus b1 = INICIALIZA_VARIABLE_BUS(FLOTANTE,PUSH_PULL,V_BAJA,
        PA3,PA4,PA5,PA6);
    static PasoAPaso pap;

    PAP_inicia(&pap,&conexionMotor,true);
    Tempo_inicializa();
    Tempo_ponAccionMilisegundo(&parpadeo.accion);
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);
    uint32_t t0=Tempo_obtMilisegundos();
    uint32_t t1=t0;
    enum {D_AVANCE,D_RETROCESO} dir =D_AVANCE;
    for(unsigned i=0;;++i){
        const uint32_t t = Tempo_obtMilisegundos();
        if (t-t0 >= PERIODO_LOOP){
            Pin_conmuta(PIN_LED);
            Bus_escribe(&b1,i);
            t0=t;
            dir = dir==D_AVANCE? D_RETROCESO : D_AVANCE;
        }
        if(t-t1 >= 1){
            if (dir==D_AVANCE){
                PAP_avanza(&pap);
            }else{
                PAP_retrocede(&pap);
            }
            t1=t;
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


static const uint8_t secuenciaMedioPaso[8] = {
    0b1000,0b1100,
    0b0100,0b0110,
    0b0010,0b0011,
    0b0001,0b1001
};


static void PAP_actualiza(PasoAPaso *pap)
{
    unsigned const valor = secuenciaMedioPaso[pap->cuenta & 0x7];
    Bus_escribe(pap->conexion,valor);
}

void PAP_inicia(PasoAPaso *pap, Bus *bus, bool medioPaso)
{
    pap->conexion  = bus;
    pap->medioPaso = medioPaso;
    pap->cuenta    = 0;
    PAP_actualiza(pap);
}

void PAP_retrocede(PasoAPaso *pap)
{
    pap->cuenta -= pap->medioPaso ? 1:2;
    PAP_actualiza(pap);    
}
void PAP_avanza(PasoAPaso *pap)
{
    pap->cuenta += pap->medioPaso ? 1:2;
    PAP_actualiza(pap);
}
