//=============================================================================
/**
 * @file	net_tool.h
 * @brief	通信ツールの関数
 * @author  GAME FREAK Inc.
 * @date    2006.12.5
 */
//=============================================================================
#ifndef __NET_TOOL_H__
#define __NET_TOOL_H__

/// @brief  通信ツールワーク
//typedef struct _NET_TOOLSYS_t NET_TOOLSYS;


// 関数切り出し自動生成 funccut.rb  k.ohno 2006.12.5 
//==============================================================================
/**
 * @brief   ネットワークツール初期化
 * @param   heapID    ヒープ確保を行うID
 * @param   num       初期化人数
 * @return  NET_TOOLSYS  ネットワークツールシステムワーク
 */
//==============================================================================
extern NET_TOOLSYS* GFL_NET_TOOL_Init(const HEAPID heapID, const int num);
//==============================================================================
/**
 * @brief   ネットワークツール終了
 * @param   pCT    ネットワークツールシステムワーク
 * @return  none
 */
//==============================================================================
extern void GFL_NET_TOOL_End(NET_TOOLSYS* pCT);
//==============================================================================
/**
 * @brief  タイミングコマンドを受信した   GFL_NET_CMD_TIMING_SYNC
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================
extern void GFL_NET_TOOL_RecvTimingSync(const int netID, const int size, const void* pData,
                                       void* pWork, GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   タイミングコマンドENDを受信した   GFL_NET_CMD_TIMING_SYNC_END
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================
extern void GFL_NET_TOOL_RecvTimingSyncEnd(const int netID, const int size, const void* pData,
                                          void* pWork, GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_TOOL_TimingSyncStart(GFL_NETHANDLE* pNet, const u8 no);
//==============================================================================
/**
 * @brief   タイミングコマンドを送信
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_TOOL_TimingSyncSend(GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   pNet   ネットワークハンドル
 * @param   no     検査する番号
 * @retval  TRUE   届いていた
 * @retval  FALSE  届いていない
 */
//==============================================================================
extern BOOL GFL_NET_TOOL_IsTimingSync(GFL_NETHANDLE* pNet, const u8 no);
//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================
extern int GFL_NET_TOOL_GetTimingSyncNo(GFL_NETHANDLE* pNet, int netID);

//==============================================================================
/**
 * @brief   DSMPモード変更のMain関数
 * @param   pNet  通信ハンドル
 * @return  noen
 */
//==============================================================================
extern void GFL_NET_TOOL_TransmissonMain(GFL_NETHANDLE* pNet);


#endif //__NET_TOOL_H__



