//*****************************************************************************
/**
 * @file	net_def.h
 * @brief	ネットワーク用グループ内定義
 * @author	k.ohno
 * @date	2006.11.20
 */
//*****************************************************************************

#ifndef __NET_DEF_H__
#define	__NET_DEF_H__

#include "net.h"

//------------------------------------------------------------------
/**
 * @brief	定義
 */
//------------------------------------------------------------------

typedef struct _NET_TOOLSYS_t NET_TOOLSYS;
typedef struct _NET_PARENTSYS_t NET_PARENTSYS;

/// @brief WIRELESS管理構造体
typedef struct _NET_WL_WORK GFL_NETWL;


/// @brief コールバック関数の書式 local解決用
typedef void (*PTRCommRecvLocalFunc)(u16 aid, u16 *data, u16 size);
/// @brief 送信完了コールバック
typedef void (*PTRSendDataCallback)(BOOL result);

/// コールバック関数の書式 内部ステート遷移用
typedef void (*PTRStateFunc)(GFL_NETHANDLE* pNetHandle);

/// ビーコンを拾ったときに呼ばれるコールバック関数
typedef void (*_PARENTFIND_CALLBACK)(GFL_NETHANDLE* pHandle);


#define _SCAN_ALL_CHANNEL  (0)  ///< スキャンするチャンネルをランダムで行う場合０に設定する


// 無線で使用するDMA番号
#define _NETWORK_DMA_NO                 (2)
//WMのパワーモード
#define _NETWORK_POWERMODE       (1)
//SSL処理のスレッド優先順位
#define _NETWORK_SSL_PRIORITY     (20)



// 親機を選択できる数。
#define  SCAN_PARENT_COUNT_MAX ( 16 )

// 子機最大数
#define  GFL_NET_CHILD_MAX  ( 7 )

// ありえないID
#define COMM_INVALID_ID  (0xff)


/// 機最大数
#define  GFL_NET_MACHINE_MAX  (GFL_NET_CHILD_MAX+1)

///   通信ハンドル最大数  子機全部＋親機 分
#define  GFL_NET_HANDLE_MAX  (GFL_NET_MACHINE_MAX+1)

/// 通信のデータサイズ  GFL_NET_MACHINE_MAX台を基本に構成
#define GFL_NET_DATA_HEADER            (4)
#define GFL_NET_CHILD_DATA_SIZE           (40)
#define GFL_NET_PARENT_DATA_SIZE          (GFL_NET_CHILD_DATA_SIZE * GFL_NET_MACHINE_MAX + GFL_NET_DATA_HEADER)



// 扱えるコマンドの受信最大サイズ
#define  COMM_COMMAND_RECV_SIZE_MAX  (256)
// 扱えるコマンドの送信最大サイズ  (ringbuffと同じ大きさ)
#define  COMM_COMMAND_SEND_SIZE_MAX  (264)



// 親のID
#define COMM_PARENT_ID    (0)

// 通信でメニューを出した場合の戻り値
#define  COMM_RETVAL_NULL    (0)     ///< 選択中
#define  COMM_RETVAL_CANCEL  (1)   ///< userキャンセル
#define  COMM_RETVAL_OK      (2)              ///< 選択完了
#define  COMM_RETVAL_ERROR   (3)              ///< 通信エラー
#define  COMM_RETVAL_DIFFER_REGULATION   (4)              ///< レギュレーションが異なる


#define CURRENT_PLAYER_WORD_IDX    (0)    // 自分の名前を入れるバッファの場所
#define TARGET_PLAYER_WORD_IDX     (1)     // 対象となる名前を入れるバッファの場所
#define ASSAILANT_PLAYER_WORD_IDX  (2)     // 被害者となる名前を入れるバッファの場所


#define CC_NOT_FOUND_PARENT_INFO (0) // 親機情報がない場合  
#define CC_BUSY_STATE  (1)  //通信の内部状態が遷移中で、接続にいけない場合
#define CC_CONNECT_STARTING (2) // 接続関数を呼び出したら


// MYSTATUSを送るための領域
#define COMM_SEND_MYSTATUS_SIZE   (32)
#define COMM_SEND_REGULATION_SIZE (32)



/// ネットワークハンドル
struct _GFL_NETHANDLE{
  NET_TOOLSYS* pTool;      ///< netTool
  NET_PARENTSYS* pParent;  ///< 親の情報を保持するポインタ
  PTRStateFunc state;      ///< ハンドルのプログラム状態
  MATHRandContext32 sRand; ///< 親子機ネゴシエーション用乱数キー
  HEAPID baseHeapID;       ///< 通信がcreateするためのID
  HEAPID netHeapID;        ///< 通信libが使用するID
  HEAPID wifiHeapID;       ///< wifiLibが使用するID
  u8 aMacAddress[6];       ///< 接続先MACアドレス格納バッファ
  u8 negotiationID[(GFL_NET_MACHINE_MAX/8)+(0!=(GFL_NET_MACHINE_MAX%8))]; ///< 接続しているハンドルの状態
  u8 wifiTargetNo;    ///< 接続再起番号(wifi用)
  u8 disconnectType;  ///< 切断時のタイプ(wifi)
  u8 bWifiDisconnect;  ///< 切断フラグ(wifi)
  u8 bDisconnectError; ///< 切断エラー(wifi)
  u8 errorCode;       ///< 最新エラーコード保存
  u8 machineNo;       ///< マシン番号
  u8 serviceNo;       ///< 通信サービス番号
  u16 timer;          ///< 進行タイマー
  u8 bFirstParent;    ///< 繰り返し親子切り替えを行う場合の最初の親状態
  u8 limitNum;        ///< 受付制限したい場合のLIMIT数
  u8 negotiation;     ///< 通信受付の状態
  u8 creatureNo;     ///< 子機としての
  u8 stateError;
  u8 bErrorAuto;
  u8 dsmpChange;        ///< 通信モード変更
  u8 dsmpChangeType;    ///< 通信モード変更タイプ
  u8 bDisconnectState;  ///< 切断状態に入っている場合TRUE
};

enum _negotiationType {
    _NEGOTIATION_CHECK,   // 接続確認中
    _NEGOTIATION_OK,      // 接続確認完了
    _NEGOTIATION_NG,     // 接続失敗
};



// 個人的なデバッグの為のプリント
#ifndef OHNO_PRINT
#if defined(DEBUG_ONLY_FOR_ohno)
//#ifdef  SDK_DEBUG

#if 0
#define OHNO_SP_PRINT(...) \
  (void) ((OS_Printf(__VA_ARGS__)))
#define OHNO_PRINT(...)           ((void) 0)

#else
#define OHNO_PRINT(...) \
  (void) ((OS_Printf(__VA_ARGS__)))
#define OHNO_SP_PRINT(...)           ((void) 0)
#endif

#else   //DEBUG_ONLY_FOR_ohno

#define OHNO_PRINT(...)           ((void) 0)
#define OHNO_SP_PRINT(...)           ((void) 0)

#endif  // DEBUG_ONLY_FOR_ohno
#endif  //OHNO_PRINT


// pNetHandleを受け取る
extern GFL_NETHANDLE* _NETHANDLE_GetSYS(void);
// pNetHandleからToolsysを受け取る
extern NET_TOOLSYS* _NETHANDLE_GetTOOLSYS(GFL_NETHANDLE* pNetHandle);

extern GFL_NETWL* _GFL_NET_GetNETWL(void);

extern GFL_NETSYS* _GFL_NET_GetNETSYS(void);

extern GFLNetInitializeStruct* _GFL_NET_GetNETInitStruct(void);

extern PTRStateFunc GFL_NET_GetStateFunc(GFL_NETHANDLE* pHandle);

extern void _GFL_NET_SetNETWL(GFL_NETWL* pWL);

extern GFL_NETHANDLE* GFL_NET_GetNetHandle(int netID);

extern void GFI_NET_DeleteNetHandle(GFL_NETHANDLE* pHandle);


#if GFL_NET_WIFI //wifi

extern void GFI_NET_NetWifiSaveUserDataFunc(void);
extern void GFI_NET_NetWifiMargeFrinedDataFunc(int deletedIndex,int srcIndex);
extern DWCUserData* GFI_NET_GetMyDWCUserData(void);
extern DWCFriendData* GFI_NET_GetMyDWCFriendData(void);
extern int GFI_NET_GetFriendNumMax(void);

#endif //GFL_NET_WIFI
 
#endif	//__NET_DEF_H__
