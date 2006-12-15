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

/// @brief WIRELESS管理構造体
typedef struct _NET_WL_WORK GFL_NETWL;


/// @brief コールバック関数の書式 local解決用
typedef void (*PTRCommRecvLocalFunc)(u16 aid, u16 *data, u16 size);
/// @brief 送信完了コールバック
typedef void (*PTRSendDataCallback)(BOOL result);


#define _SCAN_ALL_CHANNEL  (0)  ///< スキャンするチャンネルをランダムで行う場合０に設定する


// 無線で使用するDMA番号
#define COMM_DMA_NO                 (2)
//WMのパワーモード
#define COMM_POWERMODE       (1)
//SSL処理のスレッド優先順位
#define COMM_SSL_PRIORITY     (20)



// 親機を選択できる数。
#define  SCAN_PARENT_COUNT_MAX ( 16 )

// 子機最大数
#define  COMM_CHILD_MAX  ( 7 )

// ありえないID
#define COMM_INVALID_ID  (0xff)


// 機最大数
#define  GFL_NET_MACHINE_MAX  (COMM_CHILD_MAX+1)

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

#endif

