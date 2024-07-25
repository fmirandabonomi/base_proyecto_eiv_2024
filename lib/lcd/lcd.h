#ifndef LCD_H
#define LCD_H
#include <stdint.h>

typedef struct Lcd Lcd;

struct Lcd{
  void (*enviaComando)(uint8_t comando);
  void (*enviaDato)(uint8_t dato);
  void (*configuraModo)(void);

};


void Lcd_inicializa(
  Lcd *lcd,
  void (*enviaComando)(uint8_t),
  void (*enviaDato)(uint8_t),
  void (*configuraModo)(void));

void Lcd_limpiaPantalla(Lcd *lcd);

void Lcd_mueveAInicio(Lcd *lcd);

void Lcd_escribeCadena(Lcd *lcd,const char *cadena);

void Lcd_establecePosicion(Lcd *lcd,unsigned fila,unsigned col);

void Lcd_enciendePantalla(Lcd *lcd);
void Lcd_apagaPantalla(Lcd *lcd);

#endif // LCD_H
