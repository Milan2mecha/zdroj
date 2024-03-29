/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "fonts.h"
#include <math.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

float Uzobrazene = 0;
float Izobrazene = 0;
float Mzobrazene = 0;
float teplota = 0;
uint8_t dataDAC [3] = {0x40, 0x0, 0x0};
uint32_t ADCout [4];					
float SMAU0 [50];							//SIMPLE MOVING AVERAGE U0
float SMAU1 [50];							//SIMPLE MOVING AVERAGE U1
float SMAU2 [50];							//SIMPLE MOVING AVERAGE U2
uint8_t SMAy = 0;							//SIMPLE MOVING AVERAGE index
uint16_t setmodeflag = 0; 					//mód zobrazení

//polling
uint8_t tlacitko [4];
uint8_t poslednistav [4];
uint8_t debounce [5];

//menu 1
uint8_t cursor = 0;
uint8_t menupage = 0;
float setvoltage = 0;
float setcurrent = 0;
uint8_t pointer_p1 = 0x01;

//USB
uint8_t message[22] = "$0000|0000|C|000|00|E";				//odesílaná zpráva
uint16_t len = sizeof(message)/sizeof(message[0]);			//délka odesílané zprávy
uint8_t buffer_usb[12];										//buffer přijaté zprávy
uint8_t receive;											//příznak přijetí nové zprávy

//servisní veličiny
float Uadc = 3.3;		//vstupní napětí ADC
float offset = 0.0;		//offset ADC
float napetiBUCK[16] = {0, 0, 0.79, 2.40, 3.52, 5.10, 5.90, 8.50, 8.80, 11.30, 12.20, 14.37, 14.97, 16.63, 17.40, 18.71};	//převodní tabulka výstupních napětí BUCK reguátoru
uint8_t dataBUCK[16] = {0, 1, 2, 4, 3, 5, 6, 7, 8, 9, 10, 12, 11, 13, 14, 15};

//chlazení
uint16_t ventilatorper = 0;  //výkon ventilátoru v %
uint8_t ventilatorhyst = 0;	//hystereze ventilatoru
//startovní sekvence
void start()
{
	SSD1306_Clear();
	SSD1306_GotoXY (25,25);
	SSD1306_Puts("zdrojOS", &Font_11x18, 1);
	SSD1306_UpdateScreen();
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1);
}



//USB
uint8_t loopcount = 0;		//loopcount pro odeslání každých 256 cyklů
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);	//odkaz na funkci odeslání z "usbd_cdc_if.c"

//funkce rozložení přijatého rámce do globálních proměnných
void read_ser()
{
	char u_char[5];	//string napětí
	for(int i = 0; i<4; i++){
		u_char[i] = buffer_usb[i+1];
	}
	u_char[4] = '\0';
	setvoltage = atoi(u_char);	//převod na číslo
	setvoltage = setvoltage/100;	//vložení do globální proměnné

	char i_char[5]; //string proudu
	for(int i = 0; i<4; i++){	
		i_char[i] = buffer_usb[i+6];
	}
	i_char[4] = '\0';
	setcurrent = atoi(i_char);	//převod na číslo
	setcurrent = setcurrent/100;	//vložení do globální proměnné

}


//vytvoření rámce
void create_mess(float v, float c, uint8_t cvcc, uint8_t err, float teplota, uint16_t vent)
{
	//převod float napětí
	v = v*100;
	char vs[5];
	itoa((uint16_t)v, vs, 10);
	if(v<10){
		message[1] = '0';
		message[2] = '0';
		message[3] = '0';
		message[4] = vs[0];
	}else if(v<100){
		for(int i = 0; i<2; i++){
			message[i+3] = vs[i];
		}
		message[2] = '0';
		message[1] = '0';
	}else if(v<1000){
		for(int i = 0; i<3; i++){
			message[i+2] = vs[i];
		}
		message[1] = '0';
	}else{
	for(int i = 1; i<5; i++){
		message[i] = vs[i-1];
	}}

	//převod float proudu
	c = c*100;
	char cs[5];
	itoa((uint16_t)c, cs, 10);
	if(c<10){
		message[6] = '0';
		message[7] = '0';
		message[8] = '0';
		message[9] = cs[0];
	}else if(c<100){
		for(int i = 0; i<2; i++){
			message[i+8] = cs[i];
		}
		message[7] = '0';
		message[6] = '0';
	}else if(c<1000){
		for(int i = 0; i<3; i++){
			message[i+7] = cs[i];
		}
		message[6] = '0';
	}else{
	for(int i = 0; i<4; i++){
		message[i+6] = cs[i];
	}}
	//převod CVCC režimu
	if(cvcc ==1){
		message[11] = 'V';
	}else{
		message[11] = 'C';
	}
	//převod float teploty
	teplota = 1000*teplota;
	char teplotas[4];
	itoa((uint16_t)teplota, teplotas, 10);
	if(teplota>100){
		message[13]=teplotas[0];
		message[14]=teplotas[1];
		message[15]=teplotas[2];
	}else if(teplota>10){
		message[13]='0';
		message[14]=teplotas[0];
		message[15]=teplotas[1];
	}else {
		message[13]='0';
		message[14]='0';
		message[15]=teplotas[0];
	}
	//pokud jede vent na 100% odešle hodnotu 99% (kvůli formátu)
	if(vent == 100)
	{
		message[17] = '9';
		message[18] = '9';
	}else{
		char vents[3];
		itoa(vent, vents, 10);
		if(vent<10){
			message[17] = '0';
			message[18] = vents[0];
		}else{
		message[17] = vents[0];
		message[18] = vents[1];}
	}
	//errror mess 99==no error
	if(err == 99){
		message[20] = 'E';
	}else{
		message[20] = err + '0';
	}
}

//funkce error handleru
/*po zachycení chyby dojde k odpojení výstupního napětí a odeslání rámce s kódem chyby 
 ta je zároveň vypsána na display zařízení a zařízení vyčkává na stisknutí potvzení na tlačítku na zařízení*/
void error(uint8_t event)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 0);
	SSD1306_Clear();
	SSD1306_GotoXY (3,3);
	SSD1306_Puts("error:", &Font_11x18, 1);
	SSD1306_GotoXY (3,25);
	create_mess(Uzobrazene, Izobrazene, Mzobrazene,event, teplota, ventilatorper);
	int pokusy = 0;
	while((CDC_Transmit_FS(message, len) != USBD_OK)&&(pokusy<100)){
		  pokusy++;
	}
	switch (event) {
		case 0:
			SSD1306_Puts("mereni tep.", &Font_11x18, 1);
			SSD1306_GotoXY (3,45);
			SSD1306_Puts("OK=>reset", &Font_11x18, 1);
			SSD1306_UpdateScreen();
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != 0)
			{
				HAL_Delay(1);
			}
			start();
			break;
		case 1:
			SSD1306_Puts("prehrati", &Font_11x18, 1);
			SSD1306_GotoXY (3,45);
			SSD1306_Puts("OK=>reset", &Font_11x18, 1);
			SSD1306_UpdateScreen();
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != 0)
			{
				HAL_Delay(1);
			}
			start();
			break;
		case 2:
			SSD1306_Puts("chyba U0", &Font_11x18, 1);
			SSD1306_GotoXY (3,45);
			SSD1306_Puts("OK=>reset", &Font_11x18, 1);
			SSD1306_UpdateScreen();
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != 0)
			{
				HAL_Delay(1);
			}
			start();
			break;
		case 3:
			SSD1306_Puts("chyba U1", &Font_11x18, 1);
			SSD1306_GotoXY (3,45);
			SSD1306_Puts("OK=>reset", &Font_11x18, 1);
			SSD1306_UpdateScreen();
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != 0)
			{
				HAL_Delay(1);
			}
			start();
			break;
		case 4:
			SSD1306_Puts("chyba U3", &Font_11x18, 1);
			SSD1306_GotoXY (3,45);
			SSD1306_Puts("OK=>reset", &Font_11x18, 1);
			SSD1306_UpdateScreen();
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) != 0)
			{
				HAL_Delay(1);
			}
			start();
			break;
		default:
			break;

	}
}
//vstupy

void readbuttons()	//pulling tlačítek
{
	if(debounce[0] == 0)
	{
	  tlacitko[0] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
	  if(tlacitko[0] != poslednistav[0])
	  {
		  poslednistav[0] = tlacitko[0];
		  if(tlacitko[0] == 0)
		  {
			setmodeflag = 500;
			debounce[0] = 10;
			if(cursor == 0)
			{
				  cursor = 0x04;
			}else{
			  if(cursor>0x08)
			  {
				  cursor = 0;
				  setmodeflag = 10;
			  }
			  if((cursor>0)&&(cursor<0x10))
			  {
				  cursor = (cursor<<4);
			  }
			}
		 }
	  }
	}
	if(debounce[1] == 0)
	{
	  tlacitko[1] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
	  if(tlacitko[1] != poslednistav[1])
	  {
		  poslednistav[1] = tlacitko[1];
		  if(tlacitko[1] == 0)
		  {
			  setmodeflag = 500;
			  debounce[1] = 10;
				if(cursor == 0)
				{
					  cursor = 0x04;
				}
				else
				{
					switch (cursor) {
						case 0x80:
							cursor = 0x10;
							break;
						case 0x08:
							cursor = 0x01;
							break;
						default:
							cursor *= 2;
							break;
					}
				}
		  }
	  }
	}
	if(debounce[2] == 0)
	{
	  tlacitko[2] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
	  if(tlacitko[2] != poslednistav[2])
	  {
		  poslednistav[2] = tlacitko[2];
		  if(tlacitko[2] == 0)
		  {
			setmodeflag = 500;
			debounce[2] = 10;
			if(cursor == 0)
			{
				  cursor = 0x04;
			}
			else
			{
				switch (cursor) {
					case 0x01:
						cursor = 0x08;
						break;
					case 0x10:
						cursor = 0x80;
						break;
					default:
						cursor = cursor>>1;
						break;
				}

			}
		  }
	  }
	}
	if(debounce[3] == 0)
	{
	  tlacitko[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
	  if(tlacitko[3] != poslednistav[3])
	  {
		  poslednistav[3] = tlacitko[3];
		  if(tlacitko[3] == 0)
		  {
			  if((setmodeflag > 0)||(menupage>0))
			  {
				  menupage++;
			  }
			  setmodeflag = 500;
			  debounce[3] = 10;
			  if(menupage>2)
			  {
				  menupage = 0;
				  setmodeflag = 10;
			  }
		  }
	  }
	}

}

//pomocné převodní funkce

char* trimm(float f)
{
	static char trimmed [4];

	f *= 100;
	uint16_t g = f;
	itoa(g, trimmed, 10);

	if(g<10)
	{
		trimmed[3] = trimmed[0];
		trimmed[2] = '0';
		trimmed[1] = '0';
		trimmed[0] = ' ';


	}
	if((g<100)&&(g>9))
	{
		trimmed[3] = trimmed[1];
		trimmed[2] = trimmed[0];
		trimmed[1] = '0';
		trimmed[0] = ' ';

	}
	if((g<1000)&&(g>99))
	{
		for(uint8_t i = 3; i>0; i--)
		{
			trimmed[i] = trimmed[i-1];
		}
		trimmed[0] = ' ';

	}

	return trimmed;
}
float ADCtoVoltage(uint16_t ADCvalue)
{
	float voltage = 0;
	if(ADCvalue >= 161)
	{
		voltage = (ADCvalue/1226.9938)+0.0693;
	}
	else
	{
		voltage = ADCvalue;
		voltage = voltage/365;
	}
	return voltage;
}
float Voltagetoteperatur(float napeti)
{
	  napeti = (Uadc/ napeti)-1;
	 napeti = 1/(((log(napeti))/3380)+(1/298.5));
	  napeti = napeti - 273.15; // K => C
	  return napeti;
}


//funkce vykreslování displaje

void drawlogoC (uint8_t x, uint8_t y){
	  SSD1306_DrawLine((5+x), (y+5), (5+x), (y+11), 1);
	  SSD1306_DrawLine((6+x), (y+3), (6+x), (y+13), 1);
	  SSD1306_DrawLine((7+x), (y+3), (7+x), (y+5), 1);
	  SSD1306_DrawLine((7+x), (y+11), (7+x), (y+13), 1);
	  SSD1306_DrawLine((8+x), (y+14), (10+x), (y+14), 1);
	  SSD1306_DrawLine((8+x), (y+13), (10+x), (y+13), 1);
	  SSD1306_DrawLine((8+x), (y+2), (10+x), (y+2), 1);
	  SSD1306_DrawLine((8+x), (y+3), (10+x), (y+3), 1);
	  SSD1306_DrawLine((11+x), (y+3), (11+x), (y+4), 1);
	  SSD1306_DrawLine((11+x), (y+13), (11+x), (y+12), 1);
	  SSD1306_DrawLine((12+x), (y+4), (12+x), (y+6), 1);
	  SSD1306_DrawLine((12+x), (y+12), (12+x), (y+10), 1);
}
void drawmenu1(uint8_t cursorm1, uint8_t cvcc, float x, float y)
{
	  SSD1306_Clear();
	  cursorm1 = ~cursorm1;
	  char* mecha = trimm(y);
	  SSD1306_GotoXY (26,3);
	  SSD1306_Putc (mecha[0], &Font_11x18, ((cursorm1 & 0x08)>>3));
	  SSD1306_GotoXY (37,3);
	  SSD1306_Putc (mecha[1], &Font_11x18, ((cursorm1 & 0x04)>>2));
	  SSD1306_GotoXY (48,3);
	  SSD1306_Putc(',', &Font_11x18, 1);
	  SSD1306_GotoXY (59,3);
	  SSD1306_Putc (mecha[2], &Font_11x18, ((cursorm1 & 0x02)>>1));
	  SSD1306_GotoXY (70,3);
	  SSD1306_Putc (mecha[3], &Font_11x18, (cursorm1 & 0x01));
	  SSD1306_GotoXY (107,3);
	  SSD1306_Putc('V', &Font_11x18, 1);
	  char* proud = trimm(x);
	  SSD1306_GotoXY (26,25);
	  SSD1306_Putc (proud[0], &Font_11x18, ((cursorm1 & 0x80)>>7));
	  SSD1306_GotoXY (37,25);
	  SSD1306_Putc (proud[1], &Font_11x18, ((cursorm1 & 0x40)>>6));
	  SSD1306_GotoXY (48,25);
	  SSD1306_Putc(',', &Font_11x18, 1);
	  SSD1306_GotoXY (59,25);
	  SSD1306_Putc (proud[2], &Font_11x18, ((cursorm1 & 0x20)>>5));
	  SSD1306_GotoXY (70,25);
	  SSD1306_Putc (proud[3], &Font_11x18, ((cursorm1 & 0x10)>>4));
	  SSD1306_GotoXY (107,25);
	  SSD1306_Putc('A', &Font_11x18, 1);
	  uint8_t xcvcc = 0;
	  uint8_t ycvcc = 0;
	  switch (cvcc) {
		case 1:
			  xcvcc = 81;
			  ycvcc = 45;
			  drawlogoC(92, ycvcc);
			  SSD1306_DrawLine((5+xcvcc), (ycvcc+2), (5+xcvcc), (ycvcc+6), 1);
			  SSD1306_DrawLine((13+xcvcc), (ycvcc+2), (13+xcvcc), (ycvcc+6), 1);
			  SSD1306_DrawLine((6+xcvcc), (ycvcc+2), (6+xcvcc), (ycvcc+11), 1);
			  SSD1306_DrawLine((12+xcvcc), (ycvcc+2), (12+xcvcc), (ycvcc+11), 1);
			  SSD1306_DrawLine((7+xcvcc), (ycvcc+6), (7+xcvcc), (ycvcc+13), 1);
			  SSD1306_DrawLine((11+xcvcc), (ycvcc+6), (11+xcvcc), (ycvcc+13), 1);
			  SSD1306_DrawLine((8+xcvcc), (ycvcc+11), (8+xcvcc), (ycvcc+14), 1);
			  SSD1306_DrawLine((10+xcvcc), (ycvcc+11), (10+xcvcc), (ycvcc+14), 1);
			  SSD1306_DrawPixel((9+xcvcc), (ycvcc+13), 1);
			  SSD1306_DrawPixel((9+xcvcc), (ycvcc+14), 1);
			break;
		case 2:
			  xcvcc = 20;
			  ycvcc = 45;
			  drawlogoC(xcvcc, ycvcc);
			  drawlogoC((xcvcc+10), ycvcc);
			break;
		default:
			break;
	}
	  if (cvcc) {
		  	  SSD1306_DrawLine((2+xcvcc), ycvcc , (25+xcvcc), ycvcc, 1);
		  	  SSD1306_DrawLine(xcvcc, (ycvcc+2), xcvcc, (ycvcc+15), 1);
		  	  SSD1306_DrawLine((2+xcvcc), (ycvcc+17), (25+xcvcc), (ycvcc+17), 1);
		  	  SSD1306_DrawLine((27+xcvcc), (ycvcc+2), (27+xcvcc), (ycvcc+15), 1);
		  	  SSD1306_DrawPixel((1+xcvcc), (ycvcc+1), 1);
		  	  SSD1306_DrawPixel((1+xcvcc), (ycvcc+16), 1);
		  	  SSD1306_DrawPixel((26+xcvcc), (ycvcc+1), 1);
		  	  SSD1306_DrawPixel((26+xcvcc), (ycvcc+16), 1);
	  }
	  SSD1306_UpdateScreen(); // update screen
}
void drawmenu2()
{
	uint16_t pz = 0;
	uint16_t temp = teplota;
	char pzc [3];
	char tempc [3];
	char ventc [3];
	itoa(temp, tempc, 10);
	itoa(ventilatorper, ventc, 10);
	pz = Uzobrazene* Izobrazene;
	itoa(pz, pzc, 10);
	SSD1306_Clear();
	SSD1306_GotoXY (10,3);
	SSD1306_Puts("Pz =", &Font_11x18, 1);
	SSD1306_GotoXY (65,3);
	SSD1306_Puts(pzc, &Font_11x18, 1);
	SSD1306_GotoXY (107,3);
	SSD1306_Putc('W', &Font_11x18, 1);
	SSD1306_GotoXY (10,25);
	SSD1306_Puts("tep=", &Font_11x18, 1);
	SSD1306_GotoXY (65,25);
	SSD1306_Puts(tempc, &Font_11x18, 1);
	SSD1306_GotoXY (107,25);
	SSD1306_Putc('C', &Font_11x18, 1);
	SSD1306_GotoXY (10,43);
	SSD1306_Puts("vent=", &Font_11x18, 1);
	SSD1306_GotoXY (65, 43);
	SSD1306_Puts(ventc, &Font_11x18, 1);
	SSD1306_GotoXY (107,43);
	SSD1306_Putc('%', &Font_11x18, 1);
	SSD1306_UpdateScreen(); // update screen

}
void drawmenu3()
{
	char U0c [3];
	char U1c [3];
	char U2c [3];
	itoa(ADCout[0], U0c, 10);
	itoa(ADCout[1], U1c, 10);
	itoa(ADCout[2], U2c, 10);
	SSD1306_Clear();
	SSD1306_GotoXY (10,3);
	SSD1306_Puts("U0 =", &Font_11x18, 1);
	SSD1306_GotoXY (65,3);
	SSD1306_Puts(U0c, &Font_11x18, 1);
	SSD1306_GotoXY (10,25);
	SSD1306_Puts("U1=", &Font_11x18, 1);
	SSD1306_GotoXY (65,25);
	SSD1306_Puts(U1c, &Font_11x18, 1);
	SSD1306_GotoXY (10,43);
	SSD1306_Puts("U2=", &Font_11x18, 1);
	SSD1306_GotoXY (65, 43);
	SSD1306_Puts(U2c, &Font_11x18, 1);
	SSD1306_UpdateScreen(); // update screen

}

// výstupní elektrické veličiny

void ventilator(float temp)  //nastavení úrovně PWM ventilátoru
{
	// kontrola správnosti
	if((teplota > 100)||(teplota < 1))
	{
		error(0);
	}
	if(teplota > 70)
	{
		error(1);
	}

	//hystereze
	if(temp > 30)
	{
		ventilatorhyst = 1;
	}
	if(temp < 28)
	{
		ventilatorhyst = 0;
	}
	//výpočet v %
	if(ventilatorhyst == 1)
	{
		ventilatorper = ((temp - 25) * 2) + 50;
		if(ventilatorper > 100)
		{
			ventilatorper = 100;
		}
	}else
	{
		ventilatorper = 0;
	}
	//nastavení časovače v 8-bit
	TIM2->CCR1 = (ventilatorper*255)/100;
}
void setDAC1 (uint16_t data) // zapíše vpravo zarovnaná 12-bit data do DAC1 na I2C2
{
	dataDAC [1] = (data >> 4);
	dataDAC [2] = (data << 4) & 0xf0;
	HAL_I2C_Master_Transmit(&hi2c1, (0b1100001<<1), dataDAC, 3, 10);
}
void setDAC2 (uint16_t data) // zapíše vpravo zarovnaná 12-bit data do DAC1 na I2C2
{
	dataDAC [1] = (data >> 4);
	dataDAC [2] = (data << 4) & 0xf0;
	HAL_I2C_Master_Transmit(&hi2c2, (0b1100001<<1), dataDAC, 3, 10);
}
void setVout (float napeti)	 //řízení spínaného napěťového regulátoru  + příprava pro ADC
{
	uint16_t output;
	uint8_t i = 1;
	if(napeti>18.71)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 0);
	}else{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1);
		while(napetiBUCK[i]<napeti)
		{
			i++;
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (1&&(dataBUCK[i] & 0x08)));
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (1&&(dataBUCK[i] & 0x04)));
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (dataBUCK[i] & 0x02));
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (dataBUCK[i] & 0x01));
	}
	output = (napeti*512)/Uadc;
	setDAC2(output);
}
void setIout(float proud)
{
	uint16_t output;
	output = (proud*4095)/Uadc;
	setDAC1(output);
}



//vektory přerušení

 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  //přerušení krok encoderu
{
	uint8_t direct = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
	if(GPIO_Pin == GPIO_PIN_0)
	{
		if((setmodeflag > 0)&&(menupage == 0)){
		if(debounce[4] == 0)
		{
		int8_t i = 1;
		setmodeflag = 500;
		if(direct == 1)
		{
			i = -1;
		}
		switch (cursor) {
			case 0x10:
				setcurrent = setcurrent + (0.01 * i);
				break;
			case 0x20:
				setcurrent = setcurrent + (0.1 * i);
				break;
			case 0x40:
				setcurrent = setcurrent + i;
				break;
			case 0x80:
				setcurrent = setcurrent + (10*i);
				break;
			case 0x01:
				setvoltage = setvoltage + (0.01 * i);
				break;
			case 0x02:
				setvoltage = setvoltage + (0.1 * i);
				break;
			case 0x04:
				setvoltage = setvoltage + i;
				break;
			case 0x08:
				setvoltage = setvoltage + (10*i);
				break;
			default:
				break;
		}
		if(setvoltage < 0)
		{
			setvoltage = 0;
		}
		if(setcurrent < 0)
		{
			setcurrent = 0;
		}
		if(setcurrent > 2.8)
		{
			setcurrent = 2.8;
		}
		if(setvoltage > 19)
		{
			setvoltage = 19;
		}
		debounce[4] = 10;
		}
	}
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t refreshflag = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
  HAL_ADC_Start_DMA(&hadc1, ADCout, 4);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  start();
  //úvodní naplnění SMA
  for(uint8_t y; y<50;y++)
  {
	  SMAU0 [y] = ADCtoVoltage(ADCout[0])*8;
	  SMAU1 [y] = ADCtoVoltage(ADCout[1])*8;
	  SMAU2 [y] = ADCtoVoltage(ADCout[2]);
	  HAL_Delay(2);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	//Iterace SMA
	  SMAy++;
	  if(SMAy > 49)
	  {
		  SMAy = 0;
	  }
	  float U0 = 0;
	  float U1 = 0;
	  float U2 = 0;
	  SMAU0[SMAy] = ADCtoVoltage(ADCout[0])*8;
	  SMAU1[SMAy] = ADCtoVoltage(ADCout[1])*8;
	  SMAU2[SMAy] = ADCtoVoltage(ADCout[2]);
	  for(uint8_t y = 0; y<50; y++)
	  {
		  U0 = U0 + SMAU0[y];
		  U1 = U1 + SMAU1[y];
		  U2 = U2 + SMAU2[y];

	  }
	  U0 = U0 / 50;
	  U1 = U1 / 50;
	  U2 = U2 / 50;


	  readbuttons();
	  if(setmodeflag>0) //display vstoupí do interaktivního módu
	  {
		  if(setmodeflag == 500)
		  {
			  switch (menupage) {
				case 1:
					drawmenu2();
					setmodeflag = 750;
					break;
				case 2:
					drawmenu3();
					setmodeflag = 750;
					break;
				default:
					drawmenu1(cursor, 0, setcurrent , setvoltage);
					break;
			}
		  }

		  if(setmodeflag > 0)
		  {
			  setmodeflag--;
		  }
		  for(uint8_t i = 0; i<5; i++)
		  {
			  if(debounce[i] > 0)
			  {
				  debounce[i]--;
			  }
		  }
		  if(setmodeflag == 0)
		  {
			 menupage = 0;
			 setVout(setvoltage);
			 setIout(setcurrent);
			 refreshflag = 1;
		  }
		  HAL_Delay(2);
	  }
	  else
	  {
		  // není v setmode
		  uint8_t rezim;
		  if(U1>2)
		  {
			//režim proudového zdroje
			  rezim = 2;

		  }
		  else
		  {
			//režim napěťového zdroje
			  rezim = 1;
			  //pro potřeby debagu USB zakomentováno dolaďování napětí
			  /*if(((setvoltage - (U0-U1)) > 0.05) | ((setvoltage - (U0-U1)) < -0.05))
				{
				  setVout(setvoltage+U1);
				}
			 if(SMAy == 49)
			  {
				  if((setvoltage - (U0-U1)) > 0.1)
				  {
					  setVout((setvoltage+U1)+0.1);
				  }
				  if((setvoltage - (U0-U1)) < -0.1)
				  {
					  setVout((setvoltage+U1)-0.1);
				  }
			  }*/
		  }
		  if(U0>24)
		  {
			//přepětí (chyba měření)
			  error(2);
		  }
		  if(U1>24)
		  {
			//přepětí (chyba měření)
			  error(3);
		  }
		  if(U2>3)
		  {
			//naproud (zkrat nezachycený proudovým regulátorem)
			  error(4);
		  }
		  
		  //podmínky pro překreslení displeje
		  if(((Uzobrazene - (U0-U1))>0.05) | ((Uzobrazene - (U0-U1)) < -0.05))
		  {refreshflag = 1;}
		  if(((Izobrazene - U2)>0.02) | ((Izobrazene - U2) < -0.02))
		  {refreshflag = 1;}
		  if(rezim != Mzobrazene)
		  {refreshflag = 1;}

		  // pokud je příznak změny údajů na display obnoví display
		  if(refreshflag > 0)  
		  {
			  Mzobrazene = rezim;
			  Uzobrazene = U0-U1;
			  Izobrazene = U2;
			  drawmenu1(0, Mzobrazene, Izobrazene , Uzobrazene);
			  //sestavení zprávy pro COM a odeslání (max 100 pokusů)
			  create_mess(Uzobrazene, Izobrazene, Mzobrazene,99, teplota, ventilatorper);
			  int pokusy = 0;
			  while((CDC_Transmit_FS(message, len) != USBD_OK)&&(pokusy<100)){
				pokusy++;
			  }
			  refreshflag = 0;
		  }
		  HAL_Delay(1);
	  }
	  teplota = Voltagetoteperatur(ADCtoVoltage(ADCout[3]));
	  ventilator(teplota);
	  if(receive == 1){
		receive = 0;
		read_ser();
		 setVout(setvoltage);
		 setIout(setcurrent);
	  }
	  //sestavení zprávy pro COM a odeslání každých 255 cyklů. Slouží pro  stavy kdy dlouho nedochází ke změně proudu či napětí ale je třeba udržovat údaje o teplotě a chlazení aktuální(max 100 pokusů)
	  loopcount++;
	  if(loopcount==255){
		  create_mess(Uzobrazene, Izobrazene, Mzobrazene,99, teplota, ventilatorper);
		  int pokusy = 0;
		  while((CDC_Transmit_FS(message, len) != USBD_OK)&&(pokusy<100)){
			pokusy++;
		  }
	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 255;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

