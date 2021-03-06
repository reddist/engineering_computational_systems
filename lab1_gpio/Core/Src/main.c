/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpioutil.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void setupPassword(uint16_t *password) {
    password[0] = 1;
    password[1] = 0;
    password[2] = 1;
    password[3] = 1;
    password[4] = 0;
    password[5] = 1;
    password[6] = 0;
    password[7] = 1;
}

void block(
        uint16_t *current_position,
        uint16_t *is_pushed_down,
        uint16_t *block_times
) {
    *current_position = 0;
    *is_pushed_down = 0;
    (*block_times)++;
    if (*block_times < 3) {
        blink(PIN_RED, 250, 3);
    } else {
        *block_times = 0;
        blink(PIN_RED, 350, 10);
    }
}

void checkBtn(
        uint16_t btn_input,
        uint16_t *current_position,
        uint16_t *password,
        uint16_t *block_times,
        uint16_t *is_pushed_down
) {
    if (btn_input == password[*current_position]) {
        (*current_position)++;
        if (*current_position == 8) {
            *current_position = 0;
            *block_times = 0;
            blink(PIN_GREEN, 350, 10);
        } else {
            blink(PIN_YELLOW, 250, 3);
        }
    } else {
        block(current_position, is_pushed_down, block_times);
    }
}

uint16_t checkTime(
        uint32_t push_down_timestamp,
        uint32_t pull_up_timestamp,
        uint32_t long_push_time_ms
) {
    return (pull_up_timestamp - push_down_timestamp >= long_push_time_ms) ? 1 : 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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

    /* USER CODE BEGIN 2 */
    uint16_t password[8]; // 1 - long, 0 - short
    setupPassword(password);
    uint16_t btn_input;

    uint16_t current_position = 0;
    uint16_t block_times = 0;
    uint16_t is_pressed = 0;

    uint32_t press_timestamp;
    uint32_t release_timestamp;
    uint32_t short_press_time_ms = 100;
    uint32_t long_press_time_ms = 1000;
    blink(PIN_GREEN, 250, 5);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    GPIO_PinState btn_state;
    while (1) {
        btn_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
        if (btn_state == GPIO_PIN_SET && current_position > 0 &&
            checkTime(release_timestamp, HAL_GetTick(), 5000) == 1) {
            block(&current_position, &is_pressed, &block_times);
        }
        if (is_pressed && btn_state == GPIO_PIN_SET) {
            release_timestamp = HAL_GetTick();
            btn_input = checkTime(press_timestamp, release_timestamp, long_press_time_ms);
            is_pressed = 0;
            checkBtn(btn_input, &current_position, password, &block_times, &is_pressed);
        }
        if (!is_pressed && btn_state == GPIO_PIN_RESET) {
            press_timestamp = HAL_GetTick();
            wait(short_press_time_ms); // debounce
            btn_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15); // debounce
            is_pressed = btn_state == GPIO_PIN_RESET;
        }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
