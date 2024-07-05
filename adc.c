/* USER CODE BEGIN Header */
#include "USART.h"
#include "delay.h"
#include "iwdg.h"
#include "stmflash.h"
#include "rtc.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */

extern char VTVALStr[20];
extern char DATE1[30];
uint16_t ADC_Value[ADCSIZE]={0};
u16 s_cnt=0;
u16 p_cnt=0;
u16 DMA_CNT=0;	//DMA点计数
int flagtrig=0;  //触发标志
u8 revFlag=0;			//前后调换数据标志
int16_t DMATrig=0;
char shuju1[ADCSIZE*3+99]={0};	//转换的字符数组
char shuju2[90100]={0};	//转换的字符数组
#define VThresholdDynamic   2482

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analog WatchDog 1
  */
  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  AnalogWDGConfig.Channel = ADC_CHANNEL_19;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = 0x9B2;
  AnalogWDGConfig.LowThreshold = 0;
  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_19;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
	
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Value,ADCSIZE);
//	__HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_AWD);
  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M = 2;
    PeriphClkInitStruct.PLL2.PLL2N = 12;
    PeriphClkInitStruct.PLL2.PLL2P = 9;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA5     ------> ADC1_INP19
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Stream0;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA5     ------> ADC1_INP19
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    /* ADC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC_IRQn);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
       
     __HAL_ADC_DISABLE_IT(&hadc1, ADC_IT_AWD);

      printf("模拟看门狗中断\r\n");
	    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);   // 进入中断的提示灯
	        
	  	DMA_CNT = ADCSIZE-__HAL_DMA_GET_COUNTER(&hdma_adc1); 			
//	    delay_us(18857);
	
//	    delay_us(18009); //20000个点             0.9 US 一个数据
//	        delay_us(27009); //30000 个点
//	    delay_us(36009);
	     delay_us(54009);   //60010个点
//    	delay_us(61441);   //65536个点
//	    delay_us(75000);   //80000个点
		 
    	HAL_DMA_Abort(&hdma_adc1);           //关闭DMA传输
			HAL_ADC_Stop(&hadc1);               //ADC采集传输通道关闭
		  flagtrig=1;
	    GetRTCCLK();  
//      printf("时间：%s",DATE1);	
	   	printf("\r\n\r\nVTVAL======%d/%d\r\n",ADC_Value[DMA_CNT],VThresholdDynamic);
	    printf("DMA_CNT=%d\r\n",DMA_CNT);
	    
			sprintf(VTVALStr,"VTVAL=%04d/%04d\r\n",ADC_Value[DMA_CNT],VThresholdDynamic);	//写入触发值和阈值
			
	if(DMA_CNT<200) 
			{
				revFlag=0;
			}
			else
			{
				revFlag=1;
//				DMA_CNT=DMA_CNT-198;	//+2-200=-198
			}
			
			if(revFlag)
			{
				//将断点之后的数据移到前面
				p_cnt=DMA_CNT;
				for(s_cnt=0;s_cnt<ADCSIZE-DMA_CNT+10;s_cnt++)
				{
					sprintf(shuju1+s_cnt*3,"%03x",ADC_Value[p_cnt++]);
				}
				//再将断点之前的数据接到后面
				p_cnt=0;

				for(s_cnt=ADCSIZE-DMA_CNT;s_cnt<ADCSIZE;s_cnt++)
				{
					sprintf(shuju1+s_cnt*3,"%03x",ADC_Value[p_cnt++]);
				}
			}
			else
			{
				for(s_cnt=0;s_cnt<ADCSIZE-200;s_cnt++)
				{
				sprintf(shuju1+s_cnt*3,"%03x",ADC_Value[s_cnt]);				
				}
				
				for(s_cnt=ADCSIZE-200;s_cnt<ADCSIZE;s_cnt++)	//将最后200个数值补0，补齐8000个数
				{
					sprintf(shuju1+s_cnt*3,"%03x",1862);	//0x746=1862
				}
			}
	    
			DMA_CNT=0;
			DMATrig=0;
			shuju1[s_cnt*3]='\0';

     // 写阈值
			STMFLASH_Write(FLASH_SAVE_ADDR1,(u32*)VTVALStr,32);
			
     //写数据
		  STMFLASH_Write(FLASH_SAVE_ADDR2,(u32*)shuju1,22500); //写入的数据大小必须为32的倍数

     //一次写不完，分两次写
			
//			memcpy(shuju2, shuju1+90000-1, 90000);
		  STMFLASH_Write(FLASH_SAVE_ADDR9,(u32*)shuju1+22500,22500);

//	
			
		  __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_AWD);	
      HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Value,ADCSIZE);	
//			
//			printf("\r\n%s\r\n",shuju1);
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);   // 进入中断的提示灯
}
/* USER CODE END 1 */
