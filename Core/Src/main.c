#include "main.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"
#include "stdint.h"
#include "P5.h"
#include "mk_dht11.h"

#define V_REF 3.3

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
#define RX_BUFFER_SIZE 128

char rx_buffer[RX_BUFFER_SIZE]; 
char days[11] = "";   
char times[9] = "";
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
dht11_t dht; 
uint32_t temp, hum, rain, LDR, MIC, MQ2, mod;
uint32_t mod = 1;
float wind;
/* USER CODE END PTD */
char msg[30];
char day[9];
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t ADC_Read(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;  
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;  
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);  

    HAL_ADC_Start(&hadc1); 
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)  
    {
        return HAL_ADC_GetValue(&hadc1);  
    }
    return 0;  
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4){
		ngatquetled();
	}
}

uint8_t requestDataFromESP32() {
    char request[] = "GET_DATA";  // Yêu c?u nh?n d? li?u t? ESP32
    HAL_UART_Transmit(&huart1, (uint8_t *)"GET_DATA\n", strlen("GET_DATA\n"), HAL_MAX_DELAY);  // G?i yêu c?u
    uint8_t response[50];  // Buffer nh?n d? li?u
		HAL_UART_Receive(&huart1, response, sizeof(response) - 1, 1000);  // Nh?n d? li?u t? UART
		if(response[0] == 't' && response[1] == ' ')
		{
			sscanf((const char *)response, "t %10s %8s %02d %f %d", days, times, &rain, &wind, &mod);
				days[2] = '/';
				days[5] = '/';
				days[6] = '\0';
				times[5] = '\0';
				return 1;
		}
return 0;
}

float ADC_ToVoltage(uint16_t adc_value) {
    return ((float)adc_value / 4095.0) * V_REF;
}

float calculate_rms(int *adc_samples) {
    float sum_squared = 0.0;
    float adc_midpoint = (1 << (12 - 1)); 

    // Tính t?ng bình phuong giá tr? AC
    for (int i = 0; i < 100; i++) {
        float ac_value = adc_samples[i] - adc_midpoint; 
        sum_squared += ac_value * ac_value;
    }
    float rms = sqrt(sum_squared / 100);
    return rms * (3.3 / (1 << 12));
		
}

int calculate_db(float rms_voltage) {
    if (rms_voltage <= 0) {
        return -INFINITY; 
    }
    float db = 20.0f * log10f(rms_voltage / 3.3) + 46;
    return db;
}

void weather()
{
	HAL_TIM_Base_Stop_IT(&htim4);
	Matrix_clearArea(0,0,64,32);
	Matrix_showIcon(12, 0, 0, RED);
	Matrix_chonvitri(6, 8);
	sprintf(msg, "%02dC", temp);
	Matrix_guichuoi(msg, RED);
	memset(msg, 0, sizeof(msg));
	Matrix_showIcon(44, 0, 1, BLUE);
	Matrix_chonvitri(38, 8);
	sprintf(msg, "%02d%%", hum);
	Matrix_guichuoi(msg, BLUE);
	memset(msg, 0, sizeof(msg));
	Matrix_chonvitri(5, 16);
	sprintf(msg, "rain: %02d", rain);
	Matrix_guichuoi(msg, GREEN);
	memset(msg, 0, sizeof(msg));
	Matrix_chonvitri(5, 24);
	sprintf(msg, "wind: %0.1f", wind);
	Matrix_guichuoi(msg, CYAN);
	memset(msg, 0, sizeof(msg));
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_Delay(5000);	
}

void clock()
{
	sprintf(day, "CLOCK");
	HAL_TIM_Base_Stop_IT(&htim4);
	Matrix_clearArea(0,0,64,32);
	Matrix_chonvitri((64-strlen(day)*7+7)/2, 2);
	sprintf(msg, "%s", day);
	Matrix_guichuoi_MAKE_COLOR((unsigned char *)msg,(unsigned char []){RED,GREEN,BLUE,MAGENTA,WHITE,CYAN, BLUE, RED});
	memset(msg, 0, sizeof(msg));
	Matrix_chonvitri(10, 12);
	sprintf(msg, "%s24", days);
	Matrix_guichuoi(msg, BLUE);
	memset(msg, 0, sizeof(msg));
	Matrix_chonvitri(17, 22);
	sprintf(msg, "%s", times);
	Matrix_guichuoi(msg, GREEN); 
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_Delay(5000);
}

void env()
{
	HAL_TIM_Base_Stop_IT(&htim4);
	Matrix_clearArea(0,0,64,32);
	Matrix_showIcon(8, 2, 2, CYAN);
	Matrix_chonvitri(32, 2);
	sprintf(msg, "%03d%%", LDR);
	Matrix_guichuoi(msg, CYAN);
	memset(msg, 0, sizeof(msg));
	Matrix_showIcon(8, 12, 3, BLUE);
	if(MIC>45){
		Matrix_showIcon(20, 12, 5, RED);
	}
	else{
		Matrix_showIcon(20, 12, 6, GREEN);
	}
	Matrix_chonvitri(32, 12);
	sprintf(msg, "%02ddB", MIC);
	Matrix_guichuoi(msg, BLUE);
	memset(msg, 0, sizeof(msg));
	Matrix_showIcon(8, 22, 4, RED);
	if(MQ2>45){
		Matrix_showIcon(20, 22, 5, RED);
	}
	else{
		Matrix_showIcon(20, 22, 6, GREEN);
	}
	Matrix_chonvitri(32, 22);
	sprintf(msg, "%03d%%", MQ2);
	Matrix_guichuoi(msg, RED);
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_Delay(5000);
}

void normal(){
		weather();
		clock();
		env();
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	init_dht11(&dht, &htim3, GPIOC, GPIO_PIN_3);
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
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  HAL_TIM_Base_Start_IT(&htim4); 
	Matrix_chonvitri(4, 12);
	Matrix_guikitu('W', RED);
	HAL_Delay(100);
	Matrix_chonvitri(12, 12);
	Matrix_guikitu('E', GREEN);
	HAL_Delay(100);
	Matrix_chonvitri(20, 12);
	Matrix_guikitu('L', BLUE);
	HAL_Delay(100);
	Matrix_chonvitri(28, 12);
	Matrix_guikitu('C', MAGENTA);
	HAL_Delay(100);
	Matrix_chonvitri(36, 12);
	Matrix_guikitu('0', WHITE);
	HAL_Delay(100);
	Matrix_chonvitri(44, 12);
	Matrix_guikitu('M', CYAN);
	HAL_Delay(100);
	Matrix_chonvitri(52, 12);
	Matrix_guikitu('E', CYAN);
	HAL_Delay(1000);
	Matrix_clearArea(0,0,64,32);
	//HAL_TIM_Base_Stop_IT(&htim4);
	uint8_t i = requestDataFromESP32();
	while(i != 1 ){
		i = requestDataFromESP32();
	};
	uint8_t result = readDHT11(&dht);
	while(temp == 0){
		result = readDHT11(&dht);
		if(result){
		temp = dht.temperature;
		hum = dht.humidty;
	}
	else{
		temp = 0;
		hum = 0;
	}
	}
  while (1)
  {
		
		//HAL_TIM_Base_Stop_IT(&htim4);
		uint8_t result = readDHT11(&dht);
		if(result){
			temp = dht.temperature;
			hum = dht.humidty;
		}
		uint8_t i = requestDataFromESP32();
		while(i != 1 ){
			i = requestDataFromESP32();
		};
		int k = ADC_Read(ADC_CHANNEL_10);
		LDR = 100 - 100*k/4096;
		int adc_samples[100];
		for (int i = 0; i < 100; i++) {
       adc_samples[i] = ADC_Read(ADC_CHANNEL_11);;
    }
    float rms_voltage = calculate_rms(adc_samples);
		MIC = calculate_db(rms_voltage);
		k = ADC_Read(ADC_CHANNEL_12);
		MQ2 = 100*k/4096; 
		Matrix_clearArea(0,0,64,32);
		if(mod == 1){
			normal();
		}
		else if(mod == 2){
			weather();
		}
		else if(mod == 3){
			clock();
		}
		else if(mod == 4){
			env();
		}
		
		char str[50];
		snprintf(str, sizeof(str), "T %d,H %d, L %d,M %d,Q %d\r\n", temp, hum, LDR, MIC, MQ2);
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
		//HAL_TIM_Base_Start_IT(&htim4);
  }
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xfff-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 359;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA3 PA4 PA5
                           PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB10 PB12 PB13
                           PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
