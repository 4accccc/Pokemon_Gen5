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

// 関数切り出し自動生成 funccut.rb  k.ohno 2006.12.5 
//==============================================================================
/**
 * @brief   通信デバイスを初期化する
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateDeviceInitialize(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   マックアドレスを指定して子機接続開始
 * @param   connectIndex 接続する親機のIndex
 * @param   bAlloc       メモリーの確保
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateConnectMacAddress(GFL_NETHANDLE* pNetHandle,BOOL bInit);
//==============================================================================
/**
 * @brief   子機開始  ビーコンの収集に入る
 * @param   connectIndex 接続する親機のIndex
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateBeaconScan(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   通信管理ステートの処理
 * @param
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateMainProc(GFL_NETHANDLE* pHandle);
//==============================================================================
/**
 * @brief   親としての通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateCreateParent(GFL_NETHANDLE* pNetHandle,HEAPID heapID);
extern void GFL_NET_StateConnectParent(GFL_NETHANDLE* pNetHandle,HEAPID heapID);
//==============================================================================
/**
 * @brief   親子自動切り替え通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateChangeoverConnect(GFL_NETHANDLE* pNetHandle,HEAPID heapID);
//==============================================================================
/**
 * @brief   終了コマンド 子機が親機にやめるように送信  全員の子機に送り返すGFL_NET_CMD_EXIT_REQ
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateRecvExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   終了コマンド   子機が親機にやめるように送信 ぶっつりきる GFL_NET_CMD_EXIT
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateRecvExitStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック CS_COMM_NEGOTIATION
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateRecvNegotiation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック CS_COMM_NEGOTIATION_RETURN
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateRecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   通信処理終了手続き開始
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateExit(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   DSMPモード変更のMain関数
 * @param   pNet  通信ハンドル
 * @return  noen
 */
//==============================================================================
extern void GFL_NET_StateTransmissonMain(GFL_NETHANDLE* pNet);
//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え  GFL_NET_CMD_DSMP_CHANGE
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_StateRecvDSMPChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_StateRecvDSMPChangeReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え 終了処理 GFL_NET_CMD_DSMP_CHANGE_END
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_StateRecvDSMPChangeEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WIFI指定接続を開始する
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target    つなぎにいく親機の番号
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_StateStartWifiPeerMatch( GFL_NETHANDLE* pNetHandle, int target );
//==============================================================================
/**
 * @brief   WIFIランダムマッチを開始する
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_StateStartWifiRandomMatch( GFL_NETHANDLE* pNetHandle );
//==============================================================================
/**
 * @brief   親としての通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateConnectWifiParent(GFL_NETHANDLE* pNetHandle,HEAPID heapID);
//==============================================================================
/**
 * @brief   マッチングが完了したかどうかを判定
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target: 負なら親、０以上ならつなぎにいく親機の番号
 * @retval  1. 完了　　0. 接続中   2. エラーやキャンセルで中断
 */
//==============================================================================
extern int GFL_NET_StateWifiIsMatched(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理
 * @param   bAuto  自分から強制切断するときはFALSE 接続状況にあわせる場合TRUE
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiMatchEnd(BOOL bAuto, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WIFI終了コマンド   子機が親機にやめるように送信 ぶっつりきる CS_WIFI_EXIT
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateRecvWifiExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理 切断コマンドを送信
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateSendWifiMatchEnd(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   Wifi切断コマンドを受け取った場合TRUE
 * @param   none
 * @retval  WifiLogoutに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiDisconnect(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WifiLoginに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiLoginState(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WifiLoginMatchに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginMatchに移行した場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiLoginMatchState(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WifiErrorの場合その番号を返す  エラーになってるかどうかを確認してから引き出すこと
 * @param   none
 * @retval  Error番号
 */
//==============================================================================
extern int GFL_NET_StateGetWifiErrorNo(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WIFIでログアウトを行う場合の処
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiLogout(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   WifiErrorかどうか
 * @param   none
 * @retval  Error
 */
//==============================================================================
extern BOOL GFL_NET_StateIsWifiError(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   wifiマッチング状態へログインする
 * @param   pNetHandle  GFL_NETHANDLE
 * @param   netHeapID   netLibで確保使用するID
 * @param   wifiHeapID  NitroWifi&NitroDWC&VCTで確保するメモリ
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_StateWifiEnterLogin(GFL_NETHANDLE* pNetHandle, HEAPID netHeapID, HEAPID wifiHeapID);
//--------------------------errfunc------------------------------------------------

//==============================================================================
/**
 * @brief   自動的にエラー検出を行うかどうか
 *          (TRUEの場合エラーをみつけるとブルースクリーン＋再起動になる)
 * @param   bAuto  TRUEで検査開始 FALSEでエラー処理を行わない
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_STATE_SetAutoErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bAuto);
//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_STATE_SetNoChildErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bFlg);


#define _ERR_DEFINE  (0)
#if _ERR_DEFINE

//==============================================================================
/**
 * @brief   ここから先エラーの検査を通信が処理するかどうかを設定
 * @param   bFlg  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
extern BOOL CommStateGetErrorCheck(GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * @brief   エラー状態に入る
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void CommSetErrorReset(GFL_NETHANDLE* pNetHandle,u8 type);
//==============================================================================
/**
 * @brief   リセットエラー状態になったかどうか確認する
 * @param   none
 * @retval  リセットエラー状態ならTRUE
 */
//==============================================================================
extern u8 CommIsResetError(GFL_NETHANDLE* pNetHandle);
//--------------------------------------------------------------
/**
 * @brief   fatalエラー用関数  この関数内から抜けられない
 * @param   no   エラー番号
 * @retval  none
 */
//--------------------------------------------------------------
extern void CommFatalErrorFunc(int no);
//--------------------------------------------------------------
/**
 * @brief   エラーを感知して 通信エラー用ウインドウを出す
 * @param   heapID    メモリー確保するHEAPID
 * @param   bgl       GF_BGL_INI
 * @retval  none
 */
//--------------------------------------------------------------
//extern void CommErrorCheck(int heapID, GF_BGL_INI* bgl);
//--------------------------------------------------------------
/**
 * @brief   エラーを感知して 通信エラー用ウインドウを出す
 * @param   heapID    メモリー確保するHEAPID
 * @param   bgl       GF_BGL_INI
 * @retval  none
 */
//--------------------------------------------------------------
extern void CommErrorDispCheck(GFL_NETHANDLE* pNetHandle,int heapID);
//--------------------------------------------------------------
/**
 * @brief   リセットできる状態かどうか
 * @param   none
 * @retval  TRUE ならリセット
 */
//--------------------------------------------------------------
extern BOOL CommStateIsResetEnd(void);
//==============================================================================
/**
 * @brief   エラーにする場合この関数を呼ぶとエラーになります
 * @param   エラー種類
 * @retval  受け付けた場合TRUE
 */
//==============================================================================
extern BOOL CommStateSetError(GFL_NETHANDLE* pNetHandle,int no);


#define COMM_ERROR_RESET_SAVEPOINT  (1)
#define COMM_ERROR_RESET_TITLE  (2)
#define COMM_ERROR_RESET_GTS  (3)
#define COMM_ERROR_RESET_OTHER (4)


#define COMM_ERRORTYPE_ARESET (0)     // Aボタンでリセット
#define COMM_ERRORTYPE_POWEROFF (1)   // 電源を切らないといけない
#define COMM_ERRORTYPE_TITLE (2)     // Aボタンでリセット タイトル戻し
#define COMM_ERRORTYPE_GTS (3)     // Aボタンでリセット セーブポイントへ戻す

#endif  //_ERR_DEFINE

#endif //__NET_STATE_H__

