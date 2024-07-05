#include "teclado.h"

void Teclado_inicializa(
    Teclado *teclado,
    void (*seleccionaFila)(unsigned),
    unsigned (*leeColumnas)(void))
{
    teclado->fila=0;
    teclado->seleccionaFila=seleccionaFila;
    teclado->leeColumnas=leeColumnas;
    teclado->estadoColumnas[0]=0b111;
    teclado->estadoColumnas[1]=0b111;
    teclado->estadoColumnas[2]=0b111;
    teclado->estadoColumnas[3]=0b111;
    teclado->hayTeclaPendiente=false;
    teclado->teclaPendiente=CodigoTecla_NO_VALIDO;
}

bool Teclado_hayEntradaPendiente(Teclado *teclado)
{
    return teclado->hayTeclaPendiente;
}

CodigoTecla Teclado_obtEntrada(Teclado *teclado)
{
    CodigoTecla cod = CodigoTecla_NO_VALIDO;
    if(teclado->hayTeclaPendiente){
        teclado->hayTeclaPendiente = false;
        cod = teclado->teclaPendiente;
    }
    return cod;
}

static CodigoTecla decodifica(unsigned fila,unsigned colsPresionadas)
{
    switch(fila){
    case 0: return (colsPresionadas & 0b100) ? CodigoTecla_1 :
                   (colsPresionadas & 0b010) ? CodigoTecla_2 :
                   (colsPresionadas & 0b001) ? CodigoTecla_3 :
                                               CodigoTecla_NO_VALIDO;
    case 1: return (colsPresionadas & 0b100) ? CodigoTecla_4 :
                   (colsPresionadas & 0b010) ? CodigoTecla_5 :
                   (colsPresionadas & 0b001) ? CodigoTecla_6 :
                                               CodigoTecla_NO_VALIDO;
    case 2: return (colsPresionadas & 0b100) ? CodigoTecla_7 :
                   (colsPresionadas & 0b010) ? CodigoTecla_8 :
                   (colsPresionadas & 0b001) ? CodigoTecla_9 :
                                               CodigoTecla_NO_VALIDO;
    case 3: return (colsPresionadas & 0b100) ? CodigoTecla_ASTERISCO :
                   (colsPresionadas & 0b010) ? CodigoTecla_0 :
                   (colsPresionadas & 0b001) ? CodigoTecla_NUMERAL :
                                               CodigoTecla_NO_VALIDO;
    default: return CodigoTecla_NO_VALIDO;
    }
}

void Teclado_procesa(Teclado *teclado)
{
    unsigned const cols = teclado->leeColumnas();
    unsigned const filaActual = teclado->fila;
    teclado->fila = (filaActual == 3)? 0 : filaActual + 1;
    teclado->seleccionaFila(teclado->fila);
    unsigned const colsAnterior = teclado->estadoColumnas[filaActual];
    teclado->estadoColumnas[filaActual] = cols;
    
    unsigned colsPresionadas = ~cols & colsAnterior;
    
    if (colsPresionadas){
        teclado->hayTeclaPendiente = colsPresionadas;
        teclado->teclaPendiente = decodifica(filaActual,colsPresionadas);
    }
}
