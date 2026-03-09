/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Callback de tempo - implementação mínima
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "stdio.h"
#include "lwip/dns.h"
#include "lwip/apps/http_client.h"
#include "lwip/apps/httpd_opts.h"
#include "lwip/netbuf.h"
#include "lwip/api.h"
#include "lwip/arch.h"
#include "string.h"  // Para a função strlen()
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERVER_IP        "192.168.0.1"  // IP do seu servidor
#define SERVER_PORT      80             // Porta do seu servidor
#define BUFFER_SIZE 5000				// Tamanho do BUFFER
#define QUANT_ENVIO 5					// Quantidade de Buffers a para serem tradados antes do envio
#define obter_offset 0					// define para obter offset da fase
#define offset_fase 0 //1979			// offset da tensão de fase
#define fator_fase 1.08451				// fator de correação para tensão de fase
#define fator_corrente 2.2
#define VP_3PH1 388.9087				//tensão de pico da fase do sistema
#define AP_3PH1 40						//Corrente de pico do sistema.
#define offset_fase_corrente 2574		//Offset da corrente.
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
//buffers usado para a coleta dos adcs via DMA ping pong.
uint16_t buffer1[BUFFER_SIZE];
uint16_t buffer2[BUFFER_SIZE];
uint16_t buffer_a_1[BUFFER_SIZE];
uint16_t buffer_a_2[BUFFER_SIZE];

int buffer_value=0;
float media_return=0.0;
int quantidade_soma_media=0;
float soma_media=0.0;
float soma_media_corrente = 0.0;
float media_fase_a =  0.0;
float media_corrente_a =  0.0;

ip_addr_t resolved_ip; //ip resolvido via dns.
extern struct netif gnetif;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC2_Init(void);
/* USER CODE BEGIN PFP */

static void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
void resolve_dns(const char *hostname);
void get_ip_address(void);
void enviarRequisicaoHttpPost(struct tcp_pcb *pcb, const void *data, u16_t len);
void initiate_http_request(float media,float corrente);
float calcular_rms(uint16_t arr[], uint8_t buffer_num);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
err_t data_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    // Implemente o que precisa ser feito após o reconhecimento dos dados pelo host remoto
    return ERR_OK;
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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_LWIP_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  //Usado para coletar o ip via dhcp.
  for (int i=0; i<100000000;i++){
	  MX_LWIP_Process();
	  if (netif_is_up(&gnetif) && (gnetif.ip_addr.addr != 0)) {
		  break;
	  }
  }
  get_ip_address();
  HAL_TIM_Base_Start(&htim2);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer1, BUFFER_SIZE);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)buffer_a_1, BUFFER_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (buffer_value==1){
		  media_return=calcular_rms(buffer1,1);
		  soma_media+=media_return;
		  media_return = calcular_rms(buffer_a_1,1);
		  soma_media_corrente += media_return;
		  quantidade_soma_media+=1;
	  }
	  else if(buffer_value==2){
		  media_return=calcular_rms(buffer2,1);
		  soma_media+=media_return;
		  media_return = calcular_rms(buffer_a_2,1);
		  soma_media_corrente += media_return;
		  quantidade_soma_media+=1;
	  }
	  if (quantidade_soma_media==5){
		  soma_media=(soma_media/5);
		  soma_media_corrente = (soma_media_corrente/5);
		  media_fase_a=(soma_media/(4095-offset_fase))*VP_3PH1*fator_fase; //valor de correção -> e transformação para VRMS
		  media_corrente_a = ((soma_media_corrente-offset_fase_corrente)/(4095-offset_fase_corrente))*AP_3PH1*fator_corrente;
		  printf("tensão = %.2f  corrente = %.2f valor do buffer %f \n\n",media_fase_a,media_corrente_a,soma_media_corrente);
		  initiate_http_request(media_fase_a,media_corrente_a);
		  quantidade_soma_media=0;
		  soma_media=0.0;
	  }
	  MX_LWIP_Process();
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
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
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 49;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  USART3->CR1 |= USART_CR1_RXNEIE;
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//configuração ADC, DMA
void HAL_ADC_ConvCptCallBack(ADC_HandleTypeDef* hadc){

HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
}
//fim da configuração do ADC

//configuração do DMA callback flag de buffer cheio
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->DMA_Handle->Instance->M0AR == (uint32_t)buffer1) {
    	buffer_value = 1;
    	HAL_ADC_Stop_DMA(&hadc1);  // Pare  DMA para alterar o buffer
    	HAL_ADC_Stop_DMA(&hadc2);  // Pare  DMA para alterar o buffer
    	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer2, BUFFER_SIZE);
    	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)buffer_a_2, BUFFER_SIZE);

    	// Buffer 1 foi preenchido
    } else if (hadc->DMA_Handle->Instance->M0AR == (uint32_t)buffer2) {
    	buffer_value = 2;
    	HAL_ADC_Stop_DMA(&hadc1);  // Pare  DMA para alterar o buffer
    	HAL_ADC_Stop_DMA(&hadc2);  // Pare  DMA para alterar o buffer
    	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer1, BUFFER_SIZE);
    	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)buffer_a_1, BUFFER_SIZE);
    	// Buffer 2 foi preenchido
    }
}

static void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr != NULL) {
        // Endereço IP resolvido com sucesso
        resolved_ip = *ipaddr;
        printf("Endereço IP resolvido: %s\n", ipaddr_ntoa(&resolved_ip));
    } else {
        // Falha na resolução DNS
        printf("Falha na resolução DNS para: %s\n", name);
    }
}
void resolve_dns(const char *hostname) {
    err_t err = dns_gethostbyname(hostname, &resolved_ip, dns_found, NULL);

    if (err == ERR_INPROGRESS) {
        // A resolução DNS está em andamento
        printf("Resolução DNS em andamento para: %s\n", hostname);
    } else {
        // Falha ao iniciar a resolução DNS
        printf("Falha ao iniciar a resolução DNS para: %s\n", hostname);
    }
}
void get_ip_address(void) {
    // Verifica se a interface de rede está pronta e tem um endereço IP
    if (netif_is_up(&gnetif) && (gnetif.ip_addr.addr != 0)) {
        printf("IP Address: %u.%u.%u.%u\r\n",
               ip4_addr1(&(gnetif.ip_addr)),
               ip4_addr2(&(gnetif.ip_addr)),
               ip4_addr3(&(gnetif.ip_addr)),
               ip4_addr4(&(gnetif.ip_addr)));
    } else {
        printf("IP Address not available\r\n");
    }
}

void enviarRequisicaoHttpPost(struct tcp_pcb *pcb, const void *data, u16_t len) {
// Verifique o espaço disponível na fila de saída
	if (tcp_sndbuf(pcb) >= len) {
		// Enfileire os dados
		err_t write_err = tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);

		if (write_err == ERR_OK) {
			// Força o envio imediato dos dados enfileirados
			tcp_output(pcb);
		} else {
			printf("ERROR1");
		}
	} else {
		// A fila de saída não tem espaço suficiente, aguarde ou trate conforme necessário
		printf("ERROR2");
	}
}

void initiate_http_request(float media,float corrente) {
    // Crie um PCB (Protocol Control Block) TCP
    struct tcp_pcb *tcp_pcb = tcp_new();
    float aux=media;
    char float_str[20];
    snprintf(float_str, sizeof(float_str), "%.2f", aux);
    // Configure o endereço IP e a porta do servidor
    ip_addr_t server_ip;
    IP4_ADDR(&server_ip, 192, 168, 252, 199);

    // Conecte ao servidor
    tcp_connect(tcp_pcb, &server_ip, 80, NULL);

    // Envie dados após a conexão ser estabelecida (isso pode ser feito em uma função de callback)
    tcp_sent(tcp_pcb, data_sent_callback);
    char json_data[64]; // Buffer para o JSON
    int json_len = snprintf(json_data, sizeof(json_data),
                             "{\"voltage\":%.2f,\"current\":%.2f}",
                             media, corrente);

     // Verifique se o JSON foi formatado corretamente
     if (json_len < 0 || json_len >= sizeof(json_data)) {
       // Trate erro de formatação (buffer pequeno ou falha)
       return;
     }
     // 3. Monte a requisição HTTP POST
       char http_request[256]; // Buffer para a requisição HTTP
       int http_len = snprintf(http_request, sizeof(http_request),
                               "POST /voltage_current HTTP/1.1\r\n"
                               "Host: 192.168.252.199:80\r\n"
                               "Content-Type: application/json\r\n"
                               "Content-Length: %d\r\n\r\n"
                               "%s", json_len, json_data);

       // Verifica se a requisição cabe no buffer
       if (http_len < 0 || http_len >= sizeof(http_request)) {
         // Trata erro de formatação
         return;
       }
    u16_t data_len = strlen(http_request);


    enviarRequisicaoHttpPost(tcp_pcb, http_request, data_len);
}


float calcular_rms(uint16_t arr[], uint8_t buffer_num){
	uint64_t soma_quadratica=0;
	float media=0;
	int16_t valor=0;
	if(obter_offset==1){
		for (int i=0;i<BUFFER_SIZE;i++){
			soma_quadratica+=arr[i];
		}
		 media=soma_quadratica/BUFFER_SIZE;
		 buffer_value=0;
		return media;
	}
	else{
		for (int z=0;z<BUFFER_SIZE;z++){
			if(buffer_num==1){
				valor=arr[z]-offset_fase;}
			else if(buffer_num==2){
				valor=arr[z]-offset_fase_corrente;
			}
			soma_quadratica+=(valor*valor);
		}
		media=sqrt(soma_quadratica/BUFFER_SIZE);
		buffer_value=0;
		return media;
	}
	return 0.0;
}
int __io_putchar(int ch)
{
	USART3->RDR = ch;						//transmite o dado
	while (!(USART3->ISR & USART_ISR_TXE));	//espera pelo fim da transmissão
	return ch;
}

// Sobrescreve a função _write para redirecionar a saída para a USART3
int _write(int file, char *data, int len)
{
    // Transmite os dados via USART3 (bloqueante)
    HAL_UART_Transmit(&huart3, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}


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
