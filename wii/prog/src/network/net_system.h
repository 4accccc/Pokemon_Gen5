//=============================================================================
/**
 * @file	net_system.h
 * @brief	通信システム
 * @author	Katsumi Ohno
 * @date    2005.07.08
 */
//=============================================================================

#ifndef _NET_SYSTEM_H_
#define _NET_SYSTEM_H_

#include "net.h"
#include "net_def.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "net_handle.h"


extern BOOL _commSystemInit(int packetSizeMax, HEAPID heapID);
extern void GFL_NET_UpdateSystem( MPDSContext *DSContext );


// 関数切り出し自動生成 funccut.rb  k.ohno 2006.12.5 
//==============================================================================
/**
 * @brief   WiFiGameの初期化を行う
 * @param   packetSizeMax パケットのサイズマックス
 * @param   regulationNo  ゲームの種類
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemWiFiModeInit(int packetSizeMax, HEAPID heapIDSys, HEAPID heapIDWifi);
//==============================================================================
/**
 * @brief   wifiのゲームを開始する
 * @param   target:   負なら親、０以上ならつなぎにいく親機の番号
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
extern int GFL_NET_SystemWifiApplicationStart( int target );
//==============================================================================
/**
 * @brief   IRC通信システムの初期化を行う
 * @param   bTGIDChange   新規のゲームの初期化の場合TRUE 古いビーコンでの誤動作を防ぐため用
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIrcModeInit(BOOL bChangeTGID);
//==============================================================================
/**
 * @brief   親機の初期化を行う
 * @param   work_area     システムで使うメモリー領域
 *                        NULLの場合すでに初期化済みとして動作
 * @param   bTGIDChange   新規のゲームの初期化の場合TRUE 古いビーコンでの誤動作を防ぐため用
 * @param   packetSizeMax パケットサイズ
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFI_NET_SystemParentModeInit(BOOL bTGIDChange, int packetSizeMax);
//==============================================================================
/**
 * @brief   親機の確立を行う
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFI_NET_SystemParentModeInitProcess(BOOL channelChange);
//==============================================================================
/**
 * @brief   子機の初期化を行う
 * @param   work_area 　システムで使うメモリー領域
 *                      NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemChildModeInit(BOOL bBconInit, int packetSizeMax);
//==============================================================================
/**
 * @brief   子機の初期化を行う+ 接続にすぐ行く
 * @param   work_area 　システムで使うメモリー領域
 *                      NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemChildModeInitAndConnect(BOOL bInit,u8* pMacAddr,int packetSizeMax,_PARENTFIND_CALLBACK pCallback);
//==============================================================================
/**
 * @brief   通信切断を行う
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemFinalize(void);
//==============================================================================
/**
 * @brief   子機 index接続
 * @param   index   親のリストのindex
 * @retval  子機接続を親機に送ったらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemChildIndexConnect(u16 index,_PARENTFIND_CALLBACK pCallback,GFL_NETHANDLE* pHandle);
//==============================================================================
/**
 * @brief   通信データの更新処理  データを収集
 *    main.c   から  vblank後にすぐに呼ばれる
 * @param   none
 * @retval  データシェアリング同期が取れなかった場合FALSE
 */
//==============================================================================
extern BOOL GFL_NET_SystemUpdateData(void);
//==============================================================================
/**
 * @brief   通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemReset(void);
//==============================================================================
/**
 * @brief   DSモードで通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemResetDS(void);
//==============================================================================
/**
 * @brief   通信バッファをクリアーする+ビーコンの初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemResetBattleChild(void);
//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemRecvCallback(u16 aid, u16 *data, u16 size);
//==============================================================================
/**
 * 通信を受信した時に呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemRecvParentCallback(u16 aid, u16 *data, u16 size);
//==============================================================================
/**
 * @brief   子機側のアクションが行われたことを設定
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemActionCommandSet(void);
//==============================================================================
/**
 * 子機送信メソッド
 * 親機がデータを子機全員に送信するのは別関数
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   size       送信量    コマンドだけの場合0
 * @param   bFast       送信量    コマンドだけの場合0
 * @param   myID       送信する人のハンドルno
 * @param   sendID     送る相手のID   全員なら0xff
 * @param   bSendBuffLock 送信バッファを固定して大容量データを送信する
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================
extern BOOL GFL_NET_SystemSendData(int command, const void* data, int size, BOOL bFast, int myID, int sendID, BOOL bSendBuffLock);
//==============================================================================
/**
 * 送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================
extern int GFL_NET_SystemGetSendRestSize(void);
//==============================================================================
/**
 * サーバ側の送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================
extern int GFL_NET_SystemGetSendRestSize_ServerSide(void);
//==============================================================================
/**
 * 通信可能状態なのかどうかを返す ただしコマンドによるネゴシエーションがまだの状態
 * @param   親子機のnetID
 * @retval  TRUE  通信可能    FALSE 通信切断
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsConnect(u16 netID);
//==============================================================================
/**
 * 通信可能状態の人数を返す
 * @param   none
 * @retval  接続人数
 */
//==============================================================================
extern int GFL_NET_SystemGetConnectNum(void);
//==============================================================================
/**
 * 初期化しているかどうかを返す
 * @param   none
 * @retval  初期が終わっていたらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsInitialize(void);
//==============================================================================
/**
 * サーバー側から子機に送る場合 送信キューへの追加
 * @param   command    comm_sharing.hに定義したラベル
 * @param   sendNetID
 * @param   data       送信したいデータ ない時はNULL
 *                     このデータは静的でなければならない  バッファに溜めないため
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================
//extern BOOL GFL_NET_SystemSetSendQueue_ServerSide(int command, const void* data, int size);
//==============================================================================
/**
 * クライアント側から親機に送る場合 送信キューへの追加
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 *                     このデータは静的でなければならない  バッファに溜めないため
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================
extern BOOL GFL_NET_SystemSetSendQueue(int command, const void* data, int size);
//==============================================================================
/**
 * DS通信MP通信の切り替え  GFL_NET_CMD_DSMP_CHANGE
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_SystemRecvDSMPChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * DS通信MP通信の切り替え
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_SystemRecvDSMPChangeReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * DS通信MP通信の切り替え 終了処理 GFL_NET_CMD_DSMP_CHANGE_END
 * @param   none
 * @retval  残り数
 */
//==============================================================================
extern void GFL_NET_SystemRecvDSMPChangeEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * 自分の機のIDを返す
 * @param   
 * @retval  自分の機のID  つながっていない場合COMM_PARENT_ID
 */
//==============================================================================
extern u16 GFL_NET_SystemGetCurrentID(void);
//==============================================================================
/**
 * WHライブラリで　通信状態のBITを確認
 * @param   none
 * @retval  接続がわかるBIT配列
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsChildsConnecting(void);
//==============================================================================
/**
 * エラー状態かどうか
 * @param   none
 * @retval  エラーの時TRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsError(void);
//==============================================================================
/**
 * 一人通信モードを設定
 * @param   bAlone    一人通信モード
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemSetAloneMode(BOOL bAlone);
//==============================================================================
/**
 * 一人通信モードかどうかを取得
 * @param   none
 * @retval  一人通信モードの場合TRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemGetAloneMode(void);
//==============================================================================
/**
 * 自動終了コマンド受信
 * @param   netID  通信ID
 * @param   size   サイズ
 * @param   pData  データ
 * @param   pWork  ワーク
 * @return  none
 */
//==============================================================================
extern void GFL_NET_SystemRecvAutoExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
//==============================================================================
/**
 * デバッグ用にダンプを表示する
 * @param   adr           表示したいアドレス
 * @param   length        長さ
 * @param   pInfoStr      表示したいメッセージ
 * @retval  サービス番号
 */
//==============================================================================
extern void GFL_NET_SystemDump_Debug(u8* adr, int length, char* pInfoStr);
//==============================================================================
/**
 * 特定のコマンドを送信し終えたかどうかを調べる クライアント側
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsSendCommand(int command, int myID);
//==============================================================================
/**
 * キューが空っぽかどうか サーバー側
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsEmptyQueue_ServerSize(void);
//==============================================================================
/**
 * キューが空っぽかどうか
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsEmptyQueue(void);
//==============================================================================
/**
 * wifi接続したかどうかを設定する
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================
extern void GFL_NET_SystemSetWifiConnect(BOOL bConnect);
//==============================================================================
/**
 * 戦闘に入る前の敵味方の立ち位置を設定
 * @param   no   立っていた位置の番号に直したもの
 * @param   netID   通信のID
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemSetStandNo(int no,int netID);
//==============================================================================
/**
 * 戦闘に入る前の敵味方の立ち位置を得る
 * @param   netID 通信のID
 * @retval  立っていた位置の番号に直したもの  0-3  0,2 vs 1,3
 */
//==============================================================================
extern int GFL_NET_SystemGetStandNo(int netID);
//==============================================================================
/**
 * VCHAT状態かどうか
 * @param   none
 * @retval  VCHAT状態ならTRUE
 */
//==============================================================================
extern BOOL GFL_NET_SystemIsVChat(void);
//==============================================================================
/**
 * WIFI通信を同期通信するか、非同期通信するかの切り替えを行う
 * @param   TRUE 同期 FALSE 非同期
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemSetWifiBothNet(BOOL bFlg);
//==============================================================================
/**
 * エラーにする場合TRUE
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemSetError(void);
//==============================================================================
/**
 * 通信を止める
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemShutdown(void);
//==============================================================================
/**
 * サーバー側の通信キューをリセットする
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemResetQueue_Server(void);
//==============================================================================
/**
 * 通信の受信を止めるフラグをセット
 * @param   bFlg  TRUEで止める  FALSEで許可
 * @retval  none
 */
//==============================================================================
extern void GFL_NET_SystemRecvStop(BOOL bFlg);
//==============================================================================
/**
 * @brief   キーシェアリング機能でキーを得る
 * @retval  TRUE  データシェアリング成功
 * @retval  FALSE データシェアリング失敗
 */
//==============================================================================

extern BOOL GFL_NET_SystemGetKey(int no, u16* key);
//==============================================================================
/**
 * @brief   指定されたIDのハンドルを返す
 * @param   NetID  ネットＩＤ
 * @retval  GFL_NETHANDLE
 */
//==============================================================================
extern GFL_NETHANDLE* GFL_NET_SystemGetHandle(int NetID);

//extern void _commRecvCallback(u16 aid, u16 *data, u16 size);
//extern BOOL _commSystemInit(int packetSizeMax, HEAPID heapID);



typedef struct{
    int dataPoint; // 受信バッファ予約があるコマンドのスタート位置
    u8* pRecvBuff; // 受信バッファ予約があるコマンドのスタート位置
    u16 realSize;   /// 通信上にあったこのパケットのサイズ
    u16 tblSize;   /// テーブル上の登録サイズ
    u8 valCommand;  /// コマンド
    u8 sendID;      /// 送信者ID
    u8 recvBit;      /// 受信ID
} _RECV_COMMAND_PACK;

typedef struct{
    /// ----------------------------子機用＆親機用BUFF
    u8* sSendBuf;          ///<  子機の送信用バッファ
    u8* sSendBufRing;  ///<  子機の送信リングバッファ
    u8* sSendServerBuf;          ///<  親機の送信用バッファ
    u8* sSendServerBufRing;
    u8* pServerRecvBufRing;       ///< 親機側受信バッファ
    u8* pRecvBufRing;             ///< 子機が受け取るバッファ
    u8* pTmpBuff;                 ///< 受信受け渡しのための一時バッファポインタ
    //----ringbuff manager
    RingBuffWork sendRing;
    RingBuffWork recvRing;                      ///< 子機の受信リングバッファ
    RingBuffWork sendServerRing;
    RingBuffWork recvServerRing[GFL_NET_MACHINE_MAX];
    ///---quemanager 関連
    SEND_QUEUE_MANAGER sendQueueMgr;
    SEND_QUEUE_MANAGER sendQueueMgrServer;
    ///---受信関連
    _RECV_COMMAND_PACK recvCommServer[GFL_NET_MACHINE_MAX];
    _RECV_COMMAND_PACK recvCommClient;
    
    //---------  同期関連
    BOOL bWifiSendRecv;   // WIFIの場合同期を取る時ととらないときが必要なので 切り分ける
    volatile int countSendRecv;   // 送ったら＋受け取ったら－ 回数
    volatile int countSendRecvServer[GFL_NET_MACHINE_MAX];   // 送ったら＋受け取ったら－ 回数

    //-------
    int packetSizeMax;
    u16 bitmap;   // 接続している機器をBIT管理
    
    //-------------------
//    NET_TOOLSYS* pTool;  ///< netTOOLのワーク
//    GFL_NETHANDLE* pNetHandle[GFL_NET_MACHINE_MAX];
  //  u16 key[GFL_NET_MACHINE_MAX];
//    UI_KEYSYS* pKey[GFL_NET_MACHINE_MAX];       ///<  キーシェアリングポインタ
//    u8 device;   ///< デバイス切り替え wifi<>wi

#if _COUNT_TEST
    u8 DSCount; // コマンドの順番が正しいかどうか確認用
    u8 DSCountRecv[GFL_NET_MACHINE_MAX];
#endif
//    u8 recvDSCatchFlg[GFL_NET_MACHINE_MAX];  // 通信をもらったことを記憶 DS同期用

    u8 bFirstCatch[GFL_NET_MACHINE_MAX];  // コマンドをはじめてもらった時用
    u8 bFirstCatchP2C;    ///< 最初のデータを受信したタイミングでTRUEになるフラグ

    u8 bNextSendData;  ///
    u8 bNextSendDataServer;  ///
    u8 bAlone;    // 一人で通信できるようにするモードの時TRUE
    u8 bWifiConnect; //WIFI通信可能になったらTRUE
    u8 bResetState;
    u8 bError;  // 復旧不可能な時はTRUE
    u8 bShutDown;
    u8 bDSSendOK;   //DS通信で次を送信してよい場合TRUE 前回エラーで前のを送る場合FALSE
} _COMM_WORK_SYSTEM;


#endif // _NET_SYSTEM_H_

