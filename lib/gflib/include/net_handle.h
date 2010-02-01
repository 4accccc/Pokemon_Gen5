#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	net_handle.h
 * @brief	通信ハンドルの関数
 * @author  GAME FREAK Inc. k.ohno
 * @date    2008.2.26
 */
//=============================================================================
#pragma once

typedef struct _GFL_NETHANDLE2 GFL_NETHANDLE2;


typedef void (FUNC2)(GFL_NETHANDLE2* handle);


///<  @brief  ネットワークハンドル
struct _GFL_NETHANDLE2{
  void* pInfomation;       ///<ユーザー同士が交換するデータのポインタ
  FUNC2* pCallback;
};



typedef struct{
	u8 child_netid;				    ///< 切断するID
  u8 bSend;  ///送ったかどうか
} NEGOTIATION_RESET_DATA;

///<  @brief  ネットワークハンドル
struct _GFL_NETHANDLE{
  void* pInfomation;       ///<ユーザー同士が交換するデータのポインタ
  int infomationSize;      ///< ユーザー同士が交換するデータのサイズ
  void* negoBuff[GFL_NET_HANDLE_MAX];       ///< 子機からの受け取り記録
  NEGOTIATION_RESET_DATA aResetBuff[GFL_NET_HANDLE_MAX];
  u16 negotiationBit;  ///< 他の接続を許すかどうか 0なら許可
  u16 negotiationReturn;  ///< 他の接続を許すかどうか 0なら許可
  u16 timingSyncMy;     ///< 自分が送ったNO
  u16 timingRecv;       ///< 通信で受け取った番号
  u16 timingSyncEnd;    ///< 同期コマンド用
  u16 timingR[(GFL_NET_HANDLE_MAX / 4) * 4];       ///< 子機からの受け取り記録padding
  u8 timingSendFlg;      ///< 送ったかどうか
  u8 negotiationType;        ///< 接続しているハンドルの状態
  u8 negotiationSendFlg;      ///< 送ったかどうか
  u8 serviceNo;              ///< 通信サービス番号
  u8 version;                ///< 通信相手の通信バージョン番号
  u8 bSendInfomation;   ///< インフォメーションデータを送るフラグ
  u8 dummy[2];
};


/// @brief  通信ツールワーク
typedef struct _NET_TOOLSYS_t NET_TOOLSYS;


typedef enum {
  GFL_NET_NEG_TYPE_FIRST,
  GFL_NET_NEG_TYPE_SEND_BEFORE,
  GFL_NET_NEG_TYPE_SEND,
  GFL_NET_NEG_TYPE_SEND_AFTER,
  GFL_NET_NEG_TYPE_RECV,
  GFL_NET_NEG_TYPE_CONNECT,       ///< 接続を親が認証した場合 CONNECTになる このコマンド後は通信可能になる
  GFL_NET_NEG_TYPE_END,           ///< 切断状態
  GFL_NET_NEG_TYPE_TYPE_ERROR,           ///< エラーである
  GFL_NET_NEG_TYPE_NONE,

} GFL_NET_NEGOTIATION_TYPE_E;

//==============================================================================
/**
 * @brief   通信ハンドルを初期化する
 * @param   netID    通信ID
 * @return  GFL_NETHANDLE  通信ハンドルのポインタ
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_InitHandle(int netID);

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
 * @brief   タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @param   networkDefId   識別ID
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_TimeSyncStart(GFL_NETHANDLE* pNet, const u8 no, const u8 networkDefId);

//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   pNet   ネットワークハンドル
 * @param   no     検査する番号
 * @param   networkDefId   識別ID
 * @retval  TRUE   届いていた
 * @retval  FALSE  届いていない
 */
//==============================================================================
extern BOOL GFL_NET_HANDLE_IsTimeSync(const GFL_NETHANDLE* pNet, const u8 no, const u8 networkDefId);

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
 * @brief       ネゴシエーション開始を親に送信する
 * @return      送信に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_HANDLE_RequestNegotiation(void);
//==============================================================================
/**
 * @brief       idに沿ったnetHandleを出す
 * @param       netID    id
 * @return      netHandle
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_GetNetHandle(int netID);
//==============================================================================
/**
 * @brief       接続ハンドルを消す  通信内部で通信切断時に使う
 * @param       netID  ただしサーバのは受け付けない
 * @return      none
 */
//==============================================================================
extern void GFI_NET_HANDLE_Delete(int netID);
//==============================================================================
/**
 * @brief   インフォメーション用コールバック GFL_NET_CMD_INFOMATION
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_RecvInfomation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//-------------------------------------------------------------------------
/**
 * @brief   GFL_NETHANDLEのインフォメーションを得る
 * @param   GFL_NETHANDLE
 * @retval  INFOMATIONデータ
 */
//-------------------------------------------------------------------------
extern void* GFL_NET_HANDLE_GetInfomationData(const GFL_NETHANDLE* pNetHandle);

//-------------------------------------------------------------------------
/**
 * @brief   GFL_NETHANDLEのインフォメーションサイズを得る
 * @param   GFL_NETHANDLE
 * @retval  INFOMATIONサイズ
 */
//-------------------------------------------------------------------------
extern int GFL_NET_HANDLE_GetInfomationDataSize(const GFL_NETHANDLE* pNetHandle);
//------------------------------------------------------------------------------
/**
 * @brief       現在のネゴシエーション結果をえる
 * @param[in]   pHandle   通信ハンドル
 * @return      GFL_NET_NEGOTIATION_TYPE_E
 */
//------------------------------------------------------------------------------
extern GFL_NET_NEGOTIATION_TYPE_E GFL_NET_HANDLE_GetNegotiationType(GFL_NETHANDLE* pHandle);

//==============================================================================
/**
 * @brief       ネゴシエーションを解消する事を子機に送信する
 * @param[in]   通信解消するID
 * @return      none
 */
//==============================================================================
extern void GFL_NET_HANDLE_RequestResetNegotiation(int netID);

//==============================================================================
/**
 * @brief   ネゴシエーション用リセットコールバック GFL_NET_CMD_NEGOTIATION_RESET
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_HANDLE_RecvNegotiationReset(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


#ifdef __cplusplus
} /* extern "C" */
#endif
