#include "gpio.h"
#include "tempo_hw.h"
#include "tempo_ms.h"

enum {PERIODO_INT = 800};

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
    enum{T_ENCODER=THW3};
    static Bus conexionMotor = INICIALIZA_VARIABLE_BUS(PULL_UP,PUSH_PULL,V_BAJA,
                      PB6,PB7,PB8,PB9);
    static PasoAPaso pap;

    Tempo_inicializa();
    Tempo_ponAccionMilisegundo(&parpadeo.accion);

    PAP_inicia(&pap,&conexionMotor,true);
    
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
