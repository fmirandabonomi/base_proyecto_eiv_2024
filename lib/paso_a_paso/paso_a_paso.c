#include "paso_a_paso.h"
#include <stdint.h>

static const uint8_t secuenciaMedioPaso[8] = {
    0b1000,0b1100,
    0b0100,0b0110,
    0b0010,0b0011,
    0b0001,0b1001
};

static void PAP_actualiza(PAP *pap)
{
    if(!pap->actualiza) return;
    int const valor = secuenciaMedioPaso[pap->cuenta & 0x7];
    pap->actualiza(valor);
}

int PAP_inicializa(PAP *pap, PAP_ActualizaSalidas actualiza, bool medioPaso)
{
    if (!actualiza) return -1;
    pap->actualiza = actualiza;
    pap->medioPaso = medioPaso;
    pap->cuenta = 0;
    PAP_actualiza(pap);
    return 0;
}

void PAP_retrocede(PAP *pap)
{
    pap->cuenta -= pap->medioPaso ? 1:2;
    PAP_actualiza(pap);    
}
void PAP_avanza(PAP *pap)
{
    pap->cuenta += pap->medioPaso ? 1:2;
    PAP_actualiza(pap);
}

