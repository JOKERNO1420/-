//#ifndef __FTPGPRSDRIVER__
//#define __FTPGPRSDRIVER__

#ifndef __EC20FTP_H__
#define __EC20FTP_H__
#include "sys.h"
#define SIM_OK 0
#define SIM_COMMUNTION_ERR 0xff
#define SIM_CPIN_ERR 0xfe
#define SIM_CREG_FAIL 0xfd
#define SIM_MAKE_CALL_ERR 0Xfc
#define SIM_ATA_ERR       0xfb

#define SIM_CMGF_ERR 0xfa
#define SIM_CSCS_ERR 0xf9
#define SIM_CSCA_ERR 0xf8
#define SIM_CSMP_ERR 0Xf7
#define SIM_CMGS_ERR       0xf6
#define SIM_CMGS_SEND_FAIL       0xf5

#define SIM_CNMI_ERR 0xf4
#define BUFLEN 150


//新FTP服务器：EC20_CONNECT_SERVER_CFG_INFOR()被动模式=1
#define usernameMacro   "lightning"
#define passwordMacro   "data@2019"
#define websiteMacro    "119.45.172.235"
#define pathnameMacro   "/TD/TD999"//二级目录
#define noteMacro       "111.txt"//二级目录下的TXT文件名
#define FileMacro       "222.DAT"

typedef struct 
{
	s32 year;    
  s32 month;
  s32 day;
  s32 hour;
  s32 minute;
  s32 second;
  //s32 timezone;  
}ST_Time;	//本地时间结构体

extern ST_Time local_time;
extern char AtStrBuf[BUFLEN];



extern u8 Flag_Rec_Message;	//收到短信标示

extern u8 EC20_INIT(void);
extern u8 EC20_CONNECT_SERVER_CFG_INFOR(u8 *username,u8 *password,u8 *website);
extern u8 EC20_FTP_SEND_DATA(u8 * pathname,u8 *note,char *DATA);   
extern u8 EC20_FTP_SEND_DATA_ADvalue(u8 * pathname,u8 *note); 
//extern u8 EC20_MQTT_SEND_DATA3(u8 *PRODUCTKEY,u8 *DEVICENAME,char *DATA);

void check_cmd(void);
u8 EC20_send_cmd(u32 *cmd,u8 *ack,u16 waittime);
u8 EC20_send_cmd2(u32 *cmd,u8 *ack,u16 waittime);
extern u8 EC20_CONNECT_FTP_SERVER(u8 *username,u8 *password,u8 *website);

u8 EC20_FTP_REname(u8 *website,u8 * yuanpathname,char *File);
extern u8 EC20_FTP_LOG(void);
u8 EC20_FTP_Close(void);
u8 EC20_FTP_updata(void);	
int16_t getVTh(u8 * pathname);
//void HMISends(char *buf1);	  //字符串发送函数
//void HMISendStr(char *Buf);
//u8 GetNetCLK();		//获取4G网络时间
void HMISendb(u8 k);		         //字节发送函数
extern int NETSTA;			//网络状态
extern u8 init4GConnErrFlg,initFtpConnErrFlg;			
void tryInit4GConn(u8 times);
void tryInitFTPConn(u8 times);
int8_t GetNetCLK(void);
void tryDat4GConn(u8 times);
void tryDatFTPConn(u8 times);
void tryDatSnd(u8 times);
#endif
