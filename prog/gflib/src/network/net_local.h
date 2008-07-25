//=============================================================================
/**
 * @file	net_local.h
 * @brief	通信ライブラリの内部公開関数
 * @author	k.ohno
 * @date    2008.3.4
 */
//=============================================================================
#ifndef __NET_LOCAL_H__
#define __NET_LOCAL_H__

#include "net_def.h"
#include "net_handle.h"

///< @brief 通信管理構造体
typedef struct _GFL_NETSYS GFL_NETSYS;

///< @brief  通信システム管理構造体
struct _GFL_NETSYS{
    GFLNetInitializeStruct aNetInit;              ///< 初期化構造体のコピー
    GFL_NETHANDLE aNetHandle[GFL_NET_HANDLE_MAX]; ///< 通信ハンドル
    UI_KEYSYS* pKey[GFL_NET_HANDLE_MAX];  ///< キーバッファ
    GFL_NETWL* pNetWL;                    ///<  ワイヤレスマネージャーポインタ
    NetModeChangeFunc pNetModeChangeFunc;
    NetEndCallback pNetEndCallback;     ///< ネットワーク終了コールバック
};


//==============================================================================
/**
 * @brief       GFL_NETSYS構造体のポインタを得る
 * @param       none
 * @return      GFL_NETSYS
 */
//==============================================================================
extern GFL_NETSYS* _GFL_NET_GetNETSYS(void);

//==============================================================================
/**
 * @brief       netHandleを全て消す
 * @param       pNet      通信システム管理構造体
 * @return      none
 */
//==============================================================================
extern void GFL_NET_HANDLE_DeleteAll(GFL_NETSYS* pNet);

#endif //__NET_LOCAL_H__

