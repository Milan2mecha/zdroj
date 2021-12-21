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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char mecha[4] = {' ','0','0','0'};
char proud[4] = {' ','0','0','0'};
float y = 0;
float x = 0;
uint8_t pointer_p1 = 0x01;


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
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  SSD1306_Clear();
	  char* mecha = trimm(y);
	  SSD1306_GotoXY (26,3);
	  SSD1306_Putc (mecha[0], &Font_11x18, ((~pointer_p1 & 0x08)>>3));
	  SSD1306_GotoXY (37,3);
	  SSD1306_Putc (mecha[1], &Font_11x18, ((~pointer_p1 & 0x04)>>2));
	  SSD1306_GotoXY (48,3);
	  SSD1306_Putc(',', &Font_11x18, 1);
	  SSD1306_GotoXY (59,3);
	  SSD1306_Putc (mecha[2], &Font_11x18, ((~pointer_p1 & 0x02)>>1));
	  SSD1306_GotoXY (70,3);
	  SSD1306_Putc (mecha[3], &Font_11x18, (~pointer_p1 & 0x01));
	  SSD1306_GotoXY (107,3);
	  SSD1306_Putc('V', &Font_11x18, 1);
	  char* proud = trimm(x);
	  SSD1306_GotoXY (26,25);
	  SSD1306_Putc (proud[0], &Font_11x18, ((~pointer_p1 & 0x80)>>7));
	  SSD1306_GotoXY (37,25);
	  SSD1306_Putc (proud[1], &Font_11x18, ((~pointer_p1 & 0x40)>>6));
	  SSD1306_GotoXY (48,25);
	  SSD1306_Putc(',', &Font_11x18, 1);
	  SSD1306_GotoXY (59,25);
	  SSD1306_Putc (proud[2], &Font_11x18, ((~pointer_p1 & 0x20)>>5));
	  SSD1306_GotoXY (70,25);
	  SSD1306_Putc (proud[3], &Font_11x18, ((~pointer_p1 & 0x10)>>4));
	  SSD1306_GotoXY (107,25);
	  SSD1306_Putc('A', &Font_11x18, 1);
	  SSD1306_DrawLine(11, 45, 32, 45, 1);
	  SSD1306_DrawLine(10, 46, 10, 62, 1);
	  SSD1306_DrawLine(11, 62, 32, 62, 1);
	  SSD1306_DrawLine(33, 46, 33, 62, 1);
	  SSD1306_UpdateScreen(); // update screen
	  y += 0.05 ;
	  x += 0.025;
	  pointer_p1 *= 2;
	  if(pointer_p1 == 0)
	  {
		  pointer_p1 = 1;
	  }
	  HAL_Delay(100);


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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

