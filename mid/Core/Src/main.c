/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
typedef enum {
    STATE_SAFE,     // 安全狀態：綠燈常亮
    STATE_WARNING,  // 警告狀態：黃燈閃爍 (持續 20 秒)
    STATE_ALARM     // 警報狀態：紅燈閃爍 + 蜂鳴器
} SystemState;

volatile SystemState currentState = STATE_SAFE;
volatile uint32_t stateStartTime = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  stateStartTime = HAL_GetTick();
  uint32_t blink_timer = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed_time = current_tick - stateStartTime;

    switch (currentState)
    {
      case STATE_SAFE:
        // 1. 輸出控制：綠燈常亮，其餘關閉
        HAL_GPIO_WritePin(led_green_GPIO_Port, led_green_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(led_yellow_GPIO_Port, led_yellow_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(led_red_GPIO_Port, led_red_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(buzz_GPIO_Port, buzz_Pin, GPIO_PIN_RESET);

        // 2. 超過 10 秒（10000 ms）未按下按鈕，轉入警告狀態
        if (elapsed_time >= 10000)
        {
          currentState = STATE_WARNING;
          stateStartTime = current_tick; // 重置狀態計時器
        }
        break;

      case STATE_WARNING:
        // 1. 輸出控制：綠燈/紅燈/蜂鳴器關閉
        HAL_GPIO_WritePin(led_green_GPIO_Port, led_green_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(led_red_GPIO_Port, led_red_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(buzz_GPIO_Port, buzz_Pin, GPIO_PIN_RESET);

        // 2. 黃燈每 250ms 閃爍一次（非阻塞）
        if (current_tick - blink_timer >= 250)
        {
          HAL_GPIO_TogglePin(led_yellow_GPIO_Port, led_yellow_Pin);
          blink_timer = current_tick;
        }

        // 3. 在黃燈狀態持續超過 20 秒（20000 ms）未按下按鈕，轉入警報狀態
        if (elapsed_time >= 20000)
        {
          currentState = STATE_ALARM;
          stateStartTime = current_tick;
        }
        break;

      case STATE_ALARM:
        // 1. 輸出控制：綠燈與黃燈關閉
        HAL_GPIO_WritePin(led_green_GPIO_Port, led_green_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(led_yellow_GPIO_Port, led_yellow_Pin, GPIO_PIN_RESET);

        // 2. 紅燈與蜂鳴器同步每 250ms 閃爍與發聲
        if (current_tick - blink_timer >= 100)
        {
          HAL_GPIO_TogglePin(led_red_GPIO_Port, led_red_Pin);
          HAL_GPIO_TogglePin(buzz_GPIO_Port, buzz_Pin);
          blink_timer = current_tick;
        }
        break;
    }
  }
}

// 系統時脈設定
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

// GPIO 初始化 (CubeMX 自動生成部分對應)
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 輸出腳位預設低電位
  HAL_GPIO_WritePin(GPIOC, led_green_Pin|led_yellow_Pin|led_red_Pin|buzz_Pin, GPIO_PIN_RESET);

  // PA9 外部中斷設定 (對應 button)
  GPIO_InitStruct.Pin = button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(button_GPIO_Port, &GPIO_InitStruct);

  // LED 與蜂鳴器輸出腳位設定
  GPIO_InitStruct.Pin = led_green_Pin|led_yellow_Pin|led_red_Pin|buzz_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // 使能 EXTI9_5 中斷向量
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* USER CODE BEGIN 4 */
// 外部中斷回呼函式 (當 PA9 偵測到下降緣時觸發)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == button_Pin)
    {
        static uint32_t last_interrupt_time = 0;
        uint32_t current_time = HAL_GetTick();

        // 200ms 防彈跳機制
        if (current_time - last_interrupt_time > 200)
        {
            currentState = STATE_SAFE;
            stateStartTime = current_time; // 重置計時，回到綠燈常亮
        }
        last_interrupt_time = current_time;
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
