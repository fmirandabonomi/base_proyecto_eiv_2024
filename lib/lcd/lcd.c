#include "lcd.h"
#include "tempo_ms.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00


void Lcd_inicializa(
  Lcd *lcd,
  void (*enviaComando)(uint8_t),
  void (*enviaDato)(uint8_t),
  void (*configuraModo)(void))
{
  lcd->enviaComando=enviaComando;
  lcd->enviaDato = enviaDato;
  lcd->configuraModo = configuraModo;
  configuraModo();
  Lcd_enciendePantalla(lcd);
}

void Lcd_limpiaPantalla(Lcd *lcd)
{
  lcd->enviaComando(LCD_CLEARDISPLAY);
  Tempo_esperaMilisegundos(2);
}

void Lcd_mueveAInicio(Lcd *lcd)
{
  lcd->enviaComando(LCD_RETURNHOME);
  Tempo_esperaMilisegundos(2);
}

void Lcd_escribeCadena(Lcd *lcd,const char *cadena)
{
  lcd->enviaComando(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
  for(const char *p = cadena;*p;++p){
    lcd->enviaDato(*p);
  }
}

void Lcd_establecePosicion(Lcd *lcd,unsigned fila,unsigned col)
{
  enum{ESTABLECE_DIR_DDRAM = 0b10000000};
  lcd->enviaComando(ESTABLECE_DIR_DDRAM | (0x40*fila + col));
}

void Lcd_enciendePantalla(Lcd *lcd)
{
  lcd->enviaComando(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
}
void Lcd_apagaPantalla(Lcd *lcd)
{
  lcd->enviaComando(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
}
