#include "es_lcd.h"
#include "gpio.h"
#include "tempo_ms.h"

static Bus datos = INICIALIZA_VARIABLE_BUS(FLOTANTE,PUSH_PULL,V_BAJA,
PB9,PB8,PB7,PB6);

static Bus control = INICIALIZA_VARIABLE_BUS(PULL_UP,PUSH_PULL,V_BAJA,
PB5,PB4,PB3);

enum{
    CTRL_E  = 0b100,
    CTRL_RW = 0b010,
    CTRL_RS = 0b001,
    BUSY = 0b1000
};

static void enviaComando(uint8_t cmd);
static void enviaDato(uint8_t dat);
static void configuraModo(void);


Lcd *inicializaLcd(void)
{
    static Lcd miLcd;
    Lcd_inicializa(&miLcd,&enviaComando,&enviaDato,&configuraModo);
    return &miLcd;
}

static void escribe4bit(uint8_t valor, bool RS)
{
    const unsigned modo = RS? CTRL_RS : 0;
    Bus_escribe(&datos, valor);
    Bus_escribe(&control,modo);
    Tempo_esperaMicrosegundos(1);
    Bus_escribe(&control,modo|CTRL_E);
    Tempo_esperaMicrosegundos(1);
    Bus_escribe(&control,modo);
    Tempo_esperaMicrosegundos(100);
}

static void escribeValor(uint8_t valor, bool RS)
{
    escribe4bit(valor>>4,RS);
    escribe4bit(valor&0xF,RS);
}

static void enviaComando(uint8_t cmd)
{
    escribeValor(cmd,false);
}
static void enviaDato(uint8_t dat)
{
    escribeValor(dat,true);
}
static void configuraModo(void)
{
    Tempo_esperaMilisegundos(50);
    escribe4bit(0b11,false);
    Tempo_esperaMicrosegundos(4500);
    escribe4bit(0b11,false);
    Tempo_esperaMicrosegundos(4500);
    escribe4bit(0b11,false);
    Tempo_esperaMicrosegundos(150);
    escribe4bit(0b10,false);
    Tempo_esperaMicrosegundos(150);
    enviaComando(0b101000); // 2 líneas, fuente 5x8
}
