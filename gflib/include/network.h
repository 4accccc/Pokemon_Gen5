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
#define NET_NETID_ALLUSER (0xff)  // NetID:全員へ送信する場合
#define NET_NETID_SERVER (0xfe)   // NetID:サーバーの場合これ 後は0からClientID

// typedef
typedef struct u8 GameServiceID;  // ゲームサービスID  通信の種類
typedef struct u8 ConnectID;      // 接続するためのID  0-16 まで
typedef struct u8 NetID;          // 通信ID  0-200 まで

typedef struct _NetHandle NetHandle; // 通信管理構造体

typedef struct{             // 通信の初期化用構造体
  GameServiceID gsid;       // ゲームサービスID  通信の種類
  u32  allocNo;             // allocするための番号
  NetRecvFuncTable *pRecvFuncTable;  //受信関数テーブル
  NetBeaconGetFunc *pBeaconGetFunc;  //ビーコンデータ取得関数
  u8 maxConnectNum;   //最大接続人数   終わりまで変更不可
  u8 maxBeaconNum;    //最大ビーコン数 終わりまで変更不可

} NetInitializeStruct;

//-------------------------------
//  関数 ～外部公開関数は全てここに定義
//-------------------------------

//==============================================================================
/**
 * 通信初期化
 * @param   NetInitializeStruct   通信初期化構造体のポインタ
 * @retval  void
 */
//==============================================================================
extern void CommInitialize(NetInitializeStruct* pNetInit);

//==============================================================================
/**
 * 通信終了
 * @param   NetHandle* pNet   通信ハンドルのポインタ
 * @retval  void
 */
//==============================================================================
extern void CommFinalize(NetHandle* pNet);

//-----ビーコン・ともだち情報関連
//==============================================================================
/**
 * ビーコンを受け取るサービスを追加指定  （いくつかのサービスのビーコンを拾いたい時に）
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  none
 */
//==============================================================================
extern void CommAddBeaconServiceID(NetHandle* pNet, GameServiceID gsid);

//==============================================================================
/**
 * 受信ビーコンデータを得る
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
extern void* CommGetBeaconData(NetHandle* pNet, ConnectID id);


//--------接続・切断
//==============================================================================
/**
 * 子機になり接続する
 * @retval  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
NetHandle* pNet = CommClientConnect(void);

//==============================================================================
/**
 * 親機になり待ち受ける
 * @retval  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
NetHandle* pNet = CommServerConnect(void);

//==============================================================================
/**
 * 親子切り替え接続を行う
 * @retval  NetHandle* pNet     通信ハンドルのポインタ
 */
//==============================================================================
NetHandle* pNet = CommSwitchConnect(void);

//このハンドルの接続番号を得る
NetID CommGetMyNetID(NetHandle* pNet);

//接続人数()
CommGetConnectNum(NetHandle* pNet);

//接続しているかどうかを返す( id )
CommIsConnectMember(NetHandle* pNet,NetID id);

//切断する
CommDisconnect(NetHandle* pNet);

//接続中かどうか()
CommIsConnect(NetHandle* pNet);
//接続数変更
CommSetClientConnectNum(NetHandle* pNet,u8 num);
//接続禁止&許可
CommSetClientConnect(NetHandle* pNet,BOOL bEnable);


//--------送信

// 送信完了を受け取るコールバック型
typedef CBSendEndFunc

//==============================================================================
/**
 * 送信開始    全員に無条件で送信  送信サイズは初期化時のテーブルから引き出す
 *             データは必ずコピーする
 * @param   NetHandle* pNet                通信ハンドル
 * @param   u16 sendCommand                送信するコマンド
 * @param   void* data                     送信データポインタ
 * @retval  成功したらTRUE 失敗の場合FALSE
 */
//==============================================================================
BOOL CommSendData(NetHandle* pNet,u16 sendCommand,void* data);

//==============================================================================
/**
 * 送信開始
 * @param   NetHandle* pNet                通信ハンドル
 * @param   u16 sendCommand                送信するコマンド
 * @param   NetID sendID                     送信相手 全員へ送信する場合 NET_SENDID_ALLUSER
 * @param   CBSendEndFunc* pCBSendEndFunc  送信完了をつたえるコールバック関数の登録
 * @param   u32 size                       送信データサイズ
 * @param   void* data                     送信データポインタ
 * @param   BOOL bDataCopy                 データをコピーする場合TRUE
 * @retval  成功したらTRUE 失敗の場合FALSE
 */
//==============================================================================
BOOL CommSendData(NetHandle* pNet,NetID sendID,CBSendEndFunc* pCBSendEndFunc,u32 size,void* data, BOOL bDataCopy);

//==============================================================================
/**
 * 送信データが無いかどうか
 * @param   NetHandle* pNet  通信ハンドル
 * @retval  無い場合にTRUE
 */
//==============================================================================
BOOL CommIsEmptySendData(NetHandle* pNet);

//--------特殊
// 同期通信に切り替える<-->親子通信に切り替える


//--------その他、ツール類
//==============================================================================
/**
 * タイミングコマンドを発行する
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================
extern void CommTimingSyncStart(NetHandle* pNet, u8 no);
//==============================================================================
/**
 * タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================
extern BOOL CommIsTimingSync(NetHandle* pNet, u8 no);


#endif //_NETWORK_H_

