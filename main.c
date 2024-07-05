/* USER CODE BEGIN Header */
#include "4G.h"
#include "led.h"
#include "rtc.h"
#include "stmflash.h"
#include "delay.h"
#include "iwdg.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void tryRptSnd(u8 times);
void tryRptFTPConn(u8 times);
void tryRpt4GConn(u8 times);
void sendReport(void);
void getDATEymd(void);
void SystemClock_Config(void);
static void MPU_Config(void);
void sd_test(void);
extern RTC_TimeTypeDef RTC_TimeStruct;
extern RTC_DateTypeDef RTC_DateStruct;
extern char DATE1[];
extern char DATE[];
extern char shuju1[ADCSIZE*3+99];
extern char VTVALStr[20];

const u8 test[]={"123456789123456789123456789123456789"};
char flashtest[32]={0};
u16 VThresholdDynamic=2482,VThresholdGet=2482;

u8 rptSndErrFlg=0,rptFtpConnErrFlg=0,rpt4GConnErrFlg=0;		//��reportʱ��״̬λ��report���ݷ��ʹ����־;fpt���Ӵ����־;4Gģ�����Ӵ����־
int trigMin=0,reportMin=0,currentMin=0,getVThMin=0;	//����ʱ�̡�����ʱ�̡���ǰʱ�̵��ۼƷ�����
u8 reportCount=0;		//��¼report�����������ǲ��ǿ�����ĵ�һ��report
//report�ļ�����
char firstReport[20]={"Initial Report:"};
char sta4G[15]={"4G state:"};
char staTrig[20]={"Trigger state:"};
char staSD[15]={"SD state:"};
char shujurep[280]={0}; 			//report��������

const u8 VThStr[4]={0};
extern char ymd[12];
extern char hms[9];
extern char DATE2_TWO[25];
extern char RxBuffer[RXBUFFERSIZE];   //��������
extern  uint8_t  USART2_RX_BUF[256];
extern int flagtrig;  //������־

  FATFS   fs;         /* FATFS �ļ�ϵͳ���� */
  FIL     fil;         /* FATFS �ļ�����    */

  uint32_t byteswritten;                /* File write counts */
  uint32_t bytesread;                   /* File read counts */
  uint8_t wtext[100] = "This is Pandior's blog!";                /* File write buffer */
  uint8_t rtext[100];                   /* File read buffers */
  char filename[50] = "sd_test.txt";


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int m;
  Cache_Enable();                			//��L1-Cache
  MPU_Config();
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
//  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
	Stm32_Clock_Init(160,5,2,2);
		delay_init(400);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	
  HAL_ADCEx_Calibration_Start(&hadc1,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED); //ADCУ׼
	
	 LED_Init();
	 RTC_Init();                         //RTCʱ���ʼ��
 
	 IWDG_Init(IWDG_PRESCALER_256,4000); 	//��Ƶ��Ϊ256,����ֵΪ4000,���ʱ��Ϊ30s	����
//  printf("weigou");
	
	HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);	
	printf("Time:%02d:%02d:%02d",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
	
	HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
	printf("Date:20%02d-%02d-%02d\r\n",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date);
	
		
//   memcpy(flashtest, test+27 , 5);
//	 printf("flash:%s",flashtest);

  
//	 STMFLASH_Write(FLASH_SAVE_ADDR2,(u32*)test+2,1);    //д������ݴ�С����Ϊ32�ı�
//////////	delay_ms(1000);
//	 STMFLASH_Read(FLASH_SAVE_ADDR2,(u32*)flashtest,10);
//	 printf("flash:%s",flashtest);
	IWDG_Feed();
	tryInit4GConn(15);		//��������4Gģ��15��
	if(NETSTA==0)					//û���Ͼ�����һ��
	{
		printf("�ٴγ���������������\r\n");
		tryInit4GConn(15);	
	}
	 
	 GetRTCCLK();
	 printf("����RTCʱ��Ϊ: %s\r\n",DATE1);
	

	if(NETSTA)
	{
	 if(((VThresholdDynamic=getVTh((u8 *)pathnameMacro))>0)&&(VThresholdDynamic<4096))
		 {
//			 
////			 
////			/** Configure Analog WatchDog 1
////  */
////	ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
////  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
////  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
////  AnalogWDGConfig.Channel = ADC_CHANNEL_19;
////  AnalogWDGConfig.ITMode = ENABLE;
////  AnalogWDGConfig.HighThreshold = 0x9b2;
////  AnalogWDGConfig.LowThreshold = 0;
////  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
////  {
////    Error_Handler();
////  }
//		
	
		  printf("�����ȡ��ֵΪ: %d\r\n",VThresholdDynamic);
			 
			//flash������ֵ����
//	    sprintf(VThStr,"%04d",VThresholdDynamic);
//	    STMFLASH_Write(FLASH_SAVE_ADDR1,(u32 *)VThStr,32);
			
//	printf("дflash�ɹ�");
//	    STMFLASH_Read(FLASH_SAVE_ADDR1,(u32 *)VThStr,32);

			 
		} 
		 


			if((GetNetCLK()==0))
		{
//			printf("����RTCʱ��\r\n");
			
			RTC_Set_Date((u8)local_time.year,(u8)local_time.month,(u8)local_time.day,1);		//��������
			RTC_Set_Time((u8)local_time.hour,(u8)local_time.minute,(u8)local_time.second,RTC_HOURFORMAT12_PM);	//����ʱ��  
		} 
		
		
		
		GetRTCCLK();
		printf("����RTCʱ��Ϊ: %s\r\n",DATE1);
	}
	

	__HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_AWD);
  /* USER CODE END 2 */

//	  sd_test();
	
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

		if(NETSTA)
		{	
			
			if(flagtrig)
			{
				//�ϴ�4G���ݷ��͸�ʽ����	
				
			getDATEymd();
			strncat(DATE1,".DAT",30);		//�ļ�����ʽ
			printf("FTP�ļ���DATE1=====%s\r\n",DATE1);
    	tryDatSnd(5);				//�������������������5��
			tryDatFTPConn(8);		//����ʧ��������������8��				
			tryDat4GConn(10);		//����������ʧ��������4G������FTP�����������ط�5��
				
	
			 
				printf("\r\n ******  sd  write ******\r\n\r\n");
   retSD = f_mount(&fs, "", 0);
   if(retSD)
   {
       printf(" mount error : %d \r\n",retSD);
       Error_Handler();
   }
   else
       printf(" mount sucess!!! \r\n");
   /*##-2- Create and Open new text file objects with write access ######*/
   retSD = f_open(&fil, 	DATE1, FA_OPEN_ALWAYS|FA_WRITE);
   if(retSD)
       printf(" open file error : %d\r\n",retSD);
   else
       printf(" open file sucess!!! \r\n");

	 
   /*##-3- Write data to the text files ###############################*/
	  STMFLASH_Read(FLASH_SAVE_ADDR1,(u32 *)VTVALStr,5);
	  retSD = f_write(&fil, VTVALStr, 17, (void *)&byteswritten);
	 
  		memset(shuju1,0,sizeof(shuju1));
			STMFLASH_Read(FLASH_SAVE_ADDR2,(u32*)shuju1,5000);
      retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten);
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR3,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten);
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR4,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten);
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR5,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten);
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR6,(u32*)shuju1,2500);
	 retSD = f_write(&fil, shuju1, 10000, (void *)&byteswritten);
	
	
	
	
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR9,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten);
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR10,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten); 
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR11,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten); 
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR12,(u32*)shuju1,5000);
	 retSD = f_write(&fil, shuju1, 20000, (void *)&byteswritten); 
	 
	 memset(shuju1,0,sizeof(shuju1));
	 STMFLASH_Read(FLASH_SAVE_ADDR13,(u32*)shuju1,2500);
	 retSD = f_write(&fil, shuju1, 10000, (void *)&byteswritten);
	 
	
   if(retSD)
       printf(" write file error : %d\r\n",retSD);

   retSD=f_write(&fil, "\r\n", sizeof("\r\n")-1, (void *)&byteswritten);
   if(retSD)
       printf(" write file error : %d\r\n",retSD);

   /*##-4- Close the open text files ################################*/
   retSD = f_close(&fil);
   if(retSD)
       printf(" close error : %d\r\n",retSD);
   else
       printf(" close sucess!!! \r\n");	
				
				
      flagtrig=0;	
			memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
			memset(RxBuffer,0,sizeof(RxBuffer));
			memset(DATE1,0,30);	
				
				
			}
			

//			strncat(shuju2,VTVALStr,20);	//�ļ����ݸ�ʽ
//			strcat(shuju2, "\r\n");
//			strcat(shuju1, "\r\n");
//			strncat(shuju1,VTVALStr,DATASIZE*3+100);	
//			strncat(shuju2, shuju1,DATASIZE*3);
//			
////			shuju1[DATASIZE*3+99]='\0';
////			shuju1[DATASIZE*3+99]='\0';
//			//STMFLASH_Write(FLASH_SAVE_ADDR,(u8 *)shuju2,24100);	//flash�������һ������
//			

//			
//			//NETSTA=0;
//			//printf("DATE1000000000000: %s\r\n",DATE1);
//			//printf("pathname0000000: %s\r\n",pathname);
//		



//		}
//    printf("123");

		delay_ms(1000);
		LED0_Toggle;
    IWDG_Feed();
//		HAL_Delay(1000);



  }
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//ÿ���Сʱ����һ�α���	 
	 sendReport();
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


	void sd_test(void)
	{
	 printf("\r\n ****** FatFs Example ******\r\n\r\n");
   retSD = f_mount(&fs, "", 0);
   if(retSD)
   {
       printf(" mount error : %d \r\n",retSD);
       Error_Handler();
   }
   else
       printf(" mount sucess!!! \r\n");
   /*##-2- Create and Open new text file objects with write access ######*/
   retSD = f_open(&fil, "weekend.txt", FA_OPEN_ALWAYS|FA_WRITE);
   if(retSD)
       printf(" open file error : %d\r\n",retSD);
   else
       printf(" open file sucess!!! \r\n");

   /*##-3- Write data to the text files ###############################*/
  
   retSD = f_write(&fil, wtext, strlen((const char*)wtext), (void *)&byteswritten);
	 retSD = f_write(&fil, wtext, strlen((const char*)wtext), (void *)&byteswritten);
   if(retSD)
       printf(" write file error : %d\r\n",retSD);

   retSD=f_write(&fil, "\r\n", sizeof("\r\n")-1, (void *)&byteswritten);
   if(retSD)
       printf(" write file error : %d\r\n",retSD);

   /*##-4- Close the open text files ################################*/
   retSD = f_close(&fil);
   if(retSD)
       printf(" close error : %d\r\n",retSD);
   else
       printf(" close sucess!!! \r\n");
	
	}

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}


//�����������ڸ�ʽ
void getDATEymd()
{
	memset(DATE,0,19);
	sprintf(DATE,"20%02d%02d%02d%02d%02d%02d",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
	print(DATE,3,6,ymd);//��ȡ������
	print(DATE,9,6,hms);
	print(DATE,1,14,DATE2_TWO);//��ȡ������ʱ���뵽report.txt���������XXXXXXX
	memcpy(DATE1,DATE,19);
	//sprintf(ymd,"%02d%02d%02d",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date);
}


void sendReport()
{
//	printf("staSD000000000======%s\r\n",staSD);
	
	//printf("sHOURT2======%d\r\n",HOUT2);
	//printf("sHOURT1======%d\r\n",HOUT1);
	
	delay_ms(200);
	
	HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);	
	//GetRTCCLK();
	currentMin=RTC_DateStruct.Date*24*60+RTC_TimeStruct.Hours*60+RTC_TimeStruct.Minutes;
	
	
	
	//ÿ20min��ȡһ����ֵ
//	if((currentMin-getVThMin>4)&&(!flagtrig))
//		{
//			if(NETSTA) tryInitFTPConn(5);		//����FTP������
//			else tryInit4GConn(8);

////			if(((VThresholdGet=getVTh((u8 *)pathnameMacro))>0)&&(VThresholdGet<4096)&&NETSTA)
////				{
////					VThresholdDynamic=VThresholdGet;
////					//flash������ֵ����
////					sprintf(VThStr,"%04d",VThresholdDynamic);
////					
//////					STMFLASH_Write(FLASH_SAVE_ADDR1,(u32 *)VThStr,sizeof(VThStr));
////					
////					getVThMin=currentMin;
//////					ADC_WatchdogConfig(VThresholdDynamic);
////					printf("�����ȡ��ֵΪ: %d\r\n",VThresholdDynamic);
////				}
////				else 
////				{
////					printf("ʹ���ϴ���ֵ���ã�%d\r\n",VThresholdDynamic);
////				}
//		}
	
	if((currentMin-reportMin>59)&&(!flagtrig)&&NETSTA)
	{

		getDATEymd();
			
//		IWDG_ReloadCounter();		//ι����

		//�Ƿ��һ�η�report
		if(reportCount==0)
		{
			strncat(firstReport,"YES",20);
		}
		else strncat(firstReport,"NO",20);
		
		//4G״̬�Ƿ�����
		if(NETSTA)
		{
			strncat(sta4G,"YES", 15);
		}
		else
		{
			strncat(sta4G,"NO", 15);
		}
		//printf("flagtrig======%d\r\n",flagtrig);
		
		//1h���Լ���ǰ�Ƿ��й�����
		if((flagtrig)||((currentMin-trigMin)<61))		//֮ǰ1h�ڷ������׻���ǰ�����׻���Trigger state���ΪYES
		{
			strncat(staTrig,"YES", 20);
		}
		else
		{
			strncat(staTrig,"NO", 20);
		}
		//printf("staTrig======%s\r\n",staTrig);
		//printf("flagSD======%d\r\n",flagSD);
	
		//�ܷ��ȡ��SD��
//		if(f_open(&fnew1,"0:/FLAGSD",FA_CREATE_ALWAYS|FA_READ)==FR_OK)
//		{
//			strncat(staSD,"YES", 15);
//		}
//		else
//		{
//			strncat(staSD,"NO", 15);
//		}
				
		//����report�ļ���ʽ����
		strcat(DATE2_TWO,"report.txt");
		strcat(shujurep,firstReport);
		strcat(shujurep,"\r\n");
		strcat(shujurep,sta4G);	
		strcat(shujurep,"\r\n");
		strcat(shujurep,staTrig);
		strcat(shujurep,"\r\n");
		strcat(shujurep,staSD);
		//strcat(shujurep,"'\0'");
		//shujurep[199]='\0';
		//printf("DATE2_TWO======%s\r\n",DATE2_TWO);	
		//reportMin=currentMin;
		//EC20_FTP_SEND_DATA((u8 *)pathname,(u8 *)DATE2_TWO,(char *)shujurep);
		
		if(NETSTA)						//�ٴ��ж�4G״̬�Ƿ�ִ�з���report
		{
			printf("shujurep======\r\n%s\r\n",shujurep);
			
			tryRptSnd(6);				//����report����
			tryRptFTPConn(10);		//����ʧ����������FTP�ٷ���
			tryRpt4GConn(12);			//����FTPʧ��������4Gģ�顢����FTP���ٷ���
						
   		IWDG_Feed();		//ι��
			
//			flagSD=flagGPS=0;   
		memset(USART2_RX_BUF,0x0,sizeof(USART2_RX_BUF));
		memset(RxBuffer,0,sizeof(RxBuffer));
			
			memset(DATE2_TWO,0,25);
			memset(shujurep,0,200);
			memset(DATE1,0,30);
			memset(firstReport,0,20);
			memset(sta4G,0,15);
			memset(staTrig,0,15);
			memset(staSD,0,15);
					
			strcat(firstReport,"Initial Report:");
			strcat(sta4G,"4G state:");
			strcat(staTrig,"Trigger state:");
			strcat(staSD,"SD state:");
			//}
	
			if(reportCount==0)
			{
				reportCount=1;
				//ADC_ITConfig(ADC1,ADC_IT_AWD,ENABLE);
			}
		}
	}
}

/********
	����tryRptSnd()��tryRptFTPConn()��tryRpt4GConn()Ϊ����report����
*******/
//�ȳ��Է���report����
void tryRptSnd(u8 times)
{
	u8 rptSndErrCt=0;	//report send error count
	while(NETSTA)
	{
 		IWDG_Feed();		//ι��
		printf("report_���ݳ��Է��ͣ�%d��\r\n",rptSndErrCt+1);
		rptSndErrFlg=(EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE2_TWO,(char *)shujurep));
		rptSndErrCt++;
	
		if(rptSndErrFlg==0)
		{
			printf("report_���ݷ��ͳɹ�\r\n");
			reportMin=currentMin;
			break;
		}
		
		if(rptSndErrCt>times)
		{
			printf("report_���ݷ���ʧ��\r\n");
			break;
		}
	}
}

//report���ݷ���ʧ�ܣ��ٳ�������FTP������
void tryRptFTPConn(u8 times)
{
	u8 rptFTPConnErrCt=0;		//report FTP connect error count
	while(rptSndErrFlg&&NETSTA)
	{
    IWDG_Feed(); 	//ι��
		printf("report_��������ftp��������%d��\r\n",rptFTPConnErrCt+1);
		rptFtpConnErrFlg=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)usernameMacro,(u8 *)passwordMacro,(u8 *)websiteMacro);
		rptFTPConnErrCt++;
		if(rptFtpConnErrFlg==0)
		{
			printf("report_FTP���������ӳɹ�\r\n");
			printf("report_���ͱ�������\r\n");
			tryRptSnd(6);					//����FTP�ɹ����Է���report����
			break;
		}
		
		if(rptFTPConnErrCt>times)
		{
			printf("report_FTP����������ʧ��\r\n");
			break;
		}
	}
}

//FTP���ӳ������ٳ�������4Gģ��
void tryRpt4GConn(u8 times)
{
	u8 rpt4GConnErrCt=0;					//report 4G connect error count
	while(rptFtpConnErrFlg&&NETSTA)
	{
    IWDG_Feed();		//ι��
		printf("report_��������4Gģ�飺%d��\r\n",rpt4GConnErrCt+1);
		rpt4GConnErrFlg=EC20_INIT();
		rpt4GConnErrCt++;
		if(rpt4GConnErrFlg==0)
		{
			printf("report_4Gģ�����ӳɹ�\r\n");
			NETSTA=1;
			tryRptFTPConn(10);
			break;
		}
		
		if(rpt4GConnErrCt>times)
		{
			printf("report_4Gģ������ʧ��\r\n");
			NETSTA=1;
			break;
		}
	}
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
