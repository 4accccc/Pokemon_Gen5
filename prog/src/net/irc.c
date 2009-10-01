#include <nitro.h>

#include "net/irc.h"
#ifdef DHC_DEBUG  
//#include "draw.h"
#endif

/***********************************************************************
***********************************************************************/
#define NUM_DS_RETRY	(5) // DS のリトライ試行回数
/***********************************************************************
NitroSDK 内で定義してある関数のextern 宣言
***********************************************************************/
extern void dhc_init(void); // NitroSDK 内に実装
extern u8 dhc_read(u8 *buf); // ユーザは呼ばなくてよいはず
extern void dhci_write(u8 *buf, u8 size, u8 command, u8 value); // ユーザは呼ばなくてよいはず
extern void dhc_write(u8 *buf, u8 size, u8 command, u8 value);
extern void dhc_savedata(void); // ユーザは呼ばなくてよいはず
extern u8* dhc_getretrydata(void); // ユーザは呼ばなくてよいはず
extern void dhc_retry(void); // ユーザは呼ばなくてよいはず
extern void dhc_setreadwait(u8 time); // ユーザは呼ばなくてよいはず
extern void dhc_setwritewait(u8 time); // ユーザは呼ばなくてよいはず
extern void dhci_command(u8 command); // 1バイト送信関数
/***********************************************************************
ライブラリとかぶってるのでなんとかしたい？
***********************************************************************/
typedef struct{
  u8	command;	// コマンド
  u8	value;		// ユーザフリー
  u8	sumL;		// チェックサム上位
  u8	sumH;		// チェックサム下位
}Header;
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
static OSTick timeout;
static u8 buf[sizeof(Header)+IRC_TRNS_MAX];
static IRCRecvCallback sRecvCallback;
/***********************************************************************
***********************************************************************/
static u16 _calcsum(void *input, u32 length)
{
  u32 sum, i;
  u8 t;
  
  sum = 0;

  for(i = 0; i<length; i++){ 
	t = *((u8*)input)++;
	if(i&1){ sum += t; }else{ sum += t<<8; }
  }

  sum = (sum >> 16) + (sum & 0xFFFF); sum += (sum >> 16);

  return ((u16)sum);
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

  isTry = FALSE;
  isSender = FALSE;
  
  cnt_retry = 0;
#ifdef DHC_DEBUG  
  OS_Printf("shutdown and retry_count cleared\n");
#endif
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
}
/***********************************************************************
接続要求。こちらをした方(実際にはこの要求を受けた方の逆) がセンダになります。
***********************************************************************/
extern void IRC_Connect(void)
{
  if(flg_connect){ return; } // 接続中
  if(isTry){ return; } isTry = TRUE;
    
  dhci_command(0xFC); // 接続要求
  timeout = OS_GetTick(); // 2008/09/19 タイムアウト時間更新
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
プロセス処理関数
***********************************************************************/
extern void IRC_Move(void)
{
  u32 size;

#ifdef DHC_DEBUG  
  static u8 count = 0;
//  SetTextColor(RGB555_GREEN);
#endif
  dhc_setreadwait(50); // 40 OK? 30 NG
  dhc_setwritewait(50); // 40 OK? 30 NG
  /*********************************************************************
  *********************************************************************/
  size = (u32)dhc_read(buf);
#ifdef DHC_DEBUG  
//  if(size){ // 生データの先頭を少し見る
//	u32 i, s;
//	s = size; if(s > 8){ s = 8; }
//	FillRect(0, 8, 8*9+8*3*8, 8, RGB555_BLACK);
//	DrawText(0, 8, "size %2d:", size);
//	for(i=0;i<s;i++){ DrawText((int)(8*9+8*3*i), 8, "%02X", buf[i]); }
//  }
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
	timeout = OS_GetTick(); // タイムアウト時間更新
	switch(buf[0]){
	case 0xFC: // 接続コマンド
	  if(isTry){ break; } isTry = TRUE;
	  isSender = FALSE;
	  if((size > 1) && (flg_connect == 0)){ break; } // 切断中の再接続コマンドは無視
	  dhci_write(NULL, 0, 0xFA, myUnitNumber); // コマンドのみ送る場合はサイズを0にすること！
	  if(flg_connect){ _restore(); } // DHC 側が異常検出して接続中に再接続としてこのコマンドを送ってきた場合
#ifdef DHC_DEBUG  
	  if(flg_restore){
		OS_Printf("再接続要求受信\n", buf[0]);
	  }else{
		OS_Printf("新規接続要求受信 -> レスポンス送信\n", buf[0]);
	  }
//	  if(flg_connect == FALSE){ ClearFrame(RGB555_BLACK); }
//	  DrawText(32, 0, ">REQ");
//	  DrawText(32+8*5, 0, "RES>");
#endif
	  break;
	case 0xFA: // 接続要求応答をIRCから受信した(DHC エミュレート用)
	  isSender = TRUE;
	  unit_number = buf[1]; // ユニット番号
	  //OS_Printf("RESPONSE FROM IRC ACCEPT\n");
	  dhc_write(NULL, 0, 0xF8, myUnitNumber); // COMMAND_ACK 相当の処理
#ifdef DHC_DEBUG  
	  OS_Printf("レスポンス受信 -> ACK 送信\n");
	 // DrawText(8*4*2, 0, ">RES ACK>");
#endif
	  flg_connect = TRUE; flg_restore = FALSE; isTry = FALSE;
	  break;
	case 0xF8: // 接続要求側からのACK(これが届いて接続完了)
#ifdef DHC_DEBUG  
	  if(flg_restore){
		OS_Printf("ACK 受信 -> 再接続完了 SIZE %d\n", size);
	  }else{
		OS_Printf("ACK 受信 -> 新規接続完了 SIZE %d\n", size);
	  }
	  //DrawText(32+8*5*2, 0, ">ACK");
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
	  flg_connect = TRUE; flg_restore = FALSE; isTry = FALSE;
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
	  sRecvCallback(&buf[4], (u8)size, buf[0], buf[1]);
	  break;
	}
  }else{ // 何もデータが届かなかった(0バイト)の場合
	if(flg_restore && (retry_count < NUM_DS_RETRY) && isTry == FALSE){
	  if(OS_TicksToMilliSeconds(OS_GetTick() - timeout) > 50){ // 再接続要求をDHCに出す
#ifdef DHC_DEBUG  
		if(isSender){
		  OS_Printf("再接続要求送信(CONNECT) TO IRC %d/%d\n", retry_count, NUM_DS_RETRY);
		}else{
		  OS_Printf("再接続要求送信(RETRY) TO DHC %d/%d\n", retry_count, NUM_DS_RETRY);
		}
#endif
		if(isSender){
		  dhci_command(0xFC); retry_count++; // 接続要求コマンド
		}else{
		  dhci_write(NULL, 0, 0xF6, retry_count++); // 0xF6 はリトライコマンド コマンドのみ送る場合はサイズを0にすること！
		}
		timeout = OS_GetTick();
	  }
	}
	if(OS_TicksToMilliSeconds(OS_GetTick() - timeout) > 100){ // タイムアウト

    if(flg_restore && (retry_count < NUM_DS_RETRY)){
      OS_Printf("タイムアウト!!%d\n",retry_count);
      IRC_Shutdown();
	  }
    else{
      if(IRC_IsConnect()){ 
        IRC_Shutdown(); 
      }
      isTry = FALSE; // 接続トライ中フラグをクリア
	  }
	}
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
