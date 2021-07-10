/*
 * zdroj.c
 *
 * Created: 20.03.2021 10:15:50
 * Author : Milan
 */ 
volatile uint8_t ADCLB[8];
volatile uint8_t ADCHB[8];
volatile uint32_t voltage;
volatile uint32_t vystup = 0;
volatile char voltage_string[5];
volatile char milivolty_string[5];
volatile char current_string[5];
#include <string.h>
#include <avr/io.h>
#include "u8g.h"
#include "util/delay.h"
volatile u8g_t u8g;
int adcRead(uint8_t mux) {
    ADMUX = mux;                                          // mux
    ADMUX |= (1 << REFS0);                                // AVcc reference
    ADMUX |= (1 << ADLAR);                                // zarovnat vlevo
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 pøeddìlièka pro nižší šum
    ADCSRA |= (1 << ADEN);                                // zapnutí
    ADCSRA |= (1 << ADSC);                                // start
    while(ADCSRA & (1 << ADSC));                          // tu se to sekne
    ADCLB[mux] = ADCL;
    ADCHB[mux] = ADCH;
}
int adcMesure(uint8_t mux)
{
  vystup = 0;
  for(uint8_t i = 0; i<8; i++)
  {
    adcRead(mux);
    vystup += ((ADCHB[mux]*0x04) + (ADCLB[mux]/0x40));
  }
  vystup /= 8;
}
void u8g_setup(void)
{
  u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_i2c, U8G_I2C_OPT_NONE);
}
void sys_init(void)
{
  #if defined(__AVR__)
  /* select minimal prescaler (max system speed) */
  CLKPR = 0x80;
  CLKPR = 0x00;
  #endif
}
void draw(void)
{
  u8g_SetFont(&u8g, u8g_font_10x20);
  u8g_DrawStr(&u8g, 5, 30, voltage_string);
  u8g_DrawStr(&u8g, 23, 30, ",");
  u8g_DrawStr(&u8g, 30, 30, milivolty_string);
  u8g_DrawStr(&u8g, 70, 30, "V");
  u8g_DrawStr(&u8g, 5, 60, current_string);
  u8g_DrawStr(&u8g, 70, 60, "mA");
}
int main(void)
{
  uint32_t current = 0;
    sys_init();
    u8g_setup();
    sei();
    while (1) 
    {
    adcMesure(0);
    voltage = vystup;
    voltage = (voltage * 127077)/10000;
    itoa(voltage /1000 , voltage_string,10);
    voltage = voltage - ((voltage /1000)*1000);
    itoa(voltage , milivolty_string,10);
  if (voltage <=99)
  {
    for (uint8_t i = 0; i < 2; i++)
    {
      milivolty_string[(1-i)] = milivolty_string[(2-i)];
    }
    milivolty_string[0] = '0';
  }
  if (voltage <=9)
  {
    milivolty_string[0] = milivolty_string[2];
    milivolty_string[0] = '0';
    milivolty_string[1] = '0';
  }
  if (voltage <=9)
  {
    milivolty_string[0] = '0';
    milivolty_string[1] = '0';
    milivolty_string[2] = '0';
  }
  adcMesure(1);
  current = vystup;
  current = (current*26)/10;
    itoa(current , current_string,10);
    u8g_FirstPage(&u8g);
    do
    {
      draw();
    } while ( u8g_NextPage(&u8g) );
    _delay_ms(10);
    }
}