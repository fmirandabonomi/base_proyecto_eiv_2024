#ifndef ACCION_H
#define ACCION_H

typedef struct Accion Accion;

struct Accion{
    void (*const ejecuta)(Accion *accion);
};

__attribute__((__always_inline__))
inline void Accion_ejecuta(Accion *accion)
{
    if(accion) accion->ejecuta(accion);
}

typedef struct AccionParam AccionParam;

struct AccionParam{
    void (*const ejecuta)(AccionParam *accion, void *param);
};

__attribute__((__always_inline__))
inline void AccionParam_ejecuta(AccionParam *accion,void *param)
{
    if(accion) accion->ejecuta(accion,param);
}

#endif
