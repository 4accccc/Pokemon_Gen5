#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	net.h
 * @brief	通信ライブラリの外部公開関数
 * @author	k.ohno
 * @date    2006.11.4
 */
//=============================================================================
#pragma once

#include "heapsys.h"


#define GFL_NET_WIFI    (1)   ///< WIFIをゲームで使用する場合 ON
#define GFL_NET_IRC     (1)   ///< IRCをゲームで使用する場合 ON

#if GFL_NET_WIFI
//#include <dwc.h>
#endif  //GFL_NET_WIFI



// デバッグ用決まり文句----------------------

#ifdef PM_DEBUG

extern BOOL GFL_NET_DebugGetPrintOn(void);
#define GFL_NET_DEBUG   (1)   ///< ユーザーインターフェイスデバッグ用 0:無効 1:有効
#define GFL_LOBBY_DEBUG	(1)		///<Wi-Fi広場デバッグ用 0:無効 1:有効

#else //PM_DEBUG

#define GFL_NET_DEBUG   (0)   ///< ユーザーインターフェイスデバッグ用 0:無効 1:有効
#define GFL_LOBBY_DEBUG	(0)		///<Wi-Fi広場デバッグ用 0:無効 1:有効

#endif  //PM_DEBUG



#ifndef NET_PRINT
#if GFL_NET_DEBUG

#define NET_PRINT(...) \
  if(GFL_NET_DebugGetPrintOn())  (void) ((OS_TPrintf(__VA_ARGS__)))

#else   //GFL_NET_DEBUG

#define NET_PRINT(...)           ((void) 0)

#endif  // GFL_NET_DEBUG
#endif  //GFL_NET_PRINT
// デバッグ用決まり文句----------------------
// データダンプ
#ifdef GFL_NET_DEBUG
extern void GFL_NET_SystemDump_Debug(u8* adr, int length, char* pInfoStr);
#define DEBUG_DUMP(a,l,s)  if(GFL_NET_DebugGetPrintOn()) GFL_NET_SystemDump_Debug(a,l,s)
#else
#define DEBUG_DUMP(a,l,s)       ((void) 0)
#endif

// デバッグ用決まり文句----------------------
#define GFL_IRC_DEBUG   (0)   ///< ユーザーインターフェイスデバッグ用 0:無効 1:有効

#ifndef IRC_PRINT
#if GFL_IRC_DEBUG
#define IRC_PRINT(...) \
  if(GFL_NET_DebugGetPrintOn()) (void) ((OS_TPrintf(__VA_ARGS__)))
#else   //GFL_IRC_DEBUG
#define IRC_PRINT(...)           ((void) 0)
#endif  // GFL_IRC_DEBUG
#endif

#ifndef LOBBY_PRINT
#if GFL_LOBBY_DEBUG
#define LOBBY_PRINT(...) \
  if(GFL_NET_DebugGetPrintOn()) (void) ((OS_TPrintf(__VA_ARGS__)))
#else   //GFL_LOBBY_DEBUG
#define LOBBY_PRINT(...)           ((void) 0)
#endif  // GFL_LOBBY_DEBUG
#endif

#ifdef PM_DEBUG
#define COMMST_DEBUG_WFLBY_START	//デバッグ：広場入室部屋任意に指定可
#endif


/// @brief ネットワーク単体のハンドル
typedef struct _GFL_NETHANDLE GFL_NETHANDLE;

///< @brief 通信管理構造体
typedef struct _GFL_NETSYS GFL_NETSYS;

// define
#define GFL_NET_NETID_SERVER (0xff)    ///< NetID:サーバーの場合これ 後は0からClientID
#define GFL_NET_NETID_SERVERBIT (0x80)    ///< サーバーBIT
#define GFL_NET_SENDID_ALLUSER (0xff)  ///< NetID:全員へ送信する場合
#define GFL_NET_NETID_INVALID  (0xfe)  ///< ありえないID
//#define GFL_NET_PARENT_NETID    (0)    ///< 親のID
#define GFL_NET_NO_PARENTMACHINE (0)  ///< 親機の番号


#define GFL_NET_CHILD_MAX  ( 7 )    ///<  子機最大数 15->7に
#define GFL_NET_MACHINE_MAX  (GFL_NET_CHILD_MAX+1)  ///< 機最大数
#define GFL_NET_HANDLE_MAX  (GFL_NET_MACHINE_MAX+1) ///<   通信ハンドル最大数  子機全部＋親機 分



#define GFL_NET_TOOL_INVALID_LIST_NO  (-1) ///<無効な選択ID

#define GFL_WICON_POSX (240)   ///< ワイヤレスアイコンのデフォルトＸＹ位置
#define GFL_WICON_POSY (0)

// 無線で使用するDMA番号   dma.hに移動しました 通信は2になります
///#define _NETWORK_DMA_NO                 (1)


//WMのパワーモード 必ず1を指定する
#define _NETWORK_POWERMODE       (1)
//SSL処理のスレッド優先順位
#define _NETWORK_SSL_PRIORITY     (20)



// 親機を選択できる数。
#define  SCAN_PARENT_COUNT_MAX ( 16 )


///赤外線通信での一度に遅れる最大バイト数
#define GFL_NET_IRC_SEND_MAX		(128)	//132byte中の残り4バイトはIRCライブラリのヘッダが使用


/// @brief  コマンド関連の定義

/// コールバック関数の書式
typedef void (*GFL_NET_PTRCommRecvFunc)(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
/// サイズが固定の場合サイズを関数で返す
//typedef int (*PTRCommRecvSizeFunc)(void);
/// 受信バッファを外部で持っている場合そのポインタ
typedef u8* (*GFL_NET_PTRCommRecvBuffAddr)(int netID, void* pWork, int size);
/// 送信した場合に呼ばれるコールバック
typedef u8* (*GFL_NET_SendCallbackType)(int netID, int command, void* pWork);




/// コマンドパケットテーブル定義
typedef struct {
  GFL_NET_PTRCommRecvFunc callbackFunc;    ///< コマンドがきた時に呼ばれるコールバック関数
  //    PTRCommRecvSizeFunc getSizeFunc; ///< コマンドの送信データサイズが固定なら書いてください
  GFL_NET_PTRCommRecvBuffAddr getAddrFunc;
} NetRecvFuncTable;

/// 可変データ送信であることを示している
//#define   GFL_NET_COMMAND_SIZE_VARIABLE   (0x00ffffff)
/// コマンドのサイズを関数か実数か判断する為の定義 (使用しないでください)
//#define   _GFL_NET_COMMAND_SIZE_H         (0xff000000)
/// コマンドのサイズを定義する為のマクロ
//#define   GFL_NET_COMMAND_SIZE( num )     (PTRCommRecvSizeFunc)(_GFL_NET_COMMAND_SIZE_H + num)
/// コマンドのサイズを定義する為のマクロ
//#define   GFL_NET_COMMAND_VARIABLE     (PTRCommRecvSizeFunc)(_GFL_NET_COMMAND_SIZE_H + GFL_NET_COMMAND_SIZE_VARIABLE)


typedef enum {
  GFL_NET_ERROR_RESET_SAVEPOINT = 1,  ///< 直前のセーブに戻すエラー
  GFL_NET_ERROR_RESET_TITLE,          ///< タイトルに戻すエラー
  GFL_NET_ERROR_RESET_OTHER,          ///< その他のエラー
} GFL_NET_ERROR_ENUM;


typedef enum {
  GFL_NET_ICON_WIFINCGR,   ///< WIFIの絵のデータ
  GFL_NET_ICON_WMNCGR,     ///< Wirelessの絵のデータ
  GFL_NET_ICON_WMNCLR,     ///< パレットのデータ
} GFL_NET_ICON_ENUM;       /// 通信アイコンファイルを外部からもらうときの識別番号

enum {
  GFL_NET_TYPE_WIRELESS,		///<ワイヤレス通信
  GFL_NET_TYPE_WIFI,			///<WIFI通信
  GFL_NET_TYPE_WIFI_LOBBY,	///<WIFI広場通信
  GFL_NET_TYPE_IRC,			///<赤外線通信
  GFL_NET_TYPE_IRC_WIRELESS,	///<赤外線通信でマッチング後、ワイヤレス通信へ移行
  GFL_NET_TYPE_WIRELESS_SCANONLY,	///<ワイヤレス通信(スキャン専用・電源ランプ非点滅)
  GFL_NET_TYPE_WIFI_GTS,		///<GTS通信（WIFIデバイスのロードのみ行う）
};

enum{
  GFL_NET_CHANGEOVER_MODE_NORMAL,       ///<通常状態(親子を一定間隔で切り替え)
  GFL_NET_CHANGEOVER_MODE_FIX_PARENT,   ///<親固定
  GFL_NET_CHANGEOVER_MODE_FIX_CHILD,    ///<子固定
};

typedef u8 GameServiceID;  ///< ゲームサービスID  通信の種類
typedef u8 ConnectID;      ///< 接続するためのID  0-16 まで
typedef u8 NetID;          ///< 通信ID  0-16 まで

/// @brief 送信完了を受け取るコールバック型
typedef void (*CBSendEndFunc)(u16 command);

/// イクニューモンの終了を待つ
typedef void (*PTRIchneumonCallback)(void* pWork, BOOL bSuccess);
/// @brief 常に送る送信データを得る
typedef u8* (*CBGetEveryTimeData)(void);
/// @brief 常に送られる受信データを得る 送られていないとpWorkにNULLが入る
typedef void (*CBRecvEveryTimeData)(int netID, void* pWork, int size);

// こちらは接続完了後に交換し合うデータ
typedef void* (*NetInfomationGetFunc)(void* pWork);    ///< ユーザー同士が交換するデータのポインタ取得関数
typedef int (*NetInfomationGetSizeFunc)(void* pWork);  ///< ユーザー同士が交換するデータのサイズ取得関数型
// こちらは親機から子機に一方的に送られるデータ
typedef void* (*NetBeaconGetFunc)(void* pWork);        ///< ビーコンデータ取得関数        上と同じでかまわない
typedef int (*NetBeaconGetSizeFunc)(void* pWork);      ///< ビーコンデータサイズ取得関数  上と同じでかまわない
typedef BOOL (*NetBeaconCompFunc)(GameServiceID GameServiceID1, GameServiceID GameServiceID2);  ///< ビーコンのサービスを比較して繋いで良いかどうか判断する

typedef void (*NetAutoParentConnect)(void* work);  ///< 自動接続したときに親になったマシンで呼び出される関数

typedef void (*NetErrorFunc)(GFL_NETHANDLE* pNet,int errNo, void* pWork);    ///< 通信エラーが起こった場合呼ばれる 通信切断するしかない
typedef void (*NetFatalErrorFunc)(GFL_NETHANDLE* pNet,int errNo, void* pWork);    ///< 通信不能なエラーが起こった場合呼ばれる 電源切断するしかない
typedef void (*NetConnectEndFunc)(void* pWork);  ///< 通信切断時に呼ばれる関数

typedef u8* (*NetGetSSID)(void);  ///< 親子接続時に認証する為のバイト列 24byte
typedef void (*NetWifiSaveUserDataFunc)(void);  ///< WiFiユーザーデータをセーブする時に呼ぶ関数
typedef void (*NetWifiMargeFrinedDataFunc)(int baseFrinedNo, int delFriendNo);  ///< WiFiフレンドデータをマージする状況時に呼ぶ関数
typedef const char** (*NetIconGraTableFunc)(void);     ///< 通信アイコンのファイルARCテーブルを返す関数
typedef void (*NetIconGraNoBuffFunc)(int* pNoBuff);  ///< 通信アイコンのファイルARCの番号を返す関数
//typedef void (*NetKeyMainFunc)(UI_KEYSYS* pKey, u16 keyData);  ///< キー内部処理

typedef void (*NetModeChangeFunc)(void* pWork, BOOL bResult);   ///< 通信形態変更時に呼ばれるコールバック
typedef void (*NetEndCallback)(void* pWork);   ///< 通信終了時に呼ばれるコールバック

typedef void (*NetStepEndCallback)(void* pWork);   ///< 通信の区切りに呼ばれる汎用コールバック型
typedef void (*NetPreStepEndCallback)(void* pWork,BOOL bParent);   ///< 通信の区切りに呼ばれる汎用コールバック型

typedef void (*NetConnectHardware)(void* pWork,int hardID);  ///< 機械的に接続した時に呼ばれる
typedef void (*NetConnectNegotiation)(void* pWork,int netID); ///< ネゴシエーションがすんだら呼ばれる

typedef void (*NetDeleteFriendListCallback)(int deletedIndex, int srcIndex, void* pWork); ///< フレンドリスト削除コールバック

typedef void* (*NetGetDWCUserDataCallback)(void* pWork); ///< DWCのユーザデータ（自分のデータ）を得るDWCUserData
typedef void* (*NetGetDWCFriendDataCallback)(void* pWork); ///< DWC形式の友達リストDWCFriendData


/// @brief 通信の初期化用構造体
typedef struct{
  const NetRecvFuncTable* recvFuncTable;  ///< 受信関数テーブルのポインタ
  int recvFuncTableNum;              ///< 受信関数テーブル項目数 NELEMS
  NetConnectHardware connectHardFunc;    ///< ハードで接続した時に呼ばれる
  NetConnectNegotiation  connectNegotiationFunc;  ///<  ネゴシエーション完了時にコール
  NetInfomationGetFunc infoFunc;             ///<ユーザー同士が交換するデータのポインタ取得関数
  NetInfomationGetSizeFunc infoSizeFunc;         ///< ユーザー同士が交換するデータのサイズ取得関数
  NetBeaconGetFunc beaconGetFunc;    ///< ビーコンデータ取得関数
  NetBeaconGetSizeFunc beaconGetSizeFunc;   ///< ビーコンデータサイズ取得関数
  NetBeaconCompFunc beaconCompFunc;  ///< ビーコンのサービスを比較して繋いで良いかどうか判断する
  NetErrorFunc errorFunc;            ///< 通信不能なエラーが起こった場合呼ばれる 通信を終了させる必要がある
  NetFatalErrorFunc fatalErrorFunc;      ///< 通信不能エラーが発生。 電源切断する必要がある
  NetConnectEndFunc connectEndFunc;  ///< 通信切断時に呼ばれる関数
  NetAutoParentConnect autoParentConnect; ///< 自動接続したときに親になったマシンで呼び出される
#if GFL_NET_WIFI
  NetWifiSaveUserDataFunc wifiSaveFunc;     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NetWifiMargeFrinedDataFunc wifiMargeFunc; ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NetDeleteFriendListCallback friendDeleteFunc;  ///< wifiフレンドリスト削除コールバック
  NetGetDWCFriendDataCallback keyList;   ///< DWC形式の友達リスト
  NetGetDWCUserDataCallback myUserData;  ///< DWCのユーザデータ（自分のデータ）
  u32 heapSize;           ///< DWCへのヒープサイズ
  u32 bDebugServer;        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  u32 ggid;                 ///< ＤＳでゲームソフトを区別する為のID 任天堂からもらう
  HEAPID baseHeapID;        ///< 元となるHEAPID
  HEAPID netHeapID;         ///< 通信用にcreateされるHEAPID
  HEAPID wifiHeapID;        ///< wifi用にcreateされるHEAPID
  HEAPID ircHeapID;         ///< IRC用にcreateされるHEAPID	※check
  u16 iconX;                ///< 通信アイコンX位置
  u16 iconY;                ///< 通信アイコンY位置
  u8 maxConnectNum;         ///< 最大接続人数
  u8 maxSendSize;           ///< 送信サイズ
  u8 maxBeaconNum;          ///< 最大ビーコン収集数  = wifiフレンドリスト数
  u8 bCRC;                  ///< CRCを自動計算するかどうか TRUEの場合すべて計算する
  u8 bMPMode;               ///< MP通信モードかどうか
  u8 bNetType;              ///< 使用する通信を指定(GFL_NET_TYPE_???)	※check
  u8 bTGIDChange;           ///< 親が再度初期化した場合、つながらないようにする場合TRUE
  GameServiceID gsid;                 ///< ゲームサービスID  通信の種類  バトルやユニオンとかで変更する値
#if GFL_NET_IRC
  u32 irc_timeout;			///< 赤外線再接続時のタイムアウト時間
#endif	//GFL_NET_IRC
  u16 maxMPParentSize;    ///< MP通信時に親の送信量を増やしたい場合サイズ指定 そうしない場合０
  u16 dummy;
} GFLNetInitializeStruct;

//-------------------------------
//  関数 ～外部公開関数は全てここに定義
//-------------------------------

//==============================================================================
/**
 * @brief    boot時の通信初期化
 *           WIFIのIPL初期設定が起動時に必要な為
 * @param    heapID    使用するtempメモリID
 * @param    errorFunc エラー時に呼び出す関数
 * @param    fixHeapID 最初から通信領域をとる場合   親ID
 * @param    childHeapID 最初から通信領域をとる場合 子供ID  この二つが一緒の場合、ここで確保をしない
 * @return   none
 */
//==============================================================================
extern void GFL_NET_Boot(HEAPID heapID, NetErrorFunc errorFunc, HEAPID fixHeapID, HEAPID childHeapID);
//==============================================================================
/**
 * @brief   通信初期化
 * @param   pNetInit  通信初期化構造体のポインタ
 * @param   NetStepEndCallback  初期化完了時に呼ばれるコールバック
 * @param   pWork  使用しているユーザーのワークポインタ コールバックで一緒に渡します
 * @return  none
 */
//==============================================================================
extern void GFL_NET_Init(const GFLNetInitializeStruct* pNetInit, NetStepEndCallback callback, void* pWork);

//==============================================================================
/**
 * @brief 通信初期化が完了したかどうかを確認する
 * @retval TRUE   初期化終了
 * @retval FALSE  初期化がまだ終わっていない
 */
//==============================================================================
extern BOOL GFL_NET_IsInit(void);
//==============================================================================
/**
 * @brief  通信が終了したかどうか
 * @retval TRUE   終了
 * @retval FALSE  まだ終了していない
 */
//==============================================================================
extern BOOL GFL_NET_IsExit(void);
//==============================================================================
/**
 * @brief   通信終了
 * @param   netEndCallback    通信が終了した際に呼ばれるコールバックです
 * @retval  TRUE:終了リクエストを受け付けた
 * @retval  FALSE:既に解放済み or システムが作成されていない
 */
//==============================================================================
extern BOOL GFL_NET_Exit(NetEndCallback netEndCallback);
//==============================================================================
/**
 * @brief   通信のメイン実行関数
 * @retval  TRUE  処理を行ってよい場合
 * @retval  FALSE 同期が取れていないので処理を行わない場合
 */
//==============================================================================
extern BOOL GFL_NET_Main(void);

//-----ビーコン関連
//==============================================================================
/**
 * @brief ビーコンを受け取るサービスを追加指定  （いくつかのサービスのビーコンを拾いたい時に）
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  none
  現在未完成
 */
//==============================================================================
//extern void GFL_NET_AddBeaconServiceID(GFL_NETHANDLE* pNet, GameServiceID gsid);
//==============================================================================
/**
 * @brief   受信ビーコンデータを得る
 * @param   index  ビーコンバッファindex
 * @return  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
extern void* GFL_NET_GetBeaconData(int index);
//==============================================================================
/**
 * @brief   受信ビーコンに対応したMACアドレスを得る
 * @param   index  ビーコンバッファindex
 * @return  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
extern u8* GFL_NET_GetBeaconMacAddress(int index);

//-----通信ハンドル管理
//==============================================================================
/**
 * @brief   このハンドルの通信番号を得る
 * @param   pNetHandle    通信ハンドルのポインタ
 * @retval  NetID         通信ID
 */
//==============================================================================
extern NetID GFL_NET_GetNetID(GFL_NETHANDLE* pNetHandle);


//-----通信接続管理
//==============================================================================
/**
 * @brief   子機初期化を行い指定した親機に接続する
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_InitClientAndConnectToParent(u8* macAddress);
//==============================================================================
/**
 * @brief   指定した親機に接続する（子機初期化済み）
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_ConnectToParent(u8* macAddress);
//==============================================================================
/**
 * @brief   子機になりビーコンを集める
 * @param   pHandle  通信ハンドルのポインタ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_StartBeaconScan(void);
//==============================================================================
/**
 * @brief    親機になり待ち受ける
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_InitServer(void);
//==============================================================================
/**
 * @brief    親機子機を繰り返し、誰でもいいので接続する
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_ChangeoverConnect(NetStepEndCallback callback);
//==============================================================================
/**
 * @brief    親機子機を繰り返し、誰でもいいので接続する 赤外線無線専用
 * @param    callback     接続コールバック
 * @param    precallback  一回目の赤外線が終了した時のコールバック
 * @return   none
 */
//==============================================================================
extern void GFL_NET_ChangeoverConnect_IRCWIRELESS(NetStepEndCallback callback,NetPreStepEndCallback precallback,NetStepEndCallback ircendcallback);

//==============================================================================
/**
 * @brief    親機子機を繰り返す
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_Changeover(NetStepEndCallback callback);
//==================================================================
/**
 * GFL_NET_Changeover or GFL_NET_ChangeoverConnect の内部モードを後から変更する
 *
 * @param   mode		      GFL_NET_CHANGEOVER_MODE_???
 * @param   bAuto		      TRUE:自動接続
 * @param   macAddress		接続先(親)へのMacAddress
 */
//==================================================================
extern void GFL_NET_ChangeoverModeSet(int mode, BOOL bAuto, const u8 *macAddress);

//==============================================================================
/**
 * @brief    Wi-Fiロビーへ接続する
 * @param    GFL_NETHANDLE        通信ハンドルのポインタ
 * @param    GFL_WIFI_FRIENDLIST  フレンドリスト構造体のポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_WifiLogin(void);
//==============================================================================
/**
 * @brief    WIFIロビーへ接続しているかどうか
 * @param    NetHandle* pNet  通信ハンドル
 * @retval   TRUE   ロビー接続
 * @retval   FALSE   まだ接続していない
 */
//==============================================================================
extern BOOL GFL_NET_IsWifiLobby(void);




// 認証に関する関数

//==============================================================================
/**
 * @brief   自分のマシンのハンドルを帰す
 * @retval  GFL_NETHANDLE
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_HANDLE_GetCurrentHandle(void);

//==============================================================================
/**
 * @brief 現在の接続台数を得る
 * @retval  int  接続数
 */
//==============================================================================
extern int GFL_NET_GetConnectNum(void);

//==============================================================================
/**
 * @brief IDの通信と接続しているかどうかを返す
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetID id            ネットID
 * @retval  BOOL  接続していたらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_IsConnectMember(NetID id);

//==============================================================================
/**
 * @brief   通信アイコンの位置を変更します
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_ChangeIconPosition(int x,int y);

//==============================================================================
/**
 * @brief   通信アイコンを再度リロードします
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_ReloadIcon(void);

//==============================================================================
/**
 * @brief 送信開始を子機に伝える（親機専用関数）
 *        この関数以降設定した送信データが子機で使用される
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   NetID id            子機ID
 * @return  none
 */
//==============================================================================
extern void GFL_NET_SendEnableCommand(GFL_NETHANDLE* pNet,const NetID id);

//==============================================================================
/**
 * @brief 送信許可が下りたかどうかを検査する（子機専用関数）
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   許可
 * @retval  FALSE  禁止
 */
//==============================================================================
extern BOOL GFL_NET_IsSendEnable(GFL_NETHANDLE* pNet);

//==============================================================================
/**
 * @brief 送信開始
 *        全員に無条件で送信  送信サイズは初期化時のテーブルから引き出す
 *        データは必ずコピーする
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u16 sendCommand                送信するコマンド
 * @param[in]   u16 size                       送信するサイズ
 * @param[in]   void* data                     送信データポインタ
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GFL_NET_SendData(GFL_NETHANDLE* pNet,const u16 sendCommand,const u16 size,const void* data);

//==============================================================================
/**
 * @brief 送信開始
 * @param[in,out]  pNet  通信ハンドル
 * @param[in]   sendID                     送信相手 全員へ送信する場合 NET_SENDID_ALLUSER
 * @param[in]   sendCommand                送信するコマンド
 * @param[in]   size                       送信データサイズ
 * @param[in]   data                       送信データポインタ
 * @param[in]   bFast                      優先順位を高くして送信する場合TRUE
 * @param[in]   bRepeat                    このコマンドがキューにないときだけ送信
 * @param[in]   bSendBuffLock              送信バッファを呼ぶ側が保持する場合（通信側のメモリを消費しないので大きいデータを送信できます）
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GFL_NET_SendDataEx(GFL_NETHANDLE* pNet,const NetID sendID,const u16 sendCommand, const u32 size,const void* data, const BOOL bFast, const BOOL bRepeat, const BOOL bSendBuffLock);

//==============================================================================
/**
 * @brief 送信開始 相手をBIT単位で指定可能
 * @param[in,out]  pNet  通信ハンドル
 * @param[in]   sendBit                    BIT指定 子機 1=0x01 ...  親機 = 0x80
 * @param[in]   sendCommand                送信するコマンド
 * @param[in]   size                       送信データサイズ
 * @param[in]   data                       送信データポインタ
 * @param[in]   bFast                      優先順位を高くして送信する場合TRUE
 * @param[in]   bRepeat                    このコマンドがキューにないときだけ送信
 * @param[in]   bSendBuffLock              送信バッファを呼ぶ側が保持する場合（通信側のメモリを消費しないので大きいデータを送信できます）
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GFL_NET_SendDataExBit(GFL_NETHANDLE* pNet,const u8 sendBit,const u16 sendCommand, const u32 size,const void* data, const BOOL bFast, const BOOL bRepeat, const BOOL bSendBuffLock);


//==============================================================================
/**
 * @brief 送信データが無いかどうか
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   無い場合
 * @retval  FALSE  残っている場合
 */
//==============================================================================
extern BOOL GFL_NET_IsEmptySendData(GFL_NETHANDLE* pNet);


//----------------------------------------------------------------
/**
 * @brief   特定のコマンドを送信し終えたかどうかを調べる
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//----------------------------------------------------------------
extern BOOL GFL_NET_IsInSendCommandQueue(GFL_NETHANDLE* pNet,int command);



//--------その他、ツール類
//==============================================================================
/**
 * @brief    接続可能なマシンの台数＝自分含む数を返す
 * @return   接続人数
 */
//==============================================================================
extern int GFL_NET_GetConnectNumMax(void);
//==============================================================================
/**
 * @brief    送信最大サイズを得る
 * @return   送信サイズ
 */
//==============================================================================
extern int GFL_NET_GetSendSizeMax(void);

//------------------------------------------------------------------------------
/**
 * @brief    MP通信時の親機送信最大サイズを得る
 * @return   送信サイズ
 */
//------------------------------------------------------------------------------
extern int GFL_NET_GetMpParentSendSizeMax(void);


//==============================================================================
/**
 * @brief    初期化構造体を得る
 * @return   初期化構造体
 */
//==============================================================================
extern GFLNetInitializeStruct* GFL_NET_GetNETInitStruct(void);
//==============================================================================
/**
 * @brief   タイミングコマンドを発行する
 * @param   NetHandle* pNet  通信ハンドル
 * @param   u8 no   タイミング取りたい番号
 * @return  none
 */
//==============================================================================
extern void GFL_NET_TimingSyncStart(GFL_NETHANDLE* pNet, const u8 no);

//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   NetHandle* pNet  通信ハンドル
 * @param   no   届く番号
 * @retval  TRUE    タイミングが合致
 * @retval  FALSE   タイミングがそろっていない
 */
//==============================================================================
extern BOOL GFL_NET_IsTimingSync(GFL_NETHANDLE* pNet, const u8 no);

//==============================================================================
/**
 * @brief   DSモードへ通信を切り替える
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetModeChangeFunc   変更した際に呼ばれるコールバック関数
 * @retval  成功したかどうか TRUEで成功
 */
//==============================================================================
extern BOOL GFL_NET_ChangeDsMode(GFL_NETHANDLE* pNet,NetModeChangeFunc func);
//==============================================================================
/**
 * @brief   MPモードへ通信を切り替える
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetModeChangeFunc   変更した際に呼ばれるコールバック関数
 * @retval  成功したかどうか TRUEで成功
 */
//==============================================================================
extern BOOL GFL_NET_ChangeMpMode(GFL_NETHANDLE* pNet,NetModeChangeFunc func);

//==============================================================================
/**
 * @brief   サービスモードを切り替える
 * @param   gsid         サービスモード
 * @param   connectnum   ローカル接続人数 ＜ マックス以下
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_ChangeGameService(GameServiceID gsid,int connectnum);

//==============================================================================
/**
 * @brief 新規接続禁止＆許可を設定
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   BOOL bEnable     TRUE=接続許可 FALSE=禁止
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SetClientConnect(GFL_NETHANDLE* pNet,BOOL bEnable);

//==============================================================================
/**
 * @brief   リセットできる状態かどうか
 * @retval  TRUEならリセット可能
 */
//==============================================================================

extern BOOL GFL_NET_IsResetEnable(void);

//==============================================================================
/**
 * @brief   親機マシンかどうか
 * @param   void
 * @retval  TRUE   親機
 * @retval  FALSE  子機
 */
//==============================================================================
extern BOOL GFL_NET_IsParentMachine(void);

//==============================================================================
/**
 * @brief   親機のハンドルかどうか
 * @param   void
 * @retval  TRUE   親機
 * @retval  FALSE  子機
 */
//==============================================================================
extern BOOL GFL_NET_IsParentHandle(GFL_NETHANDLE* pNetHandle);

//==============================================================================
/**
 * @brief    FatalError実行
 * @param    pNetHandle  ネットハンドル
 * @param    errorNo     エラー番号
 * @return   none
 */
//==============================================================================
extern void GFL_NET_FatalErrorFunc(int errorNo);
//==============================================================================
/**
 * @brief    通信不能なエラーが起こった場合呼ばれる 通信を終了させる必要がある
 * @param    pNetHandle  ネットハンドル
 * @param    errorNo     エラー番号
 * @return   none
 */
//==============================================================================
extern void GFL_NET_ErrorFunc(int errorNo);

//==============================================================================
/**
 * @brief   エラーにする場合この関数を呼ぶとエラーになります
 * @param   エラー種類
 * @retval  受け付けた場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateSetError(int no);

//==============================================================================
/**
 * @brief   GFL_NET_StateSetErrorで設定されたエラー番号を検出します
 * @retval  エラー種類
 */
//==============================================================================
extern int GFL_NET_StateGetError(void);

//==============================================================================
/**
 * @brief   WifiErrorの場合その番号を返す  エラーになってるかどうかを確認してから引き出すこと
 * @param   none
 * @retval  Error番号
 */
//==============================================================================
extern int GFL_NET_StateGetWifiErrorNo(void);

#if GFL_NET_WIFI
//==============================================================================
/**
 * @brief   ユーザーデータを初期化する
 * @param   DWCUserData  ユーザーデータ構造体
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_WIFI_InitUserData(void *pUserdata);
#endif

//==============================================================================
/**
 * @brief   WIFI接続要求が来たかどうか
 * @retval  TRUE  きた
 * @retval  FALSE こない
 */
//==============================================================================
extern BOOL GFL_NET_WIFI_IsNewPlayer(void);
//==============================================================================
/**
 * @brief   ランダムマッチ接続を開始する
 * @param   pNetHandle  ネットハンドル
 * @retval  TRUE  成功
 * @retval  FALSE 失敗
 */
//==============================================================================
extern BOOL GFL_NET_StartRandomMatch(void);
//==============================================================================
/**
 * @brief   物理的なWIFIコネクション番号を得る
 * @retval  -1    コネクションが張られてない
 * @retval  0以上 コネクションのID
 */
//==============================================================================
extern int GFL_NET_WIFI_GetLocalConnectNo(void);
//==============================================================================
/**
 * @brief   通信を同期か非同期かにきりかえる
 * @param   TRUE  同期    FALSE 非同期
 * @return  なし
 */
//==============================================================================
extern void GFL_NET_SetWifiBothNet(BOOL flag);
//==============================================================================
/**
 * @brief   ユーザーの使用ワークを再設定する
 * @param   ワークのポインタ
 * @return  なし
 */
//==============================================================================
extern void GFL_NET_SetUserWork(void* pWork);
//==============================================================================
/**
 * @brief   自動的にエラー検出を行うかどうか
 *          (TRUEの場合エラーをみつけるとブルースクリーン＋再起動になる)
 * @param   bAuto  TRUEで検査開始 FALSEでエラー処理を行わない
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SetAutoErrorCheck(BOOL bAuto);
//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SetNoChildErrorCheck(BOOL bFlg);
//==============================================================================
/**
 * @brief    ビーコン強度を得る
 * @param    index ビーコンバッファに対するindex
 * @retval   u16
 */
//==============================================================================
extern u16 GFL_NET_WL_GetRssi(int index);
//==============================================================================
/**
 * @brief   データシェアリングが成功したかどうか
 * @retval  TRUE  データシェアリング成功
 * @retval  FALSE データシェアリング失敗
 */
//==============================================================================
extern BOOL GFL_NET_SystemCheckDataSharing(void);

//==============================================================================
/**
 * @brief   この関数はライブラリ外に作成する関数
 *          ビーコンのヘッダーを文字列で定義 sizeバイト分必要 (６バイト)
 * @param   pHeader  文字列を入れる関数
 * @param   size     サイズ
 */
//==============================================================================
extern void GFLR_NET_GetBeaconHeader(u8* pHeader, int size);

//==============================================================================
/**
 * @brief   この関数はライブラリ外に作成する関数
 *          GGID  ゲームの通信IDを取得る関数
 * @retval  GGID
 */
//==============================================================================
extern u32 GFLR_NET_GetGGID(void);


/*-------------------------------------------------------------------------*
 * Name        : GFL_NET_Align32Alloc
 * Description : メモリ確保関数
 * Arguments   : name  - 確保する名前
 *             : size  - 確保するサイズ
 *             : align - 確保するアライメント
 * Returns     : *void - 確保したポインタ
 * HEAPは4byte境界で返ってくるため、それを無理やり32byte境界へ
 * アライメントし直す。
 * 返すアドレス-4の4byteに元のallocしたアドレスを保存しておいて、
 * freeする際にはその値を参照してフリーするようにしてある
 *-------------------------------------------------------------------------*/


#ifndef HEAPSYS_DEBUG
//この関数を直接呼び出すのは禁止
extern void* GFI_NET_Align32Alloc( HEAPID id, u32 size );

#define GFL_NET_Align32Alloc( heapID, size )  GFI_NET_Align32Alloc( heapID, size )

#else

//この関数を直接呼び出すのは禁止
extern void* GFI_NET_Align32Alloc( HEAPID heapID, u32 size, const char* filename, u16 linenum );

#define GFL_NET_Align32Alloc( heapID, size )  GFI_NET_Align32Alloc( heapID, size, __FILE__, __LINE__)

#endif  // #ifndef HEAPSYS_DEBUG



/*-------------------------------------------------------------------------*
 * Name        : GFL_NET_Align32Free
 * Description : メモリ開放関数
 * Arguments   : *ptr - 解放するメモリのポインタ
 * Returns     : None.
 *-------------------------------------------------------------------------*/
extern void GFL_NET_Align32Free( void* ptr );


extern void GFL_NET_BG1PosSet(int x, int y);
extern void GFL_NET_BG1PosGet(int *x, int *y);


extern void debugcheck(u32* data,int size );

//==============================================================================
/**
 * @brief       GFL_NETSYS構造体のにあるpWorkを得る
 * @param       none
 * @return      GFL_NETSYS
 */
//==============================================================================
extern void* GFL_NET_GetWork(void);
//==============================================================================
/**
 * 自分の機のIDを返す
 * @param
 * @retval  自分の機のID  つながっていない場合GFL_NET_NO_PARENTMACHINE
 */
//==============================================================================
extern u16 GFL_NET_SystemGetCurrentID(void);
//--------------------------------------------------------------
/**
 * @brief  初期化構造体の変更
 * @param[in]   pNetInit  通信初期化構造体のポインタ
 */
//--------------------------------------------------------------
extern void GFL_NET_ChangeInitStruct(const GFLNetInitializeStruct* pNetInit);
//----------------------------------------------------------------
/**
 * @brief   wifi接続したかどうかを設定する
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//----------------------------------------------------------------
extern BOOL GFL_NET_SystemGetWifiConnect(void);

//==============================================================================
/**
 * @brief   エラー状態かどうか
 * @param   none
 * @retval  エラーならTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsError(void);
//----------------------------------------------------------------
/**
 * @brief   エラー状態ならばエラー番号を取得する
 * @param   none
 * @retval  エラーNo(エラーでない場合は0)
 */
//----------------------------------------------------------------
extern int GFL_NET_SystemGetErrorCode(void);

//--------------------------------------------------------------
/**
 * @brief   Wi-Fi広場の独自エラーが発生しているか
 * @param   none
 * @retval  エラーNo(エラーでない場合は0)
 */
//--------------------------------------------------------------
extern int GFL_NET_SystemIsLobbyError(void);

//----------------------------------------------------------------
/**
 * @brief   通信可能状態の人数を返す
 * @param   none
 * @retval  接続人数
 */
//----------------------------------------------------------------
extern int GFL_NET_SystemGetConnectNum(void);
//----------------------------------------------------------------
/**
 * @brief		特定のコマンドを送信し終えたかどうかを調べる クライアント側
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//----------------------------------------------------------------
extern BOOL GFL_NET_SystemIsSendCommand(int command, int myID);
//----------------------------------------------------------------
/**
 * @brief   エラー状態かどうか
 * @param   none
 * @retval  エラーNo(エラーでない場合は0)
 */
//----------------------------------------------------------------
extern BOOL GFL_NET_IsError(void);
//----------------------------------------------------------------
/**
 * @brief   エラー状態ならばエラー番号を取得する
 * @param   none
 * @retval  エラーNo(エラーでない場合は0)
 */
//----------------------------------------------------------------
extern int GFL_NET_GetErrorCode(void);

//--------低優先度送信
//セットすると、毎フレーム通信バッファの空きを見て、その分を送信します。
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：システム作成(送信側
 *
 * @param   heapId システム作成用HEAP
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_InitSystem( const HEAPID heapId );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：システム削除
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_ExitSystem();
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：送信データ設定
 *
 * @param   data   送信データ
 * @param   size   送信サイズ
 * @param   sendBit送信対象bit
 * @param   isMp   MP通信モードか？
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_SetSendData( void* data , const u32 size , const u8 sendBit , const BOOL isMp );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信バッファ作成
 *
 * @param   size   受信サイズ(最大値
 * @param   netID  送信側のnetID
 * @param   heapId システム作成用HEAP
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_CreatePostBuffer( u32 size , const u8 netId , const HEAPID heapId );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信バッファ作成(外からバッファを渡す
 *
 * @param   size   受信サイズ
 * @param   netID  送信側のnetID
 * @param   heapId システム作成用HEAP
 * @param   bufferAdr バッファ
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_CreatePostBuffer_SetAddres( u32 size , const u8 netId , const HEAPID heapId , void* bufferAdr );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信バッファ削除
 *
 * @param   netID  送信側のnetID
 */
//--------------------------------------------------------------
extern void GFL_NET_LDATA_DeletePostBuffer( const u8 netId );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信バッファ取得
 *
 * @return  void* データ
 */
//--------------------------------------------------------------
extern void* GFL_NET_LDATA_GetPostData( const u8 netId );
//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信サイズ取得
 *
 * @return  u32 サイズ
 */
//--------------------------------------------------------------
extern const u32 GFL_NET_LDATA_GetPostDataSize( const u8 netId );

//--------------------------------------------------------------
/**
 * @brief   低優先度送信：送信完了チェック
 *
 * @return  BOOL TRUE:終了したか
 */
//--------------------------------------------------------------
extern const BOOL GFL_NET_LDATA_IsFinishSend();

//--------------------------------------------------------------
/**
 * @brief   低優先度送信：受信完了チェック
 *
 * @return  BOOL TRUE:終了したか
 */
//--------------------------------------------------------------
extern const BOOL GFL_NET_LDATA_IsFinishPost( const u8 netId );


#ifdef PM_DEBUG
//----------------------------------------------------------------
/**
 * @brief		デバッグしたい時にON
 * @retval  none
 */
//----------------------------------------------------------------
extern void GFL_NET_DebugPrintOn(void);
#endif


#include "net_devicetbl.h"

#if GFL_NET_WIFI
#include "net_wifi.h"
#endif //GFL_NET_WIFI

//#if GFL_NET_IRC
//#include "net_irc.h"
//#endif	//GFL_NET_IRC

#include "net_command.h"
#include "net_handle.h"
#include "net_icondata.h"
#include "net_icon.h"

#ifdef __cplusplus
} /* extern "C" */
#endif
