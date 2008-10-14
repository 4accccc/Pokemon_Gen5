#ifndef __IRC_H__
#define __IRC_H__

/***********************************************************************
***********************************************************************/
#define DHC_DEBUG	(1) // 非デバッグ時は定義を消しておいてください
/***********************************************************************
***********************************************************************/
#define IRC_TRNS_MAX		(128)	// 一度に送れる転送バイト数

#define IRC_UNIT_DHC		(0x00)	// DHC
#define IRC_UNIT_IRC		(0x01)	// 赤外線通信カード
#define IRC_UNIT_UNKNOWN	(0xFF)	// 機器不明

/***********************************************************************
コールバック関数の型定義
***********************************************************************/
typedef void (*IRCRecvCallback)(unsigned char *data, unsigned char size, unsigned char command, unsigned char value);
/***********************************************************************
***********************************************************************/
extern void IRC_Init(void);		// 初期化関数
extern void IRC_Move(void);		// 毎フレーム呼ぶプロセス関数

extern void IRC_Send(unsigned char *buf, unsigned char size, unsigned char command, unsigned char value);	// データ送信
extern void IRC_SetRecvCallback(IRCRecvCallback cb); // データ受信コールバックの設定

extern void IRC_Connect(void);	// 接続要求。こちらをした方(実際にはこの要求を受けた方の逆)がセンダになります。
extern BOOL IRC_IsSender(void); // センダかどうかの確認(接続中のみ有効)

extern BOOL IRC_IsConnect(void); // 接続の確認
extern void IRC_Shutdown(void); // 切断
/***********************************************************************
動作には必須でないもの
***********************************************************************/
extern void IRC_SetUnitNumber(unsigned char id); // 自身のユニット番号を設定する
extern unsigned char IRC_GetUnitNumber(void); // 通信相手のユニット番号の確認(切断時は0xFF)

extern unsigned int IRC_GetRetryCount(void); // トータルのリトライ回数を取得する(デバッグ用)
/***********************************************************************
接続タイムアウト時間の設定(ミリ秒単位)
0を指定するとタイムアウトしなくなります
IRC_Init で100に設定されています
***********************************************************************/
extern void IRC_SetDisconnectTime(unsigned int time);
/***********************************************************************
USB のチェック(PC専用)
***********************************************************************/
#ifndef NITRO
extern BOOL IRC_IsConnectUsb(void);
extern TCHAR *IRC_GetPortName(void);
#endif

#endif
