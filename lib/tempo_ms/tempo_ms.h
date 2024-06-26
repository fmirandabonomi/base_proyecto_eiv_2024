#ifndef TEMPORIZACION_SW_H
#define TEMPORIZACION_SW_H

#include <stdint.h>

void Tempo_inicializa(void);

uint32_t Tempo_obtMilisegundos(void);

void Tempo_esperaMilisegundos(uint32_t tiempo);

#endif // TEMPORIZACION_SW_H
