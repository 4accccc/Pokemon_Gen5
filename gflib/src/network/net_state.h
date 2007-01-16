//=============================================================================
/**
 * @file	net_state.h
 * @brief	通信状態を管理するサービス  通信の上位にある
 *          スレッドのひとつとして働き、自分の通信状態や他の機器の
 *          開始や終了を管理する
 *          communication部分のみを再移動
 * @author	k.ohno
 * @date    2006.01.12
 */
//=============================================================================

#ifndef __NET_STATE_H__
#define __NET_STATE_H__


//==============================================================================
// extern宣言
//==============================================================================
#ifdef PM_DEBUG
extern void DebugOhnoCommDebugUnderNo(int no);
extern void DebugOhnoCommDebugUnderParentOnly(int no);
extern void DebugOhnoCommDebugUnderChildOnly(int no);
#endif


extern void GFL_NET_StateDeviceInitialize(GFL_NETHANDLE* pNetHandle);

extern void GFL_NET_StateConnectMacAddress(GFL_NETHANDLE* pNetHandle);
extern void GFL_NET_StateConnectParent(GFL_NETHANDLE* pNetHandle,int heapID);
extern void GFL_NET_StateRecvExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
extern void GFL_NET_StateRecvNegotiation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
extern void GFL_NET_StateRecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
extern void GFL_NET_StateExit(GFL_NETHANDLE* pNetHandle);


extern BOOL CommStateIsInitialize(void);

// バトル時に親になる場合の処理開始
extern void CommStateEnterBattleParent(int serviceNo, int regulationNo, BOOL bWifi);
// バトル時に子になる場合の処理開始
extern void CommStateEnterBattleChild(int serviceNo, int regulationNo, BOOL bWifi);
// バトル時に親を決めた際の子機の処理
extern void CommStateConnectBattleChild(int connectIndex);
// バトル時に子機を再起動する時の処理
extern void CommStateRebootBattleChild(void);
// 終了処理手続き
extern void CommStateExitBattle(void);
// バトル接続中STATEなのかどうか返す
extern BOOL CommIsBattleConnectingState(void);






extern void CommStateSetPokemon(u8* sel);
// コピー
extern void CommStateGetPokemon(u8* sel);




/// 接続確認検査
extern void CommRecvNegotiation(const int netID, const int size, const void* pData, void* pWork,GFL_NETHANDLE* pNetHandle);
/// 接続確認検査 親機からの返事
extern void CommRecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork,GFL_NETHANDLE* pNetHandle);
/// 接続確認コマンドのサイズ
extern int CommRecvGetNegotiationSize(void);

/// 入ってくる人の数を制限
extern void CommStateSetLimitNum(int num);


/// リセット用の通信切断
extern BOOL CommStateExitReset(void);

extern void CommStateCheckFunc(void);

extern void CommStateSetEntryChildEnable(BOOL bEntry);

extern void CommStateSetErrorCheck(BOOL bFlg,BOOL bAuto);
extern BOOL CommStateGetErrorCheck(void);

extern BOOL CommStateGetWifiDPWError(void);


extern void* CommStateGetMatchWork(void);
extern BOOL CommStateIsWifiLoginState(void);
extern BOOL CommStateIsWifiLoginMatchState(void);
extern void CommStateWifiMatchEnd(void);
extern void CommStateWifiTradeMatchEnd(void);
extern void CommStateWifiBattleMatchEnd(void);
extern BOOL CommStateIsWifiDisconnect(void);

extern int CommWifiWaitingCancel( void );

extern int CommStateGetServiceNo(void);
extern int CommStateGetRegulationNo(void);
#ifdef PM_DEBUG
extern int CommStateGetSoloDebugNo(void);
#endif

extern void CommStateWifiLogout(void);
extern int CommStateGetWifiErrorNo(void);
extern BOOL CommStateIsWifiError(void);

//ゲームを開始する。// 2006.4.13 吉原追加
extern int CommWifiBattleStart( int target );

//マッチングが完了したかどうかを判定 // 2006.4.13 吉原追加
extern int CommWifiIsMatched();

// 通信がWIFI接続しているかどうかを返す
extern BOOL CommStateIsWifiConnect(void);



#define COMM_ERROR_RESET_SAVEPOINT  (1)
#define COMM_ERROR_RESET_TITLE  (2)
#define COMM_ERROR_RESET_GTS  (3)
#define COMM_ERROR_RESET_OTHER (4)


// リセットするエラーが発生した場合に呼ぶ
extern void CommSetErrorReset(u8 type);
// リセットするエラーが発生したかどうか
extern u8 CommIsResetError(void);

// fatalエラー関数
extern void CommFatalErrorFunc(int no);
extern void CommFatalErrorFunc_NoNumber( void );


// 地下はじめのイベントを行う場合設定
extern void CommStateSetFirstEvent(void);

// WIFIから切断要求
extern void CommRecvExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

extern BOOL CommStateIsResetEnd(void);

extern void CommStateSetParentOnlyFlg(BOOL bFlg);


#define COMM_ERRORTYPE_ARESET (0)     // Aボタンでリセット
#define COMM_ERRORTYPE_POWEROFF (1)   // 電源を切らないといけない
#define COMM_ERRORTYPE_TITLE (2)     // Aボタンでリセット タイトル戻し
#define COMM_ERRORTYPE_GTS (3)     // Aボタンでリセット セーブポイントへ戻す

// エラー表示関数
extern void ComErrorWarningResetCall( int heapID, int type, int code );
extern void DWClibWarningCall( int heapID, int error_msg_id );


// エラーにする関数   以下の番号を使用してください
//#define COMM_ERROR_RESET_SAVEPOINT  (1)
//#define COMM_ERROR_RESET_TITLE  (2)
//#define COMM_ERROR_RESET_GTS  (3)
//#define COMM_ERROR_RESET_OTHER (4)
extern BOOL CommStateSetError(int no);


extern void GFL_NET_StateMainProc(GFL_NETHANDLE* pHandle);



#endif //__NET_STATE_H__

