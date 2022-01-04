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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "fonts.h"
#include <math.h>
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

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

float Im = 0;
float Um = 0;
float teplota = 0;
float p = 0;
uint8_t pointer_p1 = 0x01;
uint8_t dataDAC [3] = {0x40, 0xFF, 0xFF};
uint32_t ADCout [4];
uint8_t setmodeflag = 0;

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
	  if (cvcc) {
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
	  } else {
		  xcvcc = 20;
		  ycvcc = 45;
		  drawlogoC(xcvcc, ycvcc);
		  drawlogoC((xcvcc+10), ycvcc);
	  }
	  SSD1306_DrawLine((2+xcvcc), ycvcc , (25+xcvcc), ycvcc, 1);
	  SSD1306_DrawLine(xcvcc, (ycvcc+2), xcvcc, (ycvcc+15), 1);
	  SSD1306_DrawLine((2+xcvcc), (ycvcc+17), (25+xcvcc), (ycvcc+17), 1);
	  SSD1306_DrawLine((27+xcvcc), (ycvcc+2), (27+xcvcc), (ycvcc+15), 1);
	  SSD1306_DrawPixel((1+xcvcc), (ycvcc+1), 1);
	  SSD1306_DrawPixel((1+xcvcc), (ycvcc+16), 1);
	  SSD1306_DrawPixel((26+xcvcc), (ycvcc+1), 1);
	  SSD1306_DrawPixel((26+xcvcc), (ycvcc+16), 1);

	  SSD1306_UpdateScreen(); // update screen
}
void setDAC1 (uint16_t data) // zapíše vpravo zarovnaná 12-bit data do DAC1 na I2C2
{
	dataDAC [1] = (data >> 4);
	dataDAC [2] = (data << 4) & 0xf0;
	HAL_I2C_Master_Transmit(&hi2c2, (0b1100001<<1), dataDAC, 3, 10);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
  HAL_ADC_Start_DMA(&hadc1, ADCout, 4);
  drawmenu1(pointer_p1, 1,0, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  float rozdilchI = 0;
	  float rozdilchU = 0;
	  uint8_t refreshflag = 0;
	  p += 1;
	  if(setmodeflag>0)
	  {

		  drawmenu1(2, 1, 22.22 , 22.22);
		  setmodeflag--;
		  HAL_Delay(10);
	  }
	  else
	  {
		  // není v setmode
		  rozdilchI = Im - ((ADCout[0]*6.6)/4095);  // Aktualní - nová hodnota
		  if((rozdilchI > 0.01)||(rozdilchI < -0.01))
		  {
			  Im = (ADCout[0]*6.6)/ 4095;
			  refreshflag |= 0x01;
		  }
		  rozdilchU = Um - (((ADCout[1]*6.6)/4095)-((ADCout[2]*6.6)/4095)); // Aktualní - nová hodnota
		  if((rozdilchU > 0.01)||(rozdilchU < -0.01))
		  {
			  Um = (((ADCout[1]*6.6)/4095)-((ADCout[2]*6.6)/4095));
			  refreshflag |= 0x01;
		  }
		  if(refreshflag > 0)  // pokud je příznak změny údajů na display obnoví display
		  {
			  drawmenu1(0, 1, Im , ((p*5)/4095));
		  }
	  }
	  teplota = (ADCout[3]*3.3)/ 4095; // adc => V
	  teplota = 10000 * (teplota / ( 3.3 - teplota)); // V => R NTC
	  teplota = 1/((log(teplota/100000)/4000)+(1/298.15)); // RNTC => K
	  teplota = teplota - 273.15; // K => C
	  if(p>0xFFF){p=0;}
	  setDAC1(p);


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
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
  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

