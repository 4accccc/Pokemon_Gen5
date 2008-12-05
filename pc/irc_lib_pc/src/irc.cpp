#ifdef NITRO
#include <nitro.h>
#include "irc.h"
#else
#include "../include/system.h"
#include "../include/irc_internal.h"
#include "../include/com.h"
#endif

#ifdef DHC_DEBUG  
#include "draw.h"
#endif

#define ACCESS(variable, value, mark) { variable = value; }
/***********************************************************************
***********************************************************************/
#define NUM_DS_RETRY	(5) // DS のリトライ試行回数
/***********************************************************************
***********************************************************************/
static u32 cnt_retry;
static u8 myUnitNumber = 0xFF;
static u8 unit_number = 0xFF;
static u8 retry_count = 0;
static BOOL flg_connect = FALSE;
static BOOL flg_restore = FALSE; // エラー復帰中かどうか
static BOOL isTry = FALSE;		// 親子が決まっているかどうか
static BOOL isSender = FALSE;	// センダかどうか
static u32 sConnectionTimeout; // 接続中の通常タイムアウト
#ifdef NITRO
static OSTick timeout;
#else
DWORD timeout;
#endif
static u8 buf[sizeof(Header)+IRC_TRNS_MAX];
static IRCRecvCallback sRecvCallback;
/***********************************************************************
***********************************************************************/
static void _update_tick(void)
{
#ifdef NITRO
  timeout = OS_GetTick();
#else
  timeout = GetTickCount();
#endif    
}
/***********************************************************************
***********************************************************************/
static void _restore(void)
{
  flg_restore = TRUE;
  cnt_retry++; // IRC のカウンタアップ
}

/***********************************************************************
共通初期化処理(シャットダウン時にしても問題ないもののみ)
***********************************************************************/
static void _init(void)
{
  retry_count = 0;
  flg_connect = FALSE;
  flg_restore = FALSE;

  ACCESS(isTry, FALSE, 0); //isTry = FALSE;
  isSender = FALSE;
  
  cnt_retry = 0;
#ifdef DHC_DEBUG  
  OS_Printf("shutdown and retry_count cleared\n");
#endif
}
/***********************************************************************
***********************************************************************/
extern void IRC_SetDisconnectTime(u32 time)
{
  sConnectionTimeout = time;
}
/***********************************************************************
初期化処理
***********************************************************************/
extern void IRC_Init(void)
{
  dhc_init();

  dhc_read(buf); // IRCに残っているデータをクリアする目的
  _init();
  myUnitNumber = 0xFF;
  unit_number = 0xFF;
  sRecvCallback = NULL;
  sConnectionTimeout = 100;
}
/***********************************************************************
接続要求。こちらをした方(実際にはこの要求を受けた方の逆) がセンダになります。
***********************************************************************/
extern void IRC_Connect(void)
{
  if(flg_connect){ // 接続中
	PRINTF("CANCEL:IRC_Connect already connected\n");
	return; 
  }
  if(isTry){ 
#ifdef PC_DEBUG
	PRINTF("CANCEL:IRC_Connect already trying\n");
#endif
	return; 
  } 
  ACCESS(isTry, TRUE, 1);//isTry = TRUE;
  PRINTF("CALL:IRC_Connect\n");
  dhci_command(0xFC); // 接続要求
  _update_tick(); // 2008/09/19 追加
}
/***********************************************************************
センダかどうかの確認
***********************************************************************/
extern BOOL IRC_IsSender(void)
{
  return (isSender);
}
/***********************************************************************
接続されているかどうかの確認
***********************************************************************/
extern BOOL IRC_IsConnect(void)
{
  return (flg_connect);
}
/***********************************************************************
切断処理
***********************************************************************/
extern void IRC_Shutdown(void)
{
  if(flg_connect && (!isSender)){ dhci_write(NULL, 0, 0xF4, 0x00); } // 切断コマンド送信。コマンドのみ送る場合はサイズを0にすること！

  _init();
}
/***********************************************************************
データ送信
***********************************************************************/
extern void IRC_Send(u8 *buf, u8 size, u8 command, u8 value)
{
  if(!flg_connect){ return; }
  dhc_write(buf, size, command, value);
}
/***********************************************************************
リトライ時のタイムアウト判定 OS_TicksToMilliSeconds(OS_GetTick() - timeout) > 50
***********************************************************************/
static void check_timeout(void)
{
  DWORD nowTime;

  if(flg_restore && (retry_count < NUM_DS_RETRY) && (isTry == FALSE)){
	if((GetTickCount() - timeout) > 50){
#ifdef PC_DEBUG  
	  if(isSender){
		PRINTF("再接続要求送信(CONNECT) TO IRC %d/%d\n", retry_count, NUM_DS_RETRY);
	  }else{
		PRINTF("再接続要求送信(RETRY) TO DHC %d/%d\n", retry_count, NUM_DS_RETRY);
	  }
#endif
	  if(isSender){
		dhci_command(0xFC); retry_count++; // 接続要求コマンド
	  }else{
		dhci_write(NULL, 0, 0xF6, retry_count++); // 0xF6 はリトライコマンド コマンドのみ送る場合はサイズを0にすること！
	  }
	  _update_tick();
	}
  }
  /*********************************************************************
  OS_TicksToMilliSeconds(OS_GetTick() - timeout) > 100
  *********************************************************************/
  if(sConnectionTimeout == 0){ return; }
  nowTime = GetTickCount();
  if((nowTime - timeout) > sConnectionTimeout){
	if(flg_restore && (retry_count < NUM_DS_RETRY)){
#ifdef DHC_DEBUG  
	  OS_Printf("RESTORE TIMEOUT %d msec\n", (u32)OS_TicksToMilliSeconds(OS_GetTick() - timeout));
#endif
#ifdef PC_DEBUG  
	  PRINTF("リトライタイムアウト CALL:IRC_Shutdown\n");
#endif
	  IRC_Shutdown();
	}else{
	  if(IRC_IsConnect()){ 
#ifdef PC_DEBUG  
		PRINTF("接続中のタイムアウト CALL:IRC_Shutdown\n");
#endif		
		IRC_Shutdown(); 
	  }
	  ACCESS(isTry, FALSE, 2);//isTry = FALSE; // 接続トライ中フラグをクリア
	}
  }
}
/***********************************************************************
プロセス処理関数
***********************************************************************/
extern void IRC_Move(void)
{
  u32 size;

#ifdef DHC_DEBUG  
  static u8 count = 0;
  SetTextColor(RGB555_GREEN);
#endif
  dhc_setreadwait(50); // 40 OK? 30 NG
  dhc_setwritewait(50); // 40 OK? 30 NG
  /*********************************************************************
  *********************************************************************/
  size = (u32)dhc_read(buf); // 暗号解除済み
  
#ifdef DHC_DEBUG  
  if(size){ // 生データの先頭を少し見る
	u32 i, s;
	s = size; if(s > 8){ s = 8; }
	FillRect(0, 8, 8*9+8*3*8, 8, RGB555_BLACK);
	DrawText(0, 8, "size %2d:", size);
	for(i=0;i<s;i++){ DrawText((int)(8*9+8*3*i), 8, "%02X", buf[i]); }
  }
#endif
  if(size > 1){ // 1バイトのネゴシエーションコマンドはチェックしない
	u16 checksum, mysum;
	Header *pHeader;
	
	pHeader = (Header*)buf;
	checksum = (u16)(pHeader->sumL + (pHeader->sumH << 8)); 
	pHeader->sumL = 0; pHeader->sumH = 0; 
	mysum = _calcsum(buf, size);
	pHeader->sumL = (u8)(checksum&0xFF); pHeader->sumH = (u8)(checksum>>8); // 戻しておく
	if(checksum != mysum){ // チェックサムエラー
	  if(flg_connect){  // 復帰処理開始
#ifdef DHC_DEBUG  
		if((pHeader->command & 0xFE) != 0xFE){
		  OS_Printf("チェックサムエラーによる再接続開始 受信コマンド 0x%X RETRY-COUNT %d: size %d 0x%X(COM) -> 0x%X(CALC)\n", pHeader->command, retry_count, size, checksum, mysum); 
		}
#endif
		_restore(); 
	  }
	  size = 0; // データが届かなかったことにする
	  //timeout = OS_GetTick(); // この場合でもタイムアウトは更新(最初のFCを考慮すれば一か所にまとめられるかも)
	}else{ // チェックサム問題なし
#ifdef DHC_DEBUG  
	  if(retry_count){ OS_Printf("retry_count cleared\n"); }
#endif
	  retry_count = 0; 
	}
  }else{ // 1バイトで送られてくるのは最初の0xFCのみ
	if(buf[0] != 0xFC){
	  if(flg_connect){ 
		size = 0; 
		///*if(buf[0] != 0)*/{ OS_Printf("RECV 0x%X\n", buf[0]); }
	  } // ここのリストアを呼ぶとノイズですぐリストアが発生してぐだぐだに
	}
  }
  /*********************************************************************
  チェックサムチェックも含めてデータが届いていた場合
  *********************************************************************/
  if(size){
	_update_tick();
	switch(buf[0]){
	case 0xFC: // 接続コマンド
	  if(isTry){ break; } ACCESS(isTry, TRUE, 3);//isTry = TRUE;
	  isSender = FALSE;
	  if((size > 1) && (flg_connect == 0)){ break; } // 切断中の再接続コマンドは無視
	  dhci_write(NULL, 0, 0xFA, myUnitNumber); // コマンドのみ送る場合はサイズを0にすること！
	  if(flg_connect){ _restore(); } // DHC 側が異常検出して接続中に再接続としてこのコマンドを送ってきた場合
#ifdef PC_DEBUG  
	  if(flg_restore){
		PRINTF("再接続要求受信(リストア状態) 0x%X\n", buf[0]);
	  }else{
		if(flg_connect){
		  PRINTF("新規接続要求(0xFC)受信 -> レスポンス送信(接続中)\n", buf[0]);
		}else{
		  PRINTF("新規接続要求(0xFC)受信 -> レスポンス送信(切断中)\n", buf[0]);
		}
	  }
#if 0
	  if(flg_connect == FALSE){ ClearFrame(RGB555_BLACK); }
	  DrawText(32, 0, ">REQ");
	  DrawText(32+8*5, 0, "RES>");
#endif
#endif
	  break;
	case 0xFA: // 接続要求応答をIRCから受信した(DHC エミュレート用)
	  isSender = TRUE;
	  unit_number = buf[1]; // ユニット番号
#ifdef DHC_DEBUG  
	  OS_Printf("RESPONSE FROM IRC ACCEPT\n");
#endif
	  dhc_write(NULL, 0, 0xF8, myUnitNumber); // COMMAND_ACK 相当の処理
#ifdef DHC_DEBUG  
	  DrawText(8*4*2, 0, ">RES ACK>");
#endif
	  flg_connect = TRUE; flg_restore = FALSE; ACCESS(isTry, FALSE, 4);//isTry = FALSE;
	  break;
	case 0xF8: // 接続要求側からのACK(これが届いて接続完了)
#ifdef PC_DEBUG  
	  if(flg_restore){
		PRINTF("ACK 受信 -> 再接続完了 SIZE %d\n", size);
	  }else{
		PRINTF("ACK 受信 -> 新規接続完了 SIZE %d\n", size);
	  }
#endif
	  unit_number = buf[1]; // ユニット番号
	  if(flg_restore){ 
#ifdef DHC_DEBUG  
		u8 *p = dhc_getretrydata();
		OS_Printf("データ再送開始 コマンド 0x%X value 0x%X SUM 0x%X%X\n", *(p+1), *(p+2), *(p+3), *(p+4)); // 最初の1バイトはデバイス選択
		//OS_Printf("RETRY COMMAND 0x%d value %d %s\n", sBackBuf[0], sBackBuf[1], &sBackBuf[4]);
#endif
		dhc_retry(); 
	  } // 前回のデータをそのまま送る
	  flg_connect = TRUE; flg_restore = FALSE; ACCESS(isTry, FALSE, 5);//isTry = FALSE;
	  break;
	case 0xF6: // COMMAND_RETRY(DHC エミュレート用)
	  flg_restore = TRUE;
	  break;
#if 0
	case 0: 
	  OS_Printf("command 0x%X: value 0x%X, SUM 0x%X, 0x%X size %d 0x%X, 0x%X\n", buf[0], buf[1], buf[2], buf[3], size, buf[4], buf[5]);
	  break; // 使用禁止
#endif
	case 0xFE: break; // ノイズ対策
	case 0xFF: break; // ノイズ対策
	default:
	  if(flg_restore == TRUE){ break; } // 復帰中に届いた通常コマンドは無視する
	  if(flg_connect == FALSE){ break; }
	  if(sRecvCallback != NULL){ sRecvCallback(&buf[4], (u8)size, buf[0], buf[1]); }
	  break;
	}
  }else{ // 何もデータが届かなかった(0バイト)の場合
	check_timeout();
  }
}
/***********************************************************************
データ受信コールバックの設定
***********************************************************************/
extern void IRC_SetRecvCallback(IRCRecvCallback cb)
{
  sRecvCallback = cb;
}
/***********************************************************************
トータルのリトライ回数を取得する
***********************************************************************/
extern u32 IRC_GetRetryCount(void)
{
  return (cnt_retry);
}
/***********************************************************************
自身のユニット番号を設定する
***********************************************************************/
extern void IRC_SetUnitNumber(u8 id)
{
  myUnitNumber = id;
}
/***********************************************************************
通信相手のユニット番号を取得する
***********************************************************************/
extern u8 IRC_GetUnitNumber(void)
{
  return (unit_number);
}
/***********************************************************************
USB のチェック(PC専用)
***********************************************************************/
#ifndef NITRO
extern BOOL IRC_IsConnectUsb(void)
{
  return ComIsConnect();
}
extern TCHAR *IRC_GetPortName(void)
{
  return ComGetPort();
}
#endif
