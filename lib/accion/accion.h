#ifndef ACCION_H
#define ACCION_H

typedef struct Accion Accion;

struct Accion{
    void (*const ejecuta)(Accion *accion);
};

__attribute__((__always_inline__))
inline void Accion_ejecuta(Accion *accion)
{
    accion->ejecuta(accion);
}
#endif
