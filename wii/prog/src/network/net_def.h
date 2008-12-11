//*****************************************************************************
/**
 * @file	net_def.h
 * @brief	ネットワーク用グループ内定義
 * @author	k.ohno
 * @date	2006.11.20
 */
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NET_DEF_H__
#define	__NET_DEF_H__

#include "common_def.h"
#include "net.h"

//------------------------------------------------------------------
/**
 * @brief	定義
 */
//------------------------------------------------------------------

//typedef struct _NET_TOOLSYS_t NET_TOOLSYS;
typedef struct _NET_PARENTSYS_t NET_PARENTSYS;

/// @brief WIRELESS管理構造体
typedef struct _NET_WL_WORK GFL_NETWL;


/// @brief コールバック関数の書式 local解決用
typedef void (*PTRCommRecvLocalFunc)(u16 aid, u16 *data, u16 size);
/// @brief 送信完了コールバック
typedef void (*PTRSendDataCallback)(BOOL result);

/// ビーコンを拾ったときに呼ばれるコールバック関数
typedef void (*_PARENTFIND_CALLBACK)(void);


#define _SCAN_ALL_CHANNEL  (0)  ///< スキャンするチャンネルをランダムで行う場合０に設定する


// 扱えるコマンドの受信最大サイズ
#define  COMM_COMMAND_RECV_SIZE_MAX  (256)
// 扱えるコマンドの送信最大サイズ  (ringbuffと同じ大きさ)
#define  COMM_COMMAND_SEND_SIZE_MAX  (264)



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




enum _negotiationType {
    _NEGOTIATION_CHECK,   // 接続確認中
    _NEGOTIATION_OK,      // 接続確認完了
    _NEGOTIATION_NG,     // 接続失敗
};


// pNetHandleを受け取る
extern GFL_NETHANDLE* _NETHANDLE_GetSYS(void);
// pNetHandleからToolsysを受け取る
extern NET_TOOLSYS* _NETHANDLE_GetTOOLSYS(GFL_NETHANDLE* pNetHandle);

extern GFL_NETWL* _GFL_NET_GetNETWL(void);

extern GFLNetInitializeStruct* _GFL_NET_GetNETInitStruct(void);

extern void _GFL_NET_SetNETWL(GFL_NETWL* pWL);

extern GFL_NETHANDLE* GFL_NET_GetNetHandle(int netID);

extern void GFI_NET_DeleteNetHandle(GFL_NETHANDLE* pHandle);

extern void GFI_NET_FatalErrorFunc(int errorNo);
extern void GFI_NET_ErrorFunc(int errorNo);

extern int GFI_NET_GetConnectNumMax(void);

extern int GFI_NET_GetSendSizeMax(void);

extern void GFI_NET_AutoParentConnectFunc(void);



#if GFL_NET_WIFI //wifi

extern void GFI_NET_NetWifiSaveUserDataFunc(void);
extern void GFI_NET_NetWifiMargeFrinedDataFunc(int deletedIndex,int srcIndex);
extern DWCUserData* GFI_NET_GetMyDWCUserData(void);
extern DWCFriendData* GFI_NET_GetMyDWCFriendData(void);
extern int GFI_NET_GetFriendNumMax(void);

#endif //GFL_NET_WIFI
 
#endif	//__NET_DEF_H__

#ifdef __cplusplus
} /* extern "C" */
#endif
