#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	net_wifi.h
 * @brief	WIFI通信ライブラリの外部公開関数
 * @author	k.ohno
 * @date    2007.2.28
 */
//=============================================================================
#ifndef __NET_WIFI_H__
#define __NET_WIFI_H__


//==============================================================================
/**
 * @brief   Wi-Fi接続初期化 初期化時に呼び出しておく必要がある
 * @param   heapID     メモリ領域
 * @param   errorFunc  エラー表示関数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_WifiStart( int heapID, NetErrorFunc errorFunc );

//==============================================================================
/**
 * @brief   Wi-Fi設定画面を呼び出す
 * @param   heapID  メモリ領域
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_WifiUtility( int heapID );

//==============================================================================
/**
 * ランダムマッチを行う関数
 * @target   負…自分でゲームを開催。０以上…接続しに行く相手の、フレンドリスト上の位置
 * @retval  正…成功。０…失敗。
 */
//==============================================================================
extern int GFL_NET_DWC_StartMatch( u8* keyStr,int numEntry, BOOL bParent, u32 timelimit );

//==============================================================================
/**
 * aidを返します。接続するまではMYDWC_NONE_AIDを返します。
 * @retval  aid。ただし接続前はMYDWC_NONE_AID
 */
//==============================================================================
extern u32 GFL_NET_DWC_GetAid(void);
#define MYDWC_NONE_AID (0xffffffff)

//==============================================================================
/**
 * ゲーム募集・参加を行う関数。
 * @target   負…自分でゲームを開催。０以上…接続しに行く相手の、フレンドリスト上の位置
 * @retval  正…成功。０…失敗。
 */
//==============================================================================
extern int GFL_NET_DWC_StartGame( int target,int maxnum, BOOL bVCT );
//==============================================================================
/**
 * 接続を検知した時に呼ばれる関数をセットする
 * @param 	pFunc  接続時に呼ばれる関数
 * @param 	pWork  ワークエリア
 * @retval  none
 */
//==============================================================================
// 接続コールバック型
//  友達無指定の場合aidは自分のaid
//  友達指定の場合aidは相手ののaid
typedef void (*MYDWCConnectFunc) (u16 aid,void* pWork);
extern void GFL_NET_DWC_SetConnectCallback( MYDWCConnectFunc pFunc, void* pWork );
//==============================================================================
/**
 * @brief   評価状態中の接続数を返す
 * @param   none
 * @retval  数
 */
//==============================================================================
extern int GFL_NET_DWC_AnybodyEvalNum(void);
//==============================================================================
/**
 * 切断を検知した時に呼ばれる関数をセットする
 * @param 	pFunc  切断時に呼ばれる関数
 * @param 	pWork  ワークエリア
 * @retval  none
 */
//==============================================================================
// 切断コールバック型
typedef void (*MYDWCDisconnectFunc) (u16 aid,void* pWork);
extern void GFL_NET_DWC_SetDisconnectCallback( MYDWCDisconnectFunc pFunc, void* pWork );

//==============================================================================
/**
 * DWC通信処理更新を行う
 * @param none
 * @retval 0…正常, 負…エラー発生
 */
//==============================================================================
extern void GFL_NET_DWC_StartVChat(int heapID);
//==============================================================================
/**
 * @brief   ボイスチャットを停止します
 * @param   void
 * @retval  void
 */
//==============================================================================
extern void GFL_NET_DWC_StopVChat(void);

// すでに、同じデータがリスト上にある。この場合は何もする必要なし。
#define DWCFRIEND_INLIST 0
// すでに、同じデータがリスト上にあるが、上書きすることが望ましい場合。
#define DWCFRIEND_OVERWRITE 1
// 同じデータはリスト上にない。
#define DWCFRIEND_NODATA 2
// データが正しくない。
#define DWCFRIEND_INVALID 3

//==============================================================================
/**
 * すでに同じ人が登録されていないか。
 * @param   index 同じデータが見つかったときの、その場所。
 * @param         見つからない場合は空いているところを返す
 * @param         どこも空いていない場合は、-1 
 * @retval  DWCFRIEND_INLIST … すでに、同じデータがリスト上にある。この場合は何もする必要なし。
 * @retval  DWCFRIEND_OVERWRITE … すでに、同じデータがリスト上にあるが、上書きすることが望ましい場合。
 * @retval  DWCFRIEND_NODATA … 同じデータはリスト上にない。
 * @retval  DWCFRIEND_INVALID … 受け取ったDWCFriendDataが異常。
 */
//==============================================================================
extern int GFL_NET_DWC_CheckFriendByToken(DWCFriendData *data, int *index);

//==============================================================================
/**
 * エラーコードを保持する
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateSetWifiError(int code, int type, int ret);

typedef struct{
  int errorCode;
  int errorType;
  int errorRet;
} GFL_NETSTATE_DWCERROR;

//==============================================================================
/**
 * エラーコードを引き出す
 * @param   none
 * @retval  none
 */
//==============================================================================
extern GFL_NETSTATE_DWCERROR* GFL_NET_StateGetWifiError(void);



/// WiFiで使うHeapのサイズ
#define MYDWC_HEAPSIZE (0x30000)

// 受信コールバック型。WHReceiverFuncと同形
typedef void (*MYDWCReceiverFunc) (u16 aid, u16 *data, u16 size);

//接続検査 フレンドコードリストインデックス
typedef BOOL (*MYDWCConnectModeCheckFunc) (int index);

#define DWCRAP_STARTGAME_OK (0)
#define DWCRAP_STARTGAME_NOTSTATE  (-1)
#define DWCRAP_STARTGAME_RETRY  (-2)
#define DWCRAP_STARTGAME_FAILED  (-3)
#define DWCRAP_STARTGAME_FIRSTSAVE (-4)

#define STEPMATCH_CONTINUE 0
#define STEPMATCH_SUCCESS  (DWC_ERROR_NUM)
#define STEPMATCH_CANCEL (STEPMATCH_SUCCESS+1)
#define STEPMATCH_FAIL (STEPMATCH_SUCCESS+2)
#define STEPMATCH_TIMEOUT (STEPMATCH_SUCCESS+3)
#define STEPMATCH_DISCONNECT (STEPMATCH_SUCCESS+4)
#define ERRORCODE_0 (STEPMATCH_SUCCESS+5)
#define ERRORCODE_HEAP (STEPMATCH_SUCCESS+6)
#define STEPMATCH_CONNECT (STEPMATCH_SUCCESS+7)


#if GFL_NET_WIFI //wifi
// ここから下はデバイスが逆アクセスする為に必要なものなので使用しないでください
extern void GFI_NET_NetWifiSaveUserDataFunc(void);
extern void GFI_NET_NetWifiMargeFrinedDataFunc(int deletedIndex,int srcIndex);
extern DWCUserData* GFI_NET_GetMyDWCUserData(void);
extern DWCFriendData* GFI_NET_GetMyDWCFriendData(void);
extern int GFI_NET_GetFriendNumMax(void);

#endif //GFL_NET_WIFI


#endif //__NET_WIFI_H__
#ifdef __cplusplus
} /* extern "C" */
#endif
