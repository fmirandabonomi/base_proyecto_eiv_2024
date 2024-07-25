#include "tempo_ms.h"
#include <stm32f1xx.h>
#include <stddef.h>

static volatile uint32_t ticks;
static volatile uint32_t ciclosPorMicrosegundo;

void Tempo_inicializa(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);
    ciclosPorMicrosegundo = SystemCoreClock / 1000000;
    if(!ciclosPorMicrosegundo) ciclosPorMicrosegundo = 1;
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

static AccionParam *accionMilisegundo = NULL;

void SysTick_Handler(void)
{
    uint32_t t = ++ticks;
    if(accionMilisegundo){
        AccionParam_ejecuta(accionMilisegundo,&t);
    }
}


int Tempo_ponAccionMilisegundo(AccionParam *accion)
{
    if (accionMilisegundo || !accion) return -1;
    accionMilisegundo = accion;
    return 0;
}
void Tempo_eliminaAccionMilisegundo(void)
{
    accionMilisegundo = NULL;
}

__attribute__((optimize(3)))
void Tempo_esperaMicrosegundos(uint32_t t)
{
    uint32_t ciclos = t*ciclosPorMicrosegundo;
    if (ciclos<19) return;
    for(volatile uint32_t i=0;i<(ciclos-19);i+=10);
}