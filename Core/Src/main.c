/* USER CODE BEGIN Header */
/*
   Лабораторна робота 8.
   Дослідження процесів передачі повідомлень і взаємодії між
   потоками у операційній системі на прикладі FreeRTOS

   Підключення: 1.1 GND - [-LED+] - [резистор 325R] - PB15
   	   	   	   	1.2 GND - [-LED+] - [резистор 325R] - PB14
   	   	   	   	1.3 GND - [-LED+] - [резистор 325R] - PB13
   	   	   	   	1.4 GND - [-LED+] - [резистор 325R] - PB12

                2.1 GND - [-Key+] - PA0 (Pull_up)
                2.2 GND - [-Key+] - PA1 (Pull_up)
                2.3 GND - [-Key+] - PA2 (Pull_up)
                2.4 GND - [-Key+] - PA3 (Pull_up)
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*Універсальна оболонка (wrapper) від ARM. Дозволяє коду бути сумісним з різними операційними системами,
 * використовуючи стандартні команди (наприклад, osDelay).*/
#include "cmsis_os.h"

/*"Серце" системи. Містить головні налаштування ядра, типи даних та конфігурацію всієї ОС
 * для мікроконтролера.*/
#include "FreeRTOS.h"

/*Менеджер потоків. Відповідає за створення задач (xTaskCreate), їхні пріоритети та паузи в роботі (vTaskDelay).*/
#include "queue.h"

/*Поштова скринька. Дозволяє кнопкам безпечно передавати нові значення затримок у потоки світлодіодів.*/
#include "task.h"

/*Регулювальник руху. Надає семафори, які змушують світлодіоди вмикатися суворо по черзі, передаючи "естафету".*/
#include "semphr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

// Семафори для передачі черги
SemaphoreHandle_t xSemNext[4]; // Семафори для синхронізації

// Оголошуємо черги для кожного світлодіода
QueueHandle_t xQueueLED[4];

// Затримки за замовчуванням для кожного кольору (мс)
uint32_t currentDelays[4] = {500, 500, 500, 500};

// Масив пінів для зручності
uint16_t ledPins[4] = {USER_LED1_Pin, USER_LED2_Pin, USER_LED3_Pin, USER_LED4_Pin};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Функція потоку (Task)
// Працює, передаючі номер індексу через pvParameters
void LedTask(void *pvParameters) {
    uint32_t index = (uint32_t)pvParameters;
    uint32_t next_index = (index + 1) % 4; // Хто йде наступним
    uint32_t localDelay = 100;             // Початкова тривалість фази

    for (;;) {
        // Чекаємо на свою чергу (семафор)
        xSemaphoreTake(xSemNext[index], portMAX_DELAY);

        // Перевіряємо, чи натискали кнопку (оновлюємо час фази)
        if (xQueueReceive(xQueueLED[index], &localDelay, 0) == pdPASS) {
        }

        // Вмикаємо світлодіод (Фаза активна)
        HAL_GPIO_WritePin(GPIOB, ledPins[index], GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(localDelay));
        HAL_GPIO_WritePin(GPIOB, ledPins[index], GPIO_PIN_RESET);

        // Передаємо естафету наступному світлодіоду
        xSemaphoreGive(xSemNext[next_index]);
    }
}

// Функція натискання кнопок (EXTI Callback)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t newDelay;

    // Визначаємо, яка кнопка натиснута, і надсилаємо нову затримку
    if (GPIO_Pin == USER_BUT1_Pin) {
        newDelay = 2500; // Для PD12
        xQueueSendFromISR(xQueueLED[0], &newDelay, &xHigherPriorityTaskWoken);
    } else if (GPIO_Pin == USER_BUT2_Pin) {
        newDelay = 2000; // Для PD13
        xQueueSendFromISR(xQueueLED[1], &newDelay, &xHigherPriorityTaskWoken);
    } else if (GPIO_Pin == USER_BUT3_Pin) {
        newDelay = 1500; // Для PD14
        xQueueSendFromISR(xQueueLED[2], &newDelay, &xHigherPriorityTaskWoken);
    } else if (GPIO_Pin == USER_BUT4_Pin) {
        newDelay = 1000; // Повільне миготіння для PD15
        xQueueSendFromISR(xQueueLED[3], &newDelay, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  // Створюємо 4 черги
  for(int i = 0; i < 4; i++) {
      xQueueLED[i] = xQueueCreate(1, sizeof(uint32_t));
      xSemNext[i] = xSemaphoreCreateBinary(); // Створюємо бінарні семафори
  }

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  //defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  // Створюємо 4 потоки, передаючи i як аргумент
  xTaskCreate(LedTask, "RED", 128, (void*)0, 1, NULL);
  xTaskCreate(LedTask, "YELLOW", 128, (void*)1, 1, NULL);
  xTaskCreate(LedTask, "GREEN", 128, (void*)2, 1, NULL);
  xTaskCreate(LedTask, "BLUE", 128, (void*)3, 1, NULL);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  //osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  xSemaphoreGive(xSemNext[0]); // Даємо старт першому світлодіоду

  vTaskStartScheduler(); // Запускаємо планувальник

  while (1)
  {
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

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, USER_LED1_Pin|USER_LED2_Pin|USER_LED3_Pin|USER_LED4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : USER_BUT1_Pin USER_BUT2_Pin USER_BUT3_Pin USER_BUT4_Pin */
  GPIO_InitStruct.Pin = USER_BUT1_Pin|USER_BUT2_Pin|USER_BUT3_Pin|USER_BUT4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USER_LED1_Pin USER_LED2_Pin USER_LED3_Pin USER_LED4_Pin */
  GPIO_InitStruct.Pin = USER_LED1_Pin|USER_LED2_Pin|USER_LED3_Pin|USER_LED4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

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
