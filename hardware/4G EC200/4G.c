
#include <stdlib.h>
#include "usart.h"
#include <string.h>
#include "stdio.h"
#include "4G.h"
#include "delay.h"
#include "usart.h"
#include "main.h"
#include "adc.h"
#include "stmflash.h"
#include "iwdg.h"


extern u8 USART2_RX_STA;
extern uint8_t  USART2_RX_BUF[256];
extern char DATE1[30];
extern char RxBuffer[RXBUFFERSIZE];   //��������
extern	char shuju1[ADCSIZE*3+99];

int NETSTA=0;			//����״̬
u8 init4GConnErrFlg=0,initFtpConnErrFlg=0;
u8 dat4GConnErrFlg=0,datFTPConnErrFlg=0,datSndErrFlg=0;		//��date�׵�����״̬λ
//char sendbuf[1*1024];


u8 EC20CSQ[BUFLEN];

char AtStrBuf[BUFLEN];   								//��ӡ������
char VTVALStr[20]={0};				         //������ֵ��Ϣ����Ϊ���ݵ�ͷ��


u8 Flag_Rec_Message=0;
ST_Time  local_time;	//����ʱ��ṹ��


//EC20�������(���Ų��ԡ����Ų��ԡ�GPRS����)���ô���
//EC20���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* EC20_check_cmd(u8 *str)
{
    char *strx=0;
	 // printf("99999daozhele\r\n");
    if(USART2_RX_STA)		//���յ�һ��������
    {			  
//        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
//			  HAL_UART_Transmit(&huart1, (uint8_t *)&USART2_RX_BUF, 10, 0xffff);
        strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
			
//			  HAL_UART_Transmit(&huart1, (uint8_t *)&strx, 10, 0xffff);
//       printf("ATCMD<-\r\n %s\r\n",USART2_RX_BUF);//��������
    }
    return (u8*)strx;
}

void check_cmd(void)
{
    if(USART2_RX_STA&0X8000)		//���յ�һ��������
    {
        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
    }
}

//��EC20��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 EC20_send_cmd(u32 *cmd,u8 *ack,u16 waittime)
{
    u8 res=0;
	
    USART2_RX_STA=0;
	            
//    if((u32)cmd<=0XFF)
//    {
//        while(DMA1_Stream6->NDTR!=0);	//�ȴ�ͨ��4�������
//        USART2->TDR=(u32)cmd;
//    }
//		else 
	  u2_printf("%s\r\n",cmd);//��������
//	  delay_ms(10);
	        
   // printf("ATCMD->\r\n %s\r\n",cmd);//��������
    if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
    {

        while(--waittime)	//�ȴ�����ʱ
        {
            HAL_Delay(10);
            if(USART2_RX_STA)//���յ��ڴ���Ӧ����
            {
		        // printf("\n22222222===\r\n");

                if(EC20_check_cmd(ack))break;//�õ���Ч����
                USART2_RX_STA=0;
							
//							memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
//							memset(RxBuffer,0,sizeof(RxBuffer));
            }
        }
        if(waittime==0)res=1;
    }
    USART2_RX_STA=0;

    return res;
}
u8 EC20_send_cmd2(u32 *cmd,u8 *ack,u16 waittime)
{
    u8 res=0;
	 
    u2_printf("%s\r\n",cmd);//��������
//    printf("ATCMD->\r\n %s\r\n",cmd);//��������
    if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
    {
//			printf("ATCMD->11111\r\n %s\r\n");//��������
			
        while(--waittime)	//�ȴ�����ʱ
        {
           HAL_Delay(100);
            if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
            {
//		            printf("\n22222222===\r\n");
                if(EC20_check_cmd(ack))break;//�õ���Ч����
                USART2_RX_STA=0;
            }
        }
        if(waittime==0)res=1;
    }
    USART2_RX_STA=0;
    return res;
}

u8 EC20_work_test(void)
{
    memset(EC20CSQ,0,BUFLEN);
    if(EC20_send_cmd((u32 *)"ATE1",(u8 *)"OK",100))
    {
			if(EC20_send_cmd((u32 *)"ATE1",(u8 *)"OK",100))	return SIM_COMMUNTION_ERR;	//ͨ�Ų���
    }
    if(EC20_send_cmd((u32 *)"AT+CPIN?",(u8 *)"READY",400))	return SIM_CPIN_ERR;	//û��SIM��
    if(EC20_send_cmd((u32 *)"AT+CREG?",(u8 *)"0,1",400))	//���0��1 ���� 0��5
    {
        if(strstr((const char*)USART2_RX_BUF,"0,5")==NULL)
        {
            if(!EC20_send_cmd((u32 *)"AT+CSQ",(u8 *)"OK",200))
            {
                memcpy(EC20CSQ,USART2_RX_BUF+15,2); 
            }
            return SIM_CREG_FAIL;	//�ȴ����ŵ�����
        }
    }
    return SIM_OK;
}



u8 EC20_INIT(void)
{
    u8 res;
	
    res=EC20_work_test();
	
    switch(res)
    {
    case SIM_OK:
        printf("4Gģ���Լ�ɹ�\r\n");
        break;
    case SIM_COMMUNTION_ERR:
        printf("�������ӵ�4Gģ��,���Ե�...\r\n");
        break;
    case SIM_CPIN_ERR:
        printf("���ڼ�⵽SIM��,���Ե�..\r\n");
        break;
    case SIM_CREG_FAIL:
        printf("ע��������...\r\n");
				if(strstr((const char*)EC20CSQ,"99"))
				{
					printf("���ź�,�����ʣ�%s%%\r\n",EC20CSQ);
				}
				else printf("��ǰ�ź�ֵ��%s\r\n",EC20CSQ);
        break;
    default:
        break;
    }
    return res;
}

u8 EC20_CONNECT_SERVER_CFG_INFOR(u8 *username,u8 *password,u8 *website)
{
    u8 res;
    res=EC20_CONNECT_FTP_SERVER(username,password,website);
    return res;
}
 
u8 EC20_CONNECT_FTP_SERVER(u8 *username,u8 *password,u8 *website)
{
	  
	
	  if(EC20_send_cmd((u32 *)"AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n",(u8 *)"OK",100))	 return 1;   //����PDP context 1, APN is�UNINET  for China Unicom
    if(EC20_send_cmd((u32 *)"AT+QIDEACT=1\r\n",(u8 *)"OK",100))          return 2;  // //���� PDP context 1
    if(EC20_send_cmd((u32 *)"AT+QIACT?\r\n",(u8 *)"OK",100))	 return 3;  ////��ѯPDP״̬
	  if(EC20_send_cmd((u32 *)"AT+QFTPCFG=\"contextid\",1\r\n",(u8 *)"OK",100))	 return 4;  //���� PDP context ID as 1
	

	  delay_ms(1000);
	//Step2:Configure user account and transfer settings*
	 

	  memset(AtStrBuf,0,BUFLEN);
    sprintf(AtStrBuf,"AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n",username,password);
//	  u2_printf("AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n",username,password);
    if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"OK",1000))
		{
		return 5;
	  }
	
	//\�����ļ�������ʽ������
	  if(EC20_send_cmd((u32 *)"AT+QFTPCFG=\"filetype\"1\r\n",(u8 *)"OK",100))	 return 6;
	//���ô���ģʽΪ����ģʽ=1������ģʽ=0
	if(EC20_send_cmd((u32 *)"AT+QFTPCFG=\"transmode\",1\r\n" ,(u8 *)"OK",100))	 return 7;
	//������Ӧ��ʱ
	if(EC20_send_cmd((u32 *)"AT+QFTPCFG=\"rsptimeout\",90\r\n" ,(u8 *)"OK",100))	 return 8;
	/**************Step3: Login to FTP server***********/		
	//ͨ����ַ��¼��FTP������
	//if(EC20_send_cmd((u32 *)"AT+QFTPOPEN=\"%s\",21\r\n" ,(u8 *)"+QFTPOPEN",100))	 return 9 ;
   memset(AtStrBuf,0,BUFLEN);
   sprintf(AtStrBuf,"AT+QFTPOPEN=\"%s\",21\r\n",website);
   if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPOPEN: 0,0",1000))	 return 9;
	
	 //printf("�豸�Ѿ����ӵ�FTP������,׼���������� [..]\r\n");
	 printf("�豸�Ѿ����ӵ�FTP������ [..]\r\n"); 	
	
	 return 0;
	
	
}


/***************��FTP������ע��*******************/
u8 EC20_FTP_Close(void)
{
//   static u8 ec20_ftp_logout = 0;

	if(EC20_send_cmd((u32 *)"AT+QFTPCLOSE\r\n",(u8 *)"OK",100))	 
	{
		printf("\r\nEC20_FTP_Close Success!\r\n");
	}
	return 0;
}

/*************ftp����������ԭ�ļ���Ϊʱ��.TXT��ɾ��ԭTXT**********/
u8 EC20_FTP_REname(u8 *website,u8 * yuanpathname,char *File)
{

	 
	  memset(AtStrBuf,0,BUFLEN); //������������ 
    sprintf(AtStrBuf,"AT+QFTPRENAME=\"111.txt\",\"%s\"\r\n",File);
	  if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPRENAME: 0,0",1000))             return 1;
	  //if(EC20_send_cmd((u32 *)"AT+QFTPRENAME=\"TD.DAT\",\"123.DAT\"\r\n",(u8 *)"+QFTPRENAME: 0,0",1000))	    return 3;   
	  printf("�ļ������ɹ�  [OK]\r\n");
	  
	  //if(EC20_send_cmd((u32 *)"AT+QFTPCWD=\"/TD.DAT\"\r\n",(u8 *)"+QFTPCWD: 0,0",100))             return 1;
	  if(EC20_send_cmd((u32 *)"AT+QFTPRMDIR=\"111.txt\"\r\n",(u8 *)"OK",1000))   return 2;
	  printf("�ļ�ɾ���ɹ�  [OK]\r\n");
    return 0;
}
/**************�ϴ��ļ���FTP������***********/
u8 EC20_FTP_SEND_DATA(u8 * pathname,u8 *note,char *DATA)
{
	  
    memset(AtStrBuf,0,BUFLEN); //������������ TD.DAT
    sprintf(AtStrBuf,"AT+QFTPCWD=\"%s\"\r\n",pathname);
	  if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPCWD: 0,0",100))             return 1;
	  memset(AtStrBuf,0,BUFLEN); //������������
	  if(EC20_send_cmd((u32 *)"AT+QFTPSTAT\r\n",(u8 *)"+QFTPSTAT: 0,1",100))	 return 2;   //����PDP context 1, APN is�UNINET  for China
	  memset(AtStrBuf,0,BUFLEN); //������������
	  sprintf(AtStrBuf,"AT+QFTPPUT=\"%s\",\"COM:\",0\r\n",note);//
    if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"CONNECT",400))   return 3;
	 
  	 memset(AtStrBuf,0,BUFLEN); //������������
	   u2_printf("%s",DATA);
//   	 u2_printf("%s",DATA);
//	   u2_printf("%s",DATA);
//		printf("\r\n%s\r\n",USART2_TX_BUF);
 	   delay_ms(300);
	   u2_printf("+++");
  	 delay_ms(200);

		if(USART2_RX_STA)
		{
		if(strstr((const char*)USART2_RX_BUF,(const char*)"OK"))
			{
					//printf("%s",USART2_RX_BUF);
					memset(USART2_RX_BUF,0x0,sizeof(USART2_RX_BUF));
					USART2_RX_STA=0;					
				}
		}
    printf("�û����ݷ��ͳɹ� [OK]\r\n");
    return 0;
}


//  transfer shuju content
//u8 EC20_FTP_SEND_DATA_ADvalue(u8 * pathname,u8 *note)
//{
//	 
//    memset(AtStrBuf,0,BUFLEN); //������������ TD.DAT
//    sprintf(AtStrBuf,"AT+QFTPCWD=\"%s\"\r\n",pathname);
//	  if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPCWD: 0,0",100))             return 1;
//	  memset(AtStrBuf,0,BUFLEN); 
//	  if(EC20_send_cmd((u32 *)"AT+QFTPSTAT\r\n",(u8 *)"+QFTPSTAT: 0,1",100))	 return 2;   //����PDP context 1, APN is�UNINET  for China
//	  memset(AtStrBuf,0,BUFLEN); 
//	  
//	  sprintf(AtStrBuf,"AT+QFTPPUT=\"%s\",\"COM:\",0\r\n",note);//
//    if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"CONNECT",400))   return 3;
//	
////	 
//	  STMFLASH_Read(FLASH_SAVE_ADDR_V,(u8 *)VTVALStr,18);
//	  
////    memset(AtStrBuf,0,BUFLEN); 
//	  u2_printf("%s",VTVALStr);
//	
//		
//	  memset(shuju1,0,DATASIZE); // clear
//	  STMFLASH_Read(FLASH_SAVE_ADDR,(u8 *)shuju1,20000);
////	
//    shuju1[20000]='\0';
////		printf("\r\n%s\r\n",shuju1);
////        delay_ms(1500);
//   u2_printf("%s",shuju1);
////	 	 
//	
//	
//	memset(shuju1,0,DATASIZE); // clear
//	STMFLASH_Read(FLASH_SAVE_ADDR1+0X3,(u8 *)shuju1,20000);

//	shuju1[20000]='\0';
//////		printf("\r\n%s\r\n",shuju1);
////      delay_ms(1500);
//	   u2_printf("%s",shuju1);
////	   
//	 

//	memset(shuju1,0,DATASIZE); // clear
//	STMFLASH_Read(FLASH_SAVE_ADDR2,(u8 *)shuju1,20000);
////			delay_ms(100);
//	shuju1[20000]='\0';
////		printf("\r\n%s\r\n",shuju1);
////     delay_ms(1500);
//	u2_printf("%s",shuju1);
//		
////		 
////	memset(shuju1,0,DATASIZE); // clear
////	STMFLASH_Read(FLASH_SAVE_ADDR+0XAFC8,(u8 *)shuju1,15000);
//////			delay_ms(100);
////	shuju1[15000]='\0';
//////		printf("\r\n%s\r\n",shuju1);
//////     delay_ms(1500);
////	u2_printf("%s",shuju1);
//	
//	
//		 
////		printf("\r\n%s\r\n",USART2_TX_BUF);
// 	   delay_ms(1100);
//	   u2_printf("+++");
//  	 delay_ms(1000);
//		 

//		if(USART2_RX_STA&0X8000)
//		{
//		if(strstr((const char*)USART2_RX_BUF,(const char*)"OK"))
//			{
//					//printf("%s",USART2_RX_BUF);
//					memset(USART2_RX_BUF,0x0,sizeof(USART2_RX_BUF));
//					USART2_RX_STA=0;					
//				}
//		}
//    printf("�û����ݷ��ͳɹ� [OK]\r\n");
//		
//		
//		memset(shuju1,0,DATASIZE); // clear
//    STMFLASH_Read(FLASH_SAVE_ADDR,(u8 *)shuju1,60000);
//	
//    return 0;
//}






int16_t getVTh(u8 *pathname)
{
	u8 errcnt=0;
	char *VThstr=0;
	
	memset(AtStrBuf,0,BUFLEN);		//�����
	
	sprintf(AtStrBuf,"AT+QFTPNLST=\"%s/CONF\"\r\n",pathname);
	
	//printf("AtStrBuf: %s\r\n",AtStrBuf);
	while(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPNLST: 0,",200))
	{
		errcnt++;
		if(errcnt>5)
		{
			printf("������ֵ��ȡʧ��\r\n");
			//break;
			return -1;
		}
	}
	
	if((VThstr=strstr((const char*)USART2_RX_BUF,(const char*)"/V="))!=NULL)
	{
		//print(VThstr,12,4,Vth);
		//printf("VThstr: %s\r\n",VThstr);
		return atoi(VThstr+3);
	}
	return -2;


}

//��ʼ��FTP����������
void tryInitFTPConn(u8 times)
{
	u8 initFTPConnErrCt=0;		//initial FTP connect error count
	while(1)
	{
    IWDG_Feed();	//ι��
		printf("Initial_��������ftp��������%d��\r\n",initFTPConnErrCt+1);
		initFtpConnErrFlg=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)usernameMacro,(u8 *)passwordMacro,(u8 *)websiteMacro);
		initFTPConnErrCt++;
		if(initFtpConnErrFlg==0)
		{
			printf("Initial_FTP���������ӳɹ�\r\n");
			NETSTA=1;
			
		  HAL_Delay(3000);
			break;
		}
		
		if(initFTPConnErrCt>times)
		{
			printf("Initial_FTP����������ʧ��\r\n");
			NETSTA=0;
			break;
		}
	}
}




void tryInit4GConn(u8 times)
{
	u8 init4GConnErrCt=0;	//initial 4G connect error count
	while(1)
	{
		IWDG_Feed();			//ι��
		printf("Initial_��������4Gģ�飺%d��\r\n",init4GConnErrCt+1);
		init4GConnErrFlg=EC20_INIT();
		init4GConnErrCt++;
		
		if(init4GConnErrFlg==0)
		{
			printf("Initial_4Gģ�����ӳɹ�\r\n");
			tryInitFTPConn(8);
			break;
		}
		
		if(init4GConnErrCt>times)
		{
			printf("Initial_4Gģ������ʧ��\r\n");
			NETSTA=0;
			break;
		}
		
		
	}
}


/**************��ȡ����ʱ��***********/
int8_t GetNetCLK(void)
{
	char *CLKstr=0;
	u8 errcont=0;
	while(EC20_send_cmd((u32 *)"AT+QLTS=2",(u8 *)"OK",300))
	{
		
//		printf("GetNetCLK ERR\r\n");
		errcont++;
		if(errcont>3)
		{
			printf("����ʱ���ȡʧ��\r\n");
			//break;
			return -1;
		}
	}
	//delay_ms(300);

	if((CLKstr=strstr((const char*)USART2_RX_BUF,(const char*)"+QLTS:"))!=NULL)
	{	   
//		  USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����	
//		
//      CLKstr=strstr((const char*)USART2_RX_BUF,(const char*)"+QLTS:");
//		  printf("4G����ʱ��:%s\r\n",CLKstr);
		//CLKstr="+QLTS: \"2018/03/14,08:08:35+32,0\"";
		local_time.year = atoi(CLKstr+10);        //����CLKstrָ��+QLTS: "2018/03/14,08:08:35+32,0"
		local_time.month = atoi(CLKstr+13);
		local_time.day = atoi(CLKstr+16);
		local_time.hour = atoi(CLKstr+19);
		local_time.minute= atoi(CLKstr+22);
		local_time.second = atoi(CLKstr+25);
		
		printf("����ʱ��: 20%02d��%d��%d�� %d:%02d:%02d\r\n",local_time.year,local_time.month,local_time.day,local_time.hour,local_time.minute,local_time.second);
		
		errcont=0;
//		memset(AtStrBuf,0,BUFLEN); //������������ TD.DAT
//    sprintf(AtStrBuf,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+32\"\r\n",local_time.year,local_time.month,local_time.day,local_time.hour,local_time.minute,local_time.second);
		//printf("AtStrBuf:%s\r\n",AtStrBuf);
//		while(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"OK",300))		//ͬ������ʱ������أ�AT+CCLK="22/10/02,16:53:56+32"
//		{
//			errcont++;
//			if(errcont>3)
//			{
//				printf("����ʱ��ͬ��ʧ��\r\n");
//				break;
//				//return -1;
//			}
//		}
	}
	return 0  ;
}


u8 EC20_FTP_SEND_DATA_ADvalue(u8 * pathname,u8 *note)
{
	 
    memset(AtStrBuf,0,BUFLEN); //������������ TD.DAT
    sprintf(AtStrBuf,"AT+QFTPCWD=\"%s\"\r\n",pathname);
	  if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"+QFTPCWD: 0,0",100))             return 1;
	  memset(AtStrBuf,0,BUFLEN); 
	  if(EC20_send_cmd((u32 *)"AT+QFTPSTAT\r\n",(u8 *)"+QFTPSTAT: 0,1",100))	 return 2;   //����PDP context 1, APN is�UNINET  for China
	  memset(AtStrBuf,0,BUFLEN); 
	  
	  sprintf(AtStrBuf,"AT+QFTPPUT=\"%s\",\"COM:\",0\r\n",note);//
	
    if(EC20_send_cmd((u32 *)AtStrBuf,(u8 *)"CONNECT",400))   return 3;
	
 
     STMFLASH_Read(FLASH_SAVE_ADDR1,(u32 *)VTVALStr,32);	  
     
//	   memset(AtStrBuf,0,BUFLEN); 
	   u2_printf("%s",VTVALStr);
//	   printf("��ֵ %s",VTVALStr);
//	   u2_printf("123456789123456789123456789123456789123456789");
	
		
	     memset(shuju1,0,ADCSIZE); // clear
	     STMFLASH_Read(FLASH_SAVE_ADDR2,(u32 *)shuju1,5000);
//	
       shuju1[20000]='\0';
//		printf("\r\n%s\r\n",shuju1);
//        delay_ms(1500);
//      printf("%s",shuju1);
       u2_printf("%s",shuju1);
////	 	 
//	
//	
	   memset(shuju1,0,ADCSIZE); // clear
	  STMFLASH_Read(FLASH_SAVE_ADDR3,(u32 *)shuju1,5000);

	  shuju1[20000]='\0';
//////		printf("\r\n%s\r\n",shuju1);
//      delay_ms(1500);
	   u2_printf("%s",shuju1);
//////	   
//	 

	memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR4,(u32 *)shuju1,5000);
////			delay_ms(100);
	shuju1[20000]='\0';
////		printf("\r\n%s\r\n",shuju1);
////     delay_ms(1500);
	u2_printf("%s",shuju1);
//		

	
	memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR5,(u32 *)shuju1,5000);
////			delay_ms(100);
	shuju1[20000]='\0';
////		printf("\r\n%s\r\n",shuju1);
////     delay_ms(1500);
	u2_printf("%s",shuju1);
	
	
		memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR6,(u32 *)shuju1,2500);
////			delay_ms(100);
	shuju1[10000]='\0';
////		printf("\r\n%s\r\n",shuju1);
////     delay_ms(1500);
	u2_printf("%s",shuju1);
//	
//	
		memset(shuju1,0,ADCSIZE); // clear
	 STMFLASH_Read(FLASH_SAVE_ADDR9,(u32 *)shuju1,5000);
//////			delay_ms(100);
	shuju1[20000]='\0';

	u2_printf("%s",shuju1);
	
//		 
	memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR10,(u32 *)shuju1,5000);
	shuju1[20000]='\0';
	u2_printf("%s",shuju1);

  memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR11,(u32 *)shuju1,5000);
	shuju1[20000]='\0';
	u2_printf("%s",shuju1);


  memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR12,(u32 *)shuju1,5000);
	shuju1[20000]='\0';
	u2_printf("%s",shuju1);
	
	  memset(shuju1,0,ADCSIZE); // clear
	STMFLASH_Read(FLASH_SAVE_ADDR13,(u32 *)shuju1,2500);
	shuju1[10000]='\0';
	u2_printf("%s",shuju1);
	
	
		 
//		printf("\r\n%s\r\n",USART2_TX_BUF);
 	   delay_ms(1100);
	   u2_printf("+++");
  	 delay_ms(1000);
		 

		if(USART2_RX_STA)
		{
		if(strstr((const char*)USART2_RX_BUF,(const char*)"OK"))
			{
					//printf("%s",USART2_RX_BUF);
					memset(USART2_RX_BUF,0x0,sizeof(USART2_RX_BUF));
					memset(RxBuffer,0,sizeof(RxBuffer));
					USART2_RX_STA=0;					
				}
		}
    printf("�û����ݷ��ͳɹ� [OK]\r\n");

//		
//		memset(shuju1,0,DATASIZE); // clear
//    STMFLASH_Read(FLASH_SAVE_ADDR,(u8 *)shuju1,60000);
	
    return 0;
}



/********
	����tryDatSnd()��tryDatFTPConn()��tryDat4GConn()Ϊ�����׵�����
*******/
void tryDatSnd(u8 times)
{
	 
	//�ȳ��Է���report����
	u8 datSndErrCt=0;	//date send error count
	while(1)
	{
		IWDG_Feed();			//ι��
		printf("date_���ݳ��Է��ͣ�%d��\r\n",datSndErrCt+1);
		datSndErrFlg=EC20_FTP_SEND_DATA_ADvalue((u8 *)pathnameMacro,(u8 *)DATE1);
	
		
		
		
//		STMFLASH_Read(FLASH_SAVE_ADDR_V,(u8 *)VTVALStr,16);
//		printf("%s",VTVALStr);
//		datSndErrFlg=EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE1,(char *)VTVALStr);
//		
	
//		
//		datSndErrFlg=EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE1,(char *)shuju1);

//		STMFLASH_Read(FLASH_SAVE_ADDR1,(u8 *)shuju1,20000);
//		shuju1[20000]='\0';
////		printf("\r\n%s\r\n",shuju1);
//		datSndErrFlg=EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE1,(char *)shuju1);
		

//		STMFLASH_Read(FLASH_SAVE_ADDR2,(u8 *)shuju1,20000);
//		shuju1[20000]='\0';
//		datSndErrFlg=EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE1,(char *)shuju1);
//		datSndErrFlg=EC20_FTP_SEND_DATA((u8 *)pathnameMacro,(u8 *)DATE1,(char *)shuju1);
//		printf("\r\n%s\r\n",shuju1);
		datSndErrCt++;
	
		if(datSndErrFlg==0)
		{
			printf("date_���ݷ��ͳɹ�\r\n");
//			delay_ms(5000);
			break;
		}
		
		if(datSndErrCt>times)
		{
			printf("date_���ݷ���ʧ��r\n");
			break;
		}
		
	}
}


void tryDatFTPConn(u8 times)
{
	//report���ݷ���ʧ�ܣ��ٳ�������FTP������
	u8 datFTPConnErrCt=0;		//FTP connect error count
	while(datSndErrFlg)
	{
		IWDG_Feed();		//ι��
		printf("date_��������ftp��������%d��\r\n",datFTPConnErrCt+1);
		datFTPConnErrFlg=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)usernameMacro,(u8 *)passwordMacro,(u8 *)websiteMacro);
		datFTPConnErrCt++;
		if(datFTPConnErrFlg==0)
		{
			printf("date_FTP���������ӳɹ�\r\n");
			printf("date_���ͱ�������\r\n");
			//����FTP�ɹ����Է���report����
			tryDatSnd(5);
			break;
		}
		
		if(datFTPConnErrCt>times)
		{
			printf("date_FTP����������ʧ��\r\n");
			break;
		}
	}
}

void tryDat4GConn(u8 times)
{
	//FTP���ӳ������ٳ�������4Gģ��
	u8 dat4GConnErrCt=0;	//4G connect error count
	while(datFTPConnErrFlg)
	{
		IWDG_Feed();			//ι��
		dat4GConnErrFlg=EC20_INIT();
		dat4GConnErrCt++;
		
		if(dat4GConnErrFlg==0)
		{
			printf("date_4Gģ�����ӳɹ�\r\n");
			tryDatFTPConn(10);
			break;
		}
		
		if(dat4GConnErrCt>times)
		{
			printf("date_4Gģ������ʧ��\r\n");
			break;
		}
	}
}


