#include "gpio.h"
#include "tempo_ms.h"
#include "serie.h"
#include "teclado.h"
#include "es_lcd.h"

static void seleccionaFilaTeclado(unsigned fila);
static unsigned leeColumnasTeclado(void);

int main(void)
{
    static const char *reportesTecla[]={
        [CodigoTecla_0]="Tecla 0 presionada! \n",
        [CodigoTecla_1]="Tecla 1 presionada! \n",
        [CodigoTecla_2]="Tecla 2 presionada! \n",
        [CodigoTecla_3]="Tecla 3 presionada! \n",
        [CodigoTecla_4]="Tecla 4 presionada! \n",
        [CodigoTecla_5]="Tecla 5 presionada! \n",
        [CodigoTecla_6]="Tecla 6 presionada! \n",
        [CodigoTecla_7]="Tecla 7 presionada! \n",
        [CodigoTecla_8]="Tecla 8 presionada! \n",
        [CodigoTecla_9]="Tecla 9 presionada! \n",
        [CodigoTecla_ASTERISCO]="Tecla * presionada! \n",
        [CodigoTecla_NUMERAL]="Tecla # presionada! \n",
        [CodigoTecla_NO_VALIDO]="Error al leer tecla! \n",
    };
    Teclado teclado;
    Tempo_inicializa();
    Lcd *const miLcd = inicializaLcd();
    Serie_init(9600);
    Pin_configuraSalida(PIN_LED,PUSH_PULL,V_BAJA);
    Pin_ponAlto(PIN_LED);
    Teclado_inicializa(&teclado,&seleccionaFilaTeclado,&leeColumnasTeclado);
    Lcd_limpiaPantalla(miLcd);
    Lcd_mueveAInicio(miLcd);
    for(;;)
    {
        Tempo_esperaMilisegundos(10);
        Teclado_procesa(&teclado);
        if(Teclado_hayEntradaPendiente(&teclado)){
            CodigoTecla cod = Teclado_obtEntrada(&teclado);
            Pin_ponBajo(PIN_LED);
            Lcd_mueveAInicio(miLcd);
            Lcd_escribeCadena(miLcd,reportesTecla[cod]);
            Serie_enviaCadena(reportesTecla[cod]);
        }else{
            Pin_ponAlto(PIN_LED);
        }
    }
    return 0;
}


static void seleccionaFilaTeclado(unsigned fila)
{
    static Bus filas = INICIALIZA_VARIABLE_BUS(PULL_UP,DRENADOR_ABIERTO,V_BAJA,
    PA5,PA6,PA7,PB0);
    Bus_escribe(&filas,~(0b1000UL >> fila));
}
static unsigned leeColumnasTeclado(void)
{
    static Bus columnas = INICIALIZA_VARIABLE_BUS(PULL_UP,DRENADOR_ABIERTO,V_BAJA,
    PB1,PB10,PB11);
    unsigned valor;
    Bus_lee(&columnas, &valor);
    return valor;
}