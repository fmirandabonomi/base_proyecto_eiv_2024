#include "lcd.h"

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
}

void Lcd_limpiaPantalla(Lcd *lcd)
{
  lcd->enviaComando(1);
}

void Lcd_mueveAInicio(Lcd *lcd)
{
  lcd->enviaComando(0b10);
}

void Lcd_escribeCadena(Lcd *lcd,const char *cadena)
{
  lcd->enviaComando(0b110);
  for(const char *p = cadena;*p;++p){
    lcd->enviaDato(*p);
  }
}

void Lcd_establecePosicion(Lcd *lcd,unsigned fila,unsigned col)
{
  enum{ESTABLECE_DIR_DDRAM = 0b10000000};
  lcd->enviaComando(ESTABLECE_DIR_DDRAM | (0x40*fila + col));
}
