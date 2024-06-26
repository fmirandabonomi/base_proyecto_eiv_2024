#include "tempo_ms.h"
#include <stm32f1xx.h>
#include <stddef.h>

static volatile uint32_t ticks;

void Tempo_inicializa(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);
}

uint32_t Tempo_obtMilisegundos(void)
{
    return ticks;
}

void Tempo_esperaMilisegundos(uint32_t tiempo)
{
    const uint32_t inicial = ticks;
    while(ticks-inicial < tiempo);
}

static Accion *accionMilisegundo = NULL;

void SysTick_Handler(void)
{
    ++ticks;
    if(accionMilisegundo){
        Accion_ejecuta(accionMilisegundo);
    }    
}


int Tempo_ponAccionMilisegundo(Accion *accion)
{
    if (accionMilisegundo || !accion) return -1;
    accionMilisegundo = accion;
    return 0;
}
void Tempo_eliminaAccionMilisegundo(void)
{
    accionMilisegundo = NULL;
}
