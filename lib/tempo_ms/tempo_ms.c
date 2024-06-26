#include "tempo_ms.h"
#include <stm32f1xx.h>

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

void SysTick_Handler(void)
{
    ++ticks;
}