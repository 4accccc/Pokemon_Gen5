//=============================================================================
/**
 * @file	network.h
 * @brief	通信ライブラリの外部公開関数
 * @author	k.ohno
 * @date    2006.11.4
 */
//=============================================================================
#ifndef _NETWORK_H_
#define _NETWORK_H_

// define 
#define NET_NETID_ALLUSER (0xff)  ///< NetID:全員へ送信する場合
#define NET_NETID_SERVER (0xfe)   ///< NetID:サーバーの場合これ 後は0からClientID

// typedef
typedef struct u8 GameServiceID;  ///< ゲームサービスID  通信の種類
typedef struct u8 ConnectID;      ///< 接続するためのID  0-16 まで
typedef struct u8 NetID;          ///< 通信ID  0-200 まで

/// 通信管理構造体
typedef struct _GFNetHandle GFNetHandle;

/// 送信完了を受け取るコールバック型
typedef CBSendEndFunc;



typedef struct{             ///< 通信の初期化用構造体
  GameServiceID gsid;       ///< ゲームサービスID  通信の種類
  u32  allocNo;             ///< allocするための番号
  NetRecvFuncTable *pRecvFuncTable;  ///< 受信関数テーブル
  NetBeaconGetFunc *pBeaconGetFunc;  ///< ビーコンデータ取得関数
  NetErrorFunc *pErrorFunc;           ///< 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NetFatalErrorFunc *pFatalErrorFunc; ///< 緊急停止エラーが起こった場合呼ばれる 切断するしかない
  NetConnectEndFunc *pConnectEndFunc;  ///< 通信切断時に呼ばれる関数
  NetDSMPSwitchEndFunc *pSDMPSwitchEndFunc;  ///< DS<>MP切り替え完了時に呼ばれる
  u8 maxConnectNum;   ///< 最大接続人数
  u8 maxBeaconNum;    ///< 最大ビーコン収集数

} GFNetInitializeStruct;

//-------------------------------
//  関数 ～外部公開関数は全てここに定義
//-------------------------------

//==============================================================================
/**
 * 通信初期化
 * @param[in]   NetInitializeStruct*  pNetInit  通信初期化構造体のポインタ
 * @retval  void
 */
//==============================================================================
extern void GF_NT_Initialize(const NetInitializeStruct* pNetInit);

//==============================================================================
/**
 * 通信終了
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @retval  void
 */
//==============================================================================
extern void GF_NT_Finalize(NetHandle* pNet);

//-----ビーコン・ともだち情報関連
//==============================================================================
/**
 * ビーコンを受け取るサービスを追加指定  （いくつかのサービスのビーコンを拾いたい時に）
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  none
 */
//==============================================================================
extern void GF_NT_AddBeaconServiceID(NetHandle* pNet, GameServiceID gsid);

//==============================================================================
/**
 * 受信ビーコンデータを得る
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
extern void* GF_NT_GetBeaconData(NetHandle* pNet, ConnectID id);


//--------接続・切断
//==============================================================================
/**
 * 子機になり接続する
 * @retval  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
extern NetHandle* pNet = GF_NT_ClientConnect(void);

//==============================================================================
/**
 * 親機になり待ち受ける
 * @return  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
extern NetHandle* pNet = GF_NT_ServerConnect(void);

//==============================================================================
/**
 * 親子切り替え接続を行う
 * @return  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
extern NetHandle* pNet = GF_NT_SwitchConnect(void);

//==============================================================================
/**
 * このハンドルの通信番号を得る
 * @param[in,out]  NetHandle* pNet     通信ハンドルのポインタ
 * @retval  NetID     通信ID
 */
//==============================================================================
extern NetID GF_NT_GetMyNetID(NetHandle* pNet);

//==============================================================================
/**
 * 現在の接続人数を得る
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @retval  int  接続数
 */
//==============================================================================
extern int GF_NT_GetConnectNum(NetHandle* pNet);

//==============================================================================
/**
 * IDの通信と接続しているかどうかを返す
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetID id            ネットID
 * @retval  BOOL  接続していたらTRUE
 */
//==============================================================================
extern BOOL GF_NT_IsConnectMember(NetHandle* pNet,NetID id);

//==============================================================================
/**
 * 通信切断する
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @retval  none
 */
//==============================================================================
//
extern void GF_NT_Disconnect(NetHandle* pNet);

//==============================================================================
/**
 * 接続中かどうか
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @retval  none
 */
//==============================================================================
extern BOOL GF_NT_IsConnect(NetHandle* pNet);

//==============================================================================
/**
 * 接続数変更
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   u8 num     変更数
 * @retval  none
 */
//==============================================================================
extern void GF_NT_SetClientConnectNum(NetHandle* pNet,u8 num);

//==============================================================================
/**
 * 新規接続禁止＆許可を設定
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   BOOL bEnable     TRUE=接続許可 FALSE=禁止
 * @retval  none
 */
//==============================================================================
extern void GF_NT_SetClientConnect(NetHandle* pNet,BOOL bEnable);


//--------送信

//==============================================================================
/**
 * 送信開始
 * @brief 全員に無条件で送信  送信サイズは初期化時のテーブルから引き出す
 *        データは必ずコピーする
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u16 sendCommand                送信するコマンド
 * @param[in]   void* data                     送信データポインタ
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GF_NT_SendData(NetHandle* pNet,const u16 sendCommand,const void* data);

//==============================================================================
/**
 * 送信開始
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u16 sendCommand                送信するコマンド
 * @param[in]   NetID sendID                     送信相手 全員へ送信する場合 NET_SENDID_ALLUSER
 * @param[in]   CBSendEndFunc* pCBSendEndFunc  送信完了をつたえるコールバック関数の登録
 * @param[in]   u32 size                       送信データサイズ
 * @param[in]   void* data                     送信データポインタ
 * @param[in]   BOOL bDataCopy                 データをコピーする場合TRUE
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
extern BOOL GF_NT_SendData(NetHandle* pNet,const NetID sendID,
                    const CBSendEndFunc* pCBSendEndFunc,const u32 size,
                    const void* data, const BOOL bDataCopy);

//==============================================================================
/**
 * 送信データが無いかどうか
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   無い場合
 * @retval  FALSE  残っている場合
 */
//==============================================================================
extern BOOL GF_NT_IsEmptySendData(NetHandle* pNet);

//--------特殊
// 
//==============================================================================
/**
 * 同期通信に切り替えたり親子通信に切り替えたりする
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   BOOL bDSSwitch  TRUE=DSに切り替える FALSE=MPに切り替える
 * @return  none
 */
//==============================================================================
extern void GF_NT_DSMPSwitch(NetHandle* pNet, const BOOL bDSSwitch);


//--------その他、ツール類
//==============================================================================
/**
 * タイミングコマンドを発行する
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u8 no   タイミング取りたい番号
 * @return      none
 */
//==============================================================================
extern void GF_NT_TimingSyncStart(NetHandle* pNet, const u8 no);
//==============================================================================
/**
 * タイミングコマンドが届いたかどうかを確認する
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   no   届く番号
 * @retval  TRUE    タイミングが合致
 * @retval  FALSE   タイミングがそろっていない
 */
//==============================================================================
extern BOOL GF_NT_IsTimingSync(NetHandle* pNet, const u8 no);


#endif //_NETWORK_H_

