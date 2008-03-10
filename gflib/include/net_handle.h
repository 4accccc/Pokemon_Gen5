//=============================================================================
/**
 * @file	net_handle.h
 * @brief	通信ハンドルの関数
 * @author  GAME FREAK Inc. k.ohno
 * @date    2008.2.26
 */
//=============================================================================
#ifndef __NET_HANDLE_H__
#define __NET_HANDLE_H__


///<  @brief  ネットワークハンドル
struct _GFL_NETHANDLE{
    void* pInfomation;       ///<ユーザー同士が交換するデータのポインタ
    int infomationSize;      ///< ユーザー同士が交換するデータのサイズ
    u8 timing[GFL_NET_HANDLE_MAX];       ///< 子機からの受け取り記録
    u16 negotiationBit;  ///< 他の接続を許すかどうか 0なら許可
    u8 timingRecv;       ///< 通信で受け取った番号
    u8 timingSyncEnd;    ///< 同期コマンド用
    u8 timingSendFlg;      ///< 送ったかどうか
    u8 timingSyncMy;     ///< 自分が送ったNO
    u8 negotiationType;        ///< 接続しているハンドルの状態
    u8 negotiationSendFlg;      ///< 送ったかどうか
    u8 serviceNo;              ///< 通信サービス番号
    u8 version;                ///< 通信相手の通信バージョン番号
};


/// @brief  通信ツールワーク
typedef struct _NET_TOOLSYS_t NET_TOOLSYS;


typedef enum {
    _NEG_TYPE_FIRST,
    _NEG_TYPE_SEND_BEFORE,
    _NEG_TYPE_SEND,
    _NEG_TYPE_SEND_AFTER,
    _NEG_TYPE_RECV,
    _NEG_TYPE_CONNECT,       ///< 接続を親が認証した場合 CONNECTになる このコマンド後は通信可能になる
    _NEG_TYPE_END,           ///< 切断状態

} _NEGOTIATION_TYPE_E;


//==============================================================================
/**
 * @brief       netHandleの番号を得る
 * @param       pNet      通信システム管理構造体
 * @param       pHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
extern int GFL_NET_HANDLE_GetNetHandleID(GFL_NETHANDLE* pHandle);

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
extern void GFL_NET_HANDLE_RecvTimingSync(const int netID, const int size, const void* pData,
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
extern void GFL_NET_HANDLE_RecvTimingSyncEnd(const int netID, const int size, const void* pData,
                                          void* pWork, GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_TimingSyncStart(GFL_NETHANDLE* pNet, const u8 no);
//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   pNet   ネットワークハンドル
 * @param   no     検査する番号
 * @retval  TRUE   届いていた
 * @retval  FALSE  届いていない
 */
//==============================================================================
extern BOOL GFL_NET_HANDLE_IsTimingSync(const GFL_NETHANDLE* pNet, const u8 no);
//==============================================================================
/**
 * @brief   ハンドル処理のメイン
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_MainProc(void);
//==============================================================================
/**
 * @brief   DSMPモード変更のMain関数
 * @param   pNet  通信ハンドル
 * @return  noen
 */
//==============================================================================
extern void GFL_NET_TOOL_TransmissonMain(GFL_NETHANDLE* pNet);

//==============================================================================
/**
 * @brief       自分のネゴシエーションがすんでいるかどうか
 * @param[in]   pHandle   通信ハンドル
 * @return      すんでいる場合TRUE   まだの場合FALSE
 */
//==============================================================================
extern BOOL GFL_NET_HANDLE_IsNegotiation(GFL_NETHANDLE* pHandle);

//==============================================================================
/**
 * @brief       ネゴシエーションが完了しているのが何人か
 * @return      すんでいる人数 
 */
//==============================================================================
extern int GFL_NET_HANDLE_GetNumNegotiation(void);
//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック GFL_NET_CMD_NEGOTIATION
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_RecvNegotiation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック GFL_NET_CMD_NEGOTIATION_RETURN
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_RecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   自分のマシンのハンドルを帰す
 * @retval  GFL_NETHANDLE
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_HANDLE_GetCurrentHandle(void);
//==============================================================================
/**
 * @brief       ネゴシエーション開始を親に送信する
 * @return      送信に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_HANDLE_RequestNegotiation(void);

#endif //__NET_HANDLE_H__



