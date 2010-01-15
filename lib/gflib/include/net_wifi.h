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
extern void GFL_NET_DWC_StartVChat(void);
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
 * エラーコードを保持する
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateSetWifiError(int code, int type, int ret);

//==============================================================================
/**
 * WifiErrorのエラーをクリアする。軽度のエラーを解消するためのもの。
 * 重度なエラーは切断やシャットダウン等をおこない解消すること！
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateClearWifiError( void );

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


#define GFL_NET_STATE_MATCHING (0)
#define GFL_NET_STATE_MATCHED (1)
#define GFL_NET_STATE_NOTMATCH (2)
#define GFL_NET_STATE_TIMEOUT (3)
#define GFL_NET_STATE_DISCONNECTING (4)
#define GFL_NET_STATE_FAIL (5)

//==============================================================================
/**
 * WIFIのステート遷移を得る
 * @brief   マッチングが完了したかどうかを判定
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target: 負なら親、０以上ならつなぎにいく親機の番号
 * @retval  1. 完了　　0. 接続中   2. エラーやキャンセルで中断
 */
//==============================================================================
extern int GFL_NET_StateGetWifiStatus(void);

//==============================================================================
/**
 * WifiErrorかどうか
 * @param   none
 * @retval  Error
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiError(void);
//==============================================================================
/**
 * @brief   Wifi切断コマンドを受け取った場合TRUE
 * @param   none
 * @retval  WifiLogoutに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiDisconnect(void);
//==============================================================================
/**
 * @brief   WIFI指定接続を開始する
 * @param   target    つなぎにいく親機の番号
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_StateStartWifiPeerMatch( int target );
//==============================================================================
/**
 * @brief   WIFIランダムマッチを開始する
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_StateStartWifiRandomMatch( void);
//==============================================================================
/**
 * @brief   WIFIランダムマッチを開始する
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_StateStartWifiRandomMatch_RateMode(void );
//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理
 * @param   bAuto  自分から強制切断するときはFALSE 接続状況にあわせる場合TRUE
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiMatchEnd(BOOL bAuto);
//==============================================================================
/**
 * @brief   wifiマッチング状態へログインする
 * @param   netHeapID   netLibで確保使用するID
 * @param   wifiHeapID  NitroWifi&NitroDWC&VCTで確保するメモリ
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiEnterLogin(void);
//==============================================================================
/**
 * @brief   WIFIでログアウトを行う場合の処
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiLogout(NetEndCallback netEndCallback);
//----------------------------------------------------------------------------
/**
 *	@brief	セーブして良いかどうか確認するフラグ
 *	@param	TRUE セーブしてよい
 */
//-----------------------------------------------------------------------------
extern u8 GFL_NET_DWC_GetSaving(void);
//==============================================================================
/**
 * @brief   WifiLoginに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiLoginState(void);
//==============================================================================
/**
 * @brief   WifiErrorかどうか
 * @param   none
 * @retval  Error
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiError(void);
//==============================================================================
/**
 * @brief   Wifi切断コマンドを受け取った場合TRUE
 * @param   none
 * @retval  WifiLogoutに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiDisconnect(void);
//==============================================================================
/**
 * @brief   WifiLoginMatchに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginMatchに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiLoginMatchState(void);
//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理 切断コマンドを送信
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateSendWifiMatchEnd(GFL_NETHANDLE* pNet);

//----------------------------------------------------------------------------
/**
 *	@brief	WiFiロビー専用	ログイン関数
 *
 *	@param	p_save				セーブデータ
 *	@param	cp_initprofile		初期化プロフィールデータ(ポインタを保持する為、実体は外側で解放しないように)
 */
//-----------------------------------------------------------------------------
extern void GFL_NET_StateWifiLobbyLogin( const void* cp_initprofile );
#ifdef PM_DEBUG
extern void GFL_NET_StateWifiLobbyLogin_Debug( const void* cp_initprofile, u32 season, u32 room );
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	WiFiロビーのログアウト
 */
//-----------------------------------------------------------------------------
extern void GFL_NET_StateWifiLobbyLogout( void );

//----------------------------------------------------------------------------
/**
 *	@brief	DWC_LoginAsyncのログインまでが完了したかチェック
 *	@retval	TRUE	した
 *	@retval	FALSE	してない
 */
//-----------------------------------------------------------------------------
extern BOOL GFL_NET_StateWifiLobbyDwcLoginCheck( void );

//----------------------------------------------------------------------------
/**
 *	@brief	WiFiロビーミニゲーム開始
 *
 *	@param	type		ゲームタイプ(DWC_LOBBY_MG_TYPE)
 */
//-----------------------------------------------------------------------------
extern void GFL_NET_StateWifiP2PStart( int type );

//----------------------------------------------------------------------------
/**
 *	@brief	WiFiロビーミニゲーム終了
 *			P2P切断チェックは、GFL_NET_StateWifiP2PGetConnectStateで行ってください
 */
//-----------------------------------------------------------------------------
extern void GFL_NET_StateWifiP2PEnd( void );



/// WiFiで使うHeapのサイズ
#define GFL_NET_DWC_HEAPSIZE        (0x45400)   //WIFIクラブのHEAP量
#define GFL_NET_DWCLOBBY_HEAPSIZE   (0x60000)   //WIFI広場のHEAP量


// 受信コールバック型。WHReceiverFuncと同形
typedef void (*GFL_NET_MYDWCReceiverFunc) (u16 aid, u16 *data, u16 size);

//接続検査 フレンドコードリストインデックス
typedef BOOL (*GFL_NET_MYDWCConnectModeCheckFunc) (int index,void* pWork);

#define DWCRAP_STARTGAME_OK (0)
#define DWCRAP_STARTGAME_NOTSTATE  (-1)
#define DWCRAP_STARTGAME_RETRY  (-2)
#define DWCRAP_STARTGAME_FAILED  (-3)
#define DWCRAP_STARTGAME_FIRSTSAVE (-4)

#define STEPMATCH_CONTINUE 0

#define __ERROR_FRIENDS_SHORTAGE_DWC  (10)  ///< @todo 強引な結びつけ dev_wifiで確認している状態

#define STEPMATCH_SUCCESS  (1000)    //昔はDWC_ERROR_NUM
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
extern void* GFI_NET_GetMyDWCUserData(void);  //DWCUserData
extern void* GFI_NET_GetMyDWCFriendData(void);//DWCFriendData
extern int GFI_NET_GetFriendNumMax(void);

#endif //GFL_NET_WIFI


#endif //__NET_WIFI_H__
#ifdef __cplusplus
} /* extern "C" */
#endif
