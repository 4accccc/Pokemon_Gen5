#include "stdafx.h"

#include "system.h"
#include "irc_internal.h"
#include "com.h"
/***********************************************************************
***********************************************************************/
#define COMMAND_READ			(0x0001)	// DHC 識別子を兼ねる
#define COMMAND_WRITE			(0x0002)	// DHC 識別子を兼ねる
/***********************************************************************
***********************************************************************/
#define COM_DATA_SIZE	(128)	// 2007/11/02 ユーザ送受信バッファサイズ
#define COM_TRNS_SIZE	(1+sizeof(Header)+COM_DATA_SIZE)	// システム送受信バッファサイズ
/***********************************************************************
***********************************************************************/
static u8 last_size, backup_size;				// リトライ用
static u8 sSendBuf[COM_TRNS_SIZE]; // リトライ用に受信と分ける
static u8 sRecvBuf[COM_TRNS_SIZE];
static u8 sBackBuf[COM_TRNS_SIZE+1]; // リトライ用 TODO: 1byte はデバグ用
/***********************************************************************
***********************************************************************/
extern u16 _calcsum(void *input, u32 length)
{
  u32 sum, i;
  u8 t;
  u8 *p;

  sum = 0;
  p = (u8*)input;
  for(i = 0; i<length; i++){
	t = *p++;
	if(i&1){ sum += t; }else{ sum += t<<8; }
  }

  sum = (sum >> 16) + (sum & 0xFFFF); sum += (sum >> 16);

  return ((u16)sum);
}
/***********************************************************************
***********************************************************************/
extern void dhc_init(void)
{
  if(ComInit(COM_AUTO) == FALSE){ 
	ComInit(COM_PORT4); // 最後はいらない
  }
}
/***********************************************************************
***********************************************************************/
static void dhc_savedata(void)
{
  int i;

  for(i=0;i<last_size;i++){ sBackBuf[i]= sSendBuf[i]; }
  backup_size = last_size;
}
/***********************************************************************
buf にデータが入る: 戻り値は受信サイズ
***********************************************************************/
extern u8 dhc_read(u8 *buf)
{
  int i;
  u8 size = 0;
#if 0
  static int noDataCount = 0;
  static int noDataMax = 0;
#endif
  {
	u8 v = COMMAND_READ;
	//printf("書き込み開始\n");
	BOOL ret = WriteData(&v, 1);
	if(ret == FALSE){ ComEnd(); dhc_init(); }
  }
  size = ReadData(buf, COM_TRNS_SIZE);
  
  if(size > COM_TRNS_SIZE){ size = 0; }
#if 0
  if(size == 0){ noDataCount++; }else{ noDataMax = noDataCount; noDataCount = 0; }
  PrintString(16*14, SCR_HT+16*0, L"カウント %d", noDataMax);
#endif
  //if(size == 0){ printf("受信サイズ 0\n"); }
  //for(int i=0;i<size;i++){ 読み込む }
  
#if 1 
	for(i=0;i<size;i++){ buf[i] = (u8)(buf[i] ^ 0xAA); } // 暗号
#endif  

  return (size);
}
/***********************************************************************
***********************************************************************/
extern void dhci_write(u8 *buf, u8 size, u8 command, u8 value)
{
  u8 *p;
  u16 sum;
  int i;
  Header *pHeader;
  /*********************************************************************
  データの準備
  *********************************************************************/
  pHeader = (Header*)&sSendBuf[1];
  pHeader->command = command;
  pHeader->value = value;
  pHeader->sumL = 0; pHeader->sumH = 0; 
  p = &sSendBuf[1+sizeof(Header)];

  for(i=0;i<size;i++){ *p++ = *buf++; }
  sum = _calcsum(&sSendBuf[1], sizeof(Header)+size);

#if 0 // 0828
  OS_Printf("command 0x%X value 0x%X checksum 0x%X\n", command, value, sum);
#endif
  pHeader->sumL = (u8)(sum&0xFF); pHeader->sumH = (u8)((sum>>8)&0xFF);
  /*********************************************************************
  *********************************************************************/
  size += 1+sizeof(Header);
  //card_param.src = (u32)sSendBuf; card_param.dst = (u32)NULL;
  sSendBuf[0] = COMMAND_WRITE;
  /*********************************************************************
  *********************************************************************/
#if 1 // 暗号 デバイス選択コマンドは暗号化しない
  for(i=1;i<size;i++){ sSendBuf[i]= (u8)(sSendBuf[i] ^ 0xAA); }
#endif
  last_size = size;
  /*********************************************************************
  *********************************************************************/
  WriteData(sSendBuf, size);
}
/***********************************************************************
***********************************************************************/
extern void dhc_write(u8 *data, u8 size, u8 command, u8 value)
{
  dhci_write(data, size, command, value);
  dhc_savedata();
}
/***********************************************************************
***********************************************************************/
extern void dhc_retry(void)
{
  //int i;
  
  sBackBuf[COM_TRNS_SIZE] = 0;
  /*********************************************************************
  *********************************************************************/
#if 1
  WriteData(sBackBuf, backup_size);
#else
  for(i=0;i<backup_size;i++){
	//CommWrite(&card_param);
  }
#endif
}
/***********************************************************************
空関数でOK
***********************************************************************/
extern void dhc_setreadwait(u8 time)
{

}
/***********************************************************************
空関数でOK
***********************************************************************/
extern void dhc_setwritewait(u8 time)
{

}
/***********************************************************************
***********************************************************************/
extern void dhci_command(u8 command) // 
{
  u8 size;
  int i;
  Header *pHeader;
  /*********************************************************************
  データの準備
  *********************************************************************/
  pHeader = (Header*)&sSendBuf[1];
  pHeader->command = command;
  pHeader->value = 0;
  pHeader->sumL = 0; pHeader->sumH = 0; 
  /*********************************************************************
  *********************************************************************/
  size = 1+1;
  sSendBuf[0] = COMMAND_WRITE; 
  /*********************************************************************
  *********************************************************************/
#if 1 // 暗号 デバイス選択コマンドは暗号化しない
  for(i=1;i<size;i++){ sSendBuf[i]= (u8)(sSendBuf[i] ^ 0xAA); }
#endif  
  WriteData(sSendBuf, size);
}

