//=============================================================================
/**
 * @file	net.h
 * @brief	通信ライブラリの外部公開関数
 * @author	k.ohno
 * @date    2006.11.4
 */
//=============================================================================
#ifndef __NET_H__
#define __NET_H__


#if defined(DEBUG_ONLY_FOR_ohno)
#define GFL_NET_WIFI    (0)   ///< WIFIをゲームで使用する場合 ON
#else
#define GFL_NET_WIFI    (0)   ///< WIFIをゲームで使用する場合 ON
#endif

//#if defined(DEBUG_ONLY_FOR_ohno)
//#define BEACON_TEST (0)
//#else
//#define BEACON_TEST    (0) 
//#endif

// デバッグ用決まり文句----------------------
#define GFL_NET_DEBUG   (0)   ///< ユーザーインターフェイスデバッグ用 0:無効 1:有効

#if defined(DEBUG_ONLY_FOR_ohno)
#undef GFL_NET_DEBUG
#define GFL_NET_DEBUG   (1)
#endif  //DEBUG_ONLY_FOR_ohno

#ifndef NET_PRINT
#if GFL_NET_DEBUG
#define NET_PRINT(...) \
  (void) ((OS_Printf(__VA_ARGS__)))
#else   //GFL_NET_DEBUG
#define NET_PRINT(...)           ((void) 0)
#endif  // GFL_NET_DEBUG
#endif  //GFL_NET_PRINT
// デバッグ用決まり文句----------------------
// データダンプ
#ifdef GFL_NET_DEBUG
extern void GFL_NET_SystemDump_Debug(u8* adr, int length, char* pInfoStr);
#define DEBUG_DUMP(a,l,s)  GFL_NET_SystemDump_Debug(a,l,s)
#else
#define DEBUG_DUMP(a,l,s)       ((void) 0)
#endif


/// @brief 通信管理構造体
typedef struct _GFL_NETSYS GFL_NETSYS;
/// @brief ネットワーク単体のハンドル
typedef struct _GFL_NETHANDLE GFL_NETHANDLE;


// define 
#define GFL_NET_NETID_SERVER (0xfe)   ///< NetID:サーバーの場合これ 後は0からClientID
#define GFL_NET_SENDID_ALLUSER (0x10)  ///< NetID:全員へ送信する場合

#define GFL_NET_TOOL_INVALID_LIST_NO  (-1) ///<無効な選択ID

#define GFL_WICON_POSX (240)   ///< ワイヤレスアイコンのデフォルトＸＹ位置
#define GFL_WICON_POSY (0)



/// @brief  コマンド関連の定義

/// コールバック関数の書式
typedef void (*PTRCommRecvFunc)(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
/// サイズが固定の場合サイズを関数で返す
typedef int (*PTRCommRecvSizeFunc)(void);
/// 受信バッファを外部で持っている場合そのポインタ
typedef u8* (*PTRCommRecvBuffAddr)(int netID, void* pWork, int size);

/// コマンドパケットテーブル定義
typedef struct {
    PTRCommRecvFunc callbackFunc;    ///< コマンドがきた時に呼ばれるコールバック関数
    PTRCommRecvSizeFunc getSizeFunc; ///< コマンドの送信データサイズが固定なら書いてください
    PTRCommRecvBuffAddr getAddrFunc;
} NetRecvFuncTable;

/// 可変データ送信であることを示している
#define   GFL_NET_COMMAND_SIZE_VARIABLE   (0x00ffffff)
/// コマンドのサイズを関数か実数か判断する為の定義 (使用しないでください)
#define   _GFL_NET_COMMAND_SIZE_H         (0xff000000)
/// コマンドのサイズを定義する為のマクロ
#define   GFL_NET_COMMAND_SIZE( num )     (PTRCommRecvSizeFunc)(_GFL_NET_COMMAND_SIZE_H + num)
/// コマンドのサイズを定義する為のマクロ
#define   GFL_NET_COMMAND_VARIABLE     (PTRCommRecvSizeFunc)(_GFL_NET_COMMAND_SIZE_H + GFL_NET_COMMAND_SIZE_VARIABLE)


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



typedef u8 GameServiceID;  ///< ゲームサービスID  通信の種類
typedef u8 ConnectID;      ///< 接続するためのID  0-16 まで
typedef u8 NetID;          ///< 通信ID  0-16 まで

/// @brief 送信完了を受け取るコールバック型
typedef void (*CBSendEndFunc)(u16 command);

/// @brief 常に送る送信データを得る
typedef u8* (*CBGetEveryTimeData)(void);
/// @brief 常に送られる受信データを得る 送られていないとpWorkにNULLが入る
typedef void (*CBRecvEveryTimeData)(int netID, void* pWork, int size);


typedef void (*NetAutoParentConnect)(void* work);  ///< 自動接続したときに親になったマシンで呼び出される
typedef void* (*NetBeaconGetFunc)(void);    ///< ビーコンデータ取得関数
typedef int (*NetBeaconGetSizeFunc)(void);    ///< ビーコンデータサイズ取得関数
typedef BOOL (*NetBeaconCompFunc)(GameServiceID GameServiceID1, GameServiceID GameServiceID2);  ///< ビーコンのサービスを比較して繋いで良いかどうか判断する
typedef void (*NetErrorFunc)(GFL_NETHANDLE* pNet,int errNo);    ///< 通信不能なエラーが起こった場合呼ばれる 切断するしかない
typedef void (*NetConnectEndFunc)(GFL_NETHANDLE* pNet);  ///< 通信切断時に呼ばれる関数
typedef u8* (*NetGetSSID)(void);  ///< 親子接続時に認証する為のバイト列 24byte
typedef void (*NetWifiSaveUserDataFunc)(void);  ///< WiFiユーザーデータをセーブする時に呼ぶ関数
typedef void (*NetWifiMargeFrinedDataFunc)(int baseFrinedNo, int delFriendNo);  ///< WiFiフレンドデータをマージする状況時に呼ぶ関数
typedef const char** (*NetIconGraTableFunc)(void);     ///< 通信アイコンのファイルARCテーブルを返す関数
typedef void (*NetIconGraNoBuffFunc)(int* pNoBuff);  ///< 通信アイコンのファイルARCの番号を返す関数



#if GFL_NET_WIFI
#include <dwc.h>
#endif  //GFL_NET_WIFI

/// @brief 通信の初期化用構造体
typedef struct{
  const NetRecvFuncTable* recvFuncTable;  ///< 受信関数テーブルのポインタ
  int recvFuncTableNum;              ///< 受信関数テーブル項目数 NELEMS
  void* pWork;                       ///< この通信ゲームで使用しているユーザーのワーク
  NetBeaconGetFunc beaconGetFunc;    ///< ビーコンデータ取得関数
  NetBeaconGetSizeFunc beaconGetSizeFunc;   ///< ビーコンデータサイズ取得関数
  NetBeaconCompFunc beaconCompFunc;  ///< ビーコンのサービスを比較して繋いで良いかどうか判断する
  NetErrorFunc errorFunc;            ///< 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NetConnectEndFunc connectEndFunc;  ///< 通信切断時に呼ばれる関数
  NetAutoParentConnect autoParentConnect; ///< 自動接続したときに親になったマシンで呼び出される
  NetIconGraTableFunc iconGetTable;     ///< 通信アイコンのファイルARCテーブルを返す関数
  NetIconGraNoBuffFunc iconGetNoBuff;   ///< 通信アイコンのファイルARCの番号を返す関数
#if GFL_NET_WIFI
  NetWifiSaveUserDataFunc wifiSaveFunc;     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NetWifiMargeFrinedDataFunc wifiMargeFunc; ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  DWCFriendData *keyList;   ///< DWC形式の友達リスト	
  DWCUserData *myUserData;  ///< DWCのユーザデータ（自分のデータ）
#endif  //GFL_NET_WIFI
  NetGetSSID getSSID;       ///< 親子接続時に認証する為のバイト列  自由に決めて良いがゲームのタイトルがわかるような文字列
  int gsid;                 ///< ゲームサービスID  通信の種類  バトルやユニオンとかで変更する値
  int ggid;                 ///< ＤＳでゲームソフトを区別する為のID 任天堂からもらう
  HEAPID baseHeapID;        ///< 元となるHEAPID
  HEAPID netHeapID;         ///< 通信用にcreateされるHEAPID
  HEAPID wifiHeapID;        ///< wifi用にcreateされるHEAPID
  u16 iconX;                ///< 通信アイコンX位置
  u16 iconY;                ///< 通信アイコンY位置
  u8 maxConnectNum;         ///< 最大接続人数
  u8 maxSendSize;           ///< 送信サイズ
  u8 maxBeaconNum;          ///< 最大ビーコン収集数  = wifiフレンドリスト数
  u8 bMPMode;               ///< MP通信モードかどうか
  u8 bWiFi;                 ///< Wi-Fi通信をするかどうか
  u8 bNetwork;              ///< 通信を開始するかどうか FALSEだとOFFLINE動作
} GFLNetInitializeStruct;

//-------------------------------
//  関数 ～外部公開関数は全てここに定義
//-------------------------------

//==============================================================================
/**
 * @brief    通信のブート時初期化
 * @param    heapID  使用するtempメモリID
 * @param    errorFunc  エラー時に呼び出す画面表示関数
 * @return   none
 */
//==============================================================================
extern void GFL_NET_Boot(HEAPID heapID, NetErrorFunc errorFunc);
//==============================================================================
/**
 * @brief   通信初期化
 * @param   pNetInit  通信初期化構造体のポインタ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_Init(const GFLNetInitializeStruct* pNetInit);
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
 * @brief   通信終了
 * @retval  TRUE  終了しました
 * @retval  FALSE 終了しません 時間を空けてもう一回呼んでください
 */
//==============================================================================
extern BOOL GFL_NET_Exit(void);
//==============================================================================
/**
 * @brief   通信のメイン実行関数
 * @return  none
 */
//==============================================================================
extern void GFL_NET_Main(void);

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
 * @brief   通信ハンドルを作る   通信一個単位のワークエリアの事
 * @return  GFL_NETHANDLE*  通信ハンドルのポインタ
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_CreateHandle(void);
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
 * @param   pHandle        通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_InitClientAndConnectToParent(GFL_NETHANDLE* pHandle,u8* macAddress);
//==============================================================================
/**
 * @brief   指定した親機に接続する（子機初期化済み）
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_ConnectToParent(GFL_NETHANDLE* pHandle,u8* macAddress);
//==============================================================================
/**
 * @brief   子機になりビーコンを集める
 * @param   pHandle  通信ハンドルのポインタ
 * @return  none
 */
//==============================================================================
extern void GFL_NET_StartBeaconScan(GFL_NETHANDLE* pHandle);
//==============================================================================
/**
 * @brief    親機になり待ち受ける
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_InitServer(GFL_NETHANDLE* pHandle);
//==============================================================================
/**
 * @brief    親機になる
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_CreateServer(GFL_NETHANDLE* pHandle);
//==============================================================================
/**
 * @brief    親機子機を繰り返し、誰でもいいので接続する
 * @param    pHandle  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_ChangeoverConnect(GFL_NETHANDLE* pHandle);

//==============================================================================
/**
 * @brief    Wi-Fiロビーへ接続する
 * @param    GFL_NETHANDLE        通信ハンドルのポインタ
 * @param    GFL_WIFI_FRIENDLIST  フレンドリスト構造体のポインタ
 * @return   none
 */
//==============================================================================
extern void GFL_NET_WifiLogin(GFL_NETHANDLE* pHandle);

//==============================================================================
/**
 * @brief    WIFIロビーへ接続しているかどうか
 * @param    NetHandle* pNet  通信ハンドル
 * @retval   TRUE   ロビー接続
 * @retval   FALSE   まだ接続していない
 */
//==============================================================================
extern BOOL GFL_NET_IsWifiLobby(GFL_NETHANDLE* pNetHandle);

//==============================================================================
/**
 * @brief 通信切断する
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_Disconnect(void);



// 認証に関する関数
//==============================================================================
/**
 * @brief       自分のネゴシエーションがすんでいるかどうか
 * @param       pHandle   通信ハンドル
 * @return      すんでいる場合TRUE   まだの場合FALSE
 */
//==============================================================================
extern BOOL GFL_NET_IsNegotiation(GFL_NETHANDLE* pHandle);

//==============================================================================
/**
 * @brief   ネゴシエーション済み人数を得る
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @return   人数
 */
//==============================================================================
extern int GFL_NET_GetNegotiationConnectNum(GFL_NETHANDLE* pNet);

//==============================================================================
/**
 * @brief       ネゴシエーション開始を親に送信する
 * @param[in]   pHandle   通信ハンドル
 * @return      送信に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_RequestNegotiation(GFL_NETHANDLE* pHandle);


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
extern BOOL GFL_NET_IsConnectMember(GFL_NETHANDLE* pNet,NetID id);

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
 * @param[in]   void* data                     送信データポインタ
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GFL_NET_SendData(GFL_NETHANDLE* pNet,const u16 sendCommand,const void* data);

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
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GFL_NET_SendDataEx(GFL_NETHANDLE* pNet,const NetID sendID,const u16 sendCommand, const u32 size,const void* data, const BOOL bFast, const BOOL bRepeat);

//==============================================================================
/**
 * @brief 送信データが無いかどうか
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   無い場合
 * @retval  FALSE  残っている場合
 */
//==============================================================================
extern BOOL GFL_NET_IsEmptySendData(GFL_NETHANDLE* pNet);



//--------その他、ツール類
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
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_ChangeDsMode(GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   MPモードへ通信を切り替える
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_ChangeMpMode(GFL_NETHANDLE* pNet);

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
 * @brief     VBlank時に行う処理を実行  (通信アイコンの表示)
 * @param     none
 * @return    none
 */
//==============================================================================
extern void GFL_NET_VBlankFunc(void);

#if GFL_NET_WIFI
//==============================================================================
/**
 * @brief   ユーザーデータを初期化する
 * @param   DWCUserData  ユーザーデータ構造体
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_WIFI_InitUserData(DWCUserData *pUserdata);
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
extern BOOL GFL_NET_StartRandomMatch(GFL_NETHANDLE* pNetHandle);
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
 * @brief   自動的にエラー検出を行うかどうか
 *          (TRUEの場合エラーをみつけるとブルースクリーン＋再起動になる)
 * @param   bAuto  TRUEで検査開始 FALSEでエラー処理を行わない
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SetAutoErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bAuto);
//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SetNoChildErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bFlg);


//==============================================================================
/**
 * @brief    イクニューモンライブラリのみを初期化する
 * @return   none
 */
//==============================================================================
extern void GFL_NET_InitIchneumon(void);

//==============================================================================
/**
 * @brief    イクニューモンライブラリの初期化が終わったかどうか
 * @retval   TREU   終了
 * @retval   FALSE  まだ終わっていない
 */
//==============================================================================
extern BOOL GFL_NET_IsInitIchneumon(void);

//==============================================================================
/**
 * @brief    イクニューモンライブラリの終了をしらせる
 * @retval   none
 */
//==============================================================================
extern void GFL_NET_ExitIchneumon(void);
//==============================================================================
/**
 * @brief    ビーコン強度を得る
 * @param    index ビーコンバッファに対するindex
 * @retval   u16
 */
//==============================================================================
extern u16 GFL_NET_WL_GetRssi(int index);


extern void debugcheck(u32* data,int size );


#if GFL_NET_WIFI
#include "net_wifi.h"
#endif //GFL_NET_WIFI

#include "net_command.h"


#endif // __NET_H__

