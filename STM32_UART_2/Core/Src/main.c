/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
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

/* USER CODE BEGIN PV */
uint8_t rx_char = 0;

typedef enum{
  MODE_OFF,
  MODE_ON,
  MODE_BLINK,
  MODE_BLINK_FADE,
} LedMode_t;

LedMode_t led_mode = MODE_OFF;

/* Toggle mode variables */
uint32_t last_toggle_tick = 0;
uint8_t toggle_state = 0;

/* Blink (fade) mode variables */
uint16_t pwm_duty = 0;
int8_t fade_step = 20;
uint32_t last_fade_tick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 0xFFFF);
  return len;
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
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /* Start PWM on Channel 1 (PA5 - onboard LED) */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  /* Send startup message to terminal */
  uint8_t msg[] = "STM32 Ready. Commands: o=On, x=Off, b=Blink, f=Fade\r\n";
  HAL_UART_Transmit(&huart2, msg, sizeof(msg) - 1, 100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* -------------------- RECEIVE (Polling, Non-Blocking) -------------------- */
	 /* timeout = 0 means: check once, return immediately if no data */
	 if (HAL_UART_Receive(&huart2, &rx_char, 1, 0) == HAL_OK)
	 {
     printf("Received: %c\r\n", rx_char);

		 switch (rx_char)
		 {
			 case 'o':
				 led_mode = MODE_ON;
				 break;

			 case 'x':
				 led_mode = MODE_OFF;
				 break;

			 case 'b':
				 led_mode = MODE_BLINK;
				 last_toggle_tick = HAL_GetTick();//A
				 toggle_state = 0;
				 break;

			 case 'f':
				 led_mode = MODE_BLINK_FADE;
				 pwm_duty = 0;
				 fade_step = fade_step;
				 last_fade_tick = HAL_GetTick();
				 break;

			 default:
				 printf("Unknown command: %c\r\n", rx_char);
				 break;
		 }
	 }

	 /* -------------------- ACTION (Switch-Case on Mode) -------------------- */
	 switch (led_mode)
	 {
		 case MODE_ON:
			 /* 100% duty cycle = LED fully ON */
			 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 999);
			 break;

		 case MODE_OFF:
			 /* 0% duty cycle = LED fully OFF */
			 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
			 break;

		 case MODE_BLINK:
			 /* Toggle between ON and OFF every 500 ms */
			 if ((HAL_GetTick() - last_toggle_tick) >= 500)
			 {
				 toggle_state = !toggle_state;
				 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, toggle_state ? 999 : 0);
				 last_toggle_tick = HAL_GetTick();
			 }
			 break;

		 case MODE_BLINK_FADE:
			 /* Fade LED in/out (breathing effect) using PWM */
			 if ((HAL_GetTick() - last_fade_tick) >= 20)  /* update every 20 ms */
			 {
				 pwm_duty += fade_step;

				 if (pwm_duty >= 999)
				 {
					 //pwm_duty = 999;
					 fade_step = -fade_step;   /* reverse direction: fade down */
				 }
				 else if (pwm_duty <= 0)
				 {
					 //pwm_duty = 1;
					 fade_step = fade_step;    /* reverse direction: fade up */
				 }

				 if(pwm_duty>999)
					 pwm_duty = 999;
				 else if(pwm_duty<0)
					 pwm_duty = 0;

				 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_duty);
				 last_fade_tick = HAL_GetTick();
			 }
			 break;
	 };
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
#ifdef USE_FULL_ASSERT
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
