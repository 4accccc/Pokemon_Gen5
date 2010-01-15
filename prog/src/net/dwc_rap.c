//=============================================================================
/**
 * @file	  dwc_rap.c
 * @brief	  DWCラッパー。オーバーレイモジュールに置くもの
 * @author	kazuki yoshihara
 * @date    06/02/23
 */
//=============================================================================

#include <nitro.h>
#include <nnsys.h>
#include <gflib.h>
#include "sc/dwc_sc.h"
#include "gdb/dwc_gdb.h"
#include "net/network_define.h"

#if GFL_NET_WIFI

// デバッグ出力を大量に吐き出す場合定義
#if defined(DEBUG_ONLY_FOR_ohno)
#define DEBUGPRINT_ON (1)
#else
#define DEBUGPRINT_ON (0)
#endif

#include "net/dwc_rap.h"
#include "vchat.h"
#include <vct.h>


// 何秒間通信が届かないとタイムアウトと判断するか
#define MYDWC_TIMEOUTSEC (10)


// 何フレーム送信がないと、KEEP_ALIVEトークンを送るか。
#define KEEPALIVE_TOKEN_TIME 240

#define MYDWC_DEBUGPRINT NET_PRINT


// フレンドリストのサイズ
#define FRIENDLIST_MAXSIZE 32

// １フレームに何人分のデータを更新するか。
#define FRIENDINFO_UPDATA_PERFRAME (1)

#define SIZE_RECV_BUFFER (4 * 1024)
#define SIZE_SEND_BUFFER 256

#define _MATCHSTRINGNUM (128)

#define _WIFI_NUM_MAX (2)  // 接続MAX数


typedef struct
{
  u8 sendBuffer[ SIZE_SEND_BUFFER ];
  //	u8* recvBuffer;
  DWCFriendData* pFriendData;  // DWC形式の友達リスト
  DWCFriendsMatchControl stDwcCnt;    // DWC制御構造体
  DWCUserData* pUserData;		// DWCのユーザデータ（自分のデータ）
  DWCInetControl stConnCtrl;
  void *recvPtr[_WIFI_NUM_MAX];  //受信バッファの32バイトアライメントしていないポインタ
  int deletedIndex[FRIENDLIST_MAXSIZE];
  int srcIndex[FRIENDLIST_MAXSIZE];
//  DWCUserData* pSaveDataUserData;
//  DWCFriendData* pSaveDataFriendData;
  GFL_NET_MYDWCReceiverFunc serverCallback;
  GFL_NET_MYDWCReceiverFunc clientCallback;
  MYDWCDisconnectFunc disconnectCallback;
  void* pDisconnectWork;
  MYDWCConnectFunc connectCallback;
  void* pConnectWork;
  GFL_NET_MYDWCConnectModeCheckFunc connectModeCheck;

  void (*fetalErrorCallback)(int);
  u8 randommatch_query[_MATCHSTRINGNUM];
  u8 friend_status[FRIENDLIST_MAXSIZE];   //WC_GetFriendStatusDataの戻り値
  u8 friendinfo[FRIENDLIST_MAXSIZE][MYDWC_STATUS_DATA_SIZE_MAX]; //WC_GetFriendStatusDataで得られる値
  u32 friendupdate_index;
  u32 connectionUserData;// 新機能 繋ぐ時のユーザーデータただし4バイト
  
  int state;
  int stepMatchResult;
  int matching_type;

  int sendbufflag;

  //	int op_aid;			// 相手のaid
  int maxConnectNum;  //最大接続人数
  u32 backupBitmap;

  int isvchat;
  int friendindex;   // 今から接続するフレンドのリストインデックス
  int newFriendConnect;  // 接続してきたらTRUE 2006.05.24 k.ohno
  BOOL bVChat;     // VCHATONOFF
  BOOL bConnectCallback;
  int vchatcodec;

  int timeoutflag;

  int sendintervaltime[_WIFI_NUM_MAX];		// 前回データを送信してからのフレーム数。
  int setupErrorCount;  //エラーした数をカウント

  int opvchaton;				// 相手のボイスチャットがオンかオフか
  u16 myvchaton;				// 自分のボイスチャットがオンかオフか
  u16 myvchat_send;			// 音声データ送信フラグ

  u32 BlockUse_BackupBitmap;

  u8 myseqno;				// 自分が送信するパケットのシーケンスNo
  u8 opseqno;				// 相手が最後に送ってきたパケットのシーケンスNo
  u8 bHeapError;  // HEAP確保失敗の場合
  u8 pausevchat; //vchat一時停止
  u8 blockClient;   // クライアントを接続禁止にする

  u8 closedflag;		// ConnectionClosedCallback でホスト数が1になったら切断処理に遷移するのか　TRUEで切断処理に遷移　080602 tomoya
  u8 saveing;  //セーブ中に1
  u8 bFriendSave;  //ともだちコードセーブ中に更新がかからないようにするフラグ
  u8 bWiFiFriendGroup;  ///< 友達と行うサービスかどうか
  u8 bConnectEnable;  ///< 接続許可と禁止を行う
} MYDWC_WORK;

// 親機のAID
#define _WIFI_PARENT_AID (0)

enum  _blockStatus{
  _BLOCK_NONE,
  _BLOCK_START,
  _BLOCK_CALLBACK,
};

// ボイスチャットのトークンと混合しないようにするため、
// ボイスチャットと違う３２バイトの数字をパケットの先頭につける。
#define MYDWC_PACKETYPE_MASK 0x000000ff
#define MYDWC_PACKET_VCHAT_MASK 0x00000100
#define MYDWC_PACKET_VCHAT_SHIFT 8
#define MYDWC_PACKET_SEQNO_POS 2

#define MYDWC_GAME_PACKET 0x0001

// タイムアウト処理を防ぐため、一定時間送信がない場合、
// からのデータを送る。そのときの数字
#define MYDWC_KEEPALIVE_PACKET 0x0002

enum{
  MDSTATE_INIT,
  MDSTATE_CONNECTING,
  MDSTATE_CONNECTED,
  MDSTATE_TRYLOGIN,
  MDSTATE_LOGIN,
  MDSTATE_MATCHING,
  MDSTATE_CANCEL,
  MDSTATE_MATCHED,
  MDSTATE_PLAYING,
  MDSTATE_CANCELFINISH,
  MDSTATE_FAIL,			// 2006.7.4 yoshihara追加
  MDSTATE_FAILFINISH,
  MDSTATE_ERROR,
  MDSTATE_ERROR_FM,
  MDSTATE_ERROR_DISCONNECT,
  MDSTATE_ERROR_FETAL,
  MDSTATE_DISCONNECTTING,
  MDSTATE_DISCONNECT,
  MDSTATE_TIMEOUT,
  MDSTATE_LAST
};

enum {
  MDTYPE_RANDOM,
  MDTYPE_PARENT,
  MDTYPE_CHILD
};


static MYDWC_WORK *_dWork = NULL;


static void LoginCallback(DWCError error, int profileID, void *param);
static void UpdateServersCallback(DWCError error, BOOL isChanged, void* param);
static void FriendStatusCallback(int index, u8 status, const char* statusString, void* param);
static void DeleteFriendListCallback(int deletedIndex, int srcIndex, void* param);
static void BuddyFriendCallback(int index, void* param);

static void ConnectToAnybodyCallback(DWCError error, BOOL cancel, BOOL self,BOOL isServer,int index, void *param);
static void SendDoneCallback( int size, u8 aid, void* param);
static void UserRecvCallback( u8 aid, u8* buffer, int size,void* param );
static void ConnectionClosedCallback(DWCError error, BOOL isLocal, BOOL isServer, u8  aid, int index, void* param);
//static int handleError();
static int EvaluateAnybodyCallback(int index, void* param);
static int mydwc_step(void);

static void recvTimeoutCallback(u8 aid, void* param);
static BOOL _isSendableReliable(void);
static void mydwc_releaseRecvBuff(int aid);
static void mydwc_releaseRecvBuffAll(void);
static void mydwc_allocRecvBuff(int i);

static void mydwc_updateFriendInfo( void );
static void _DWC_StartVChat(int heapID);

static void* mydwc_AllocFunc( DWCAllocType name, u32   size, int align );
static void mydwc_FreeFunc( DWCAllocType name, void* ptr,  u32 size  );
static void _FuncNonSave(void);


#if DEBUGPRINT_ON

static void _setStateDebug(int state,int line)
{
  NET_PRINT("rap%d\n",line);
   _dWork->state = state;
}
#define   _CHANGE_STATE(state)  _setStateDebug(state, __LINE__)

#else  //DEBUGPRINT_ON

static void _setState(int state)
{
   _dWork->state = state;
}
#define   _CHANGE_STATE(state)  _setState(state)

#endif //DEBUGPRINT_ON





//==============================================================================
/**
 * @brief   MYDWCが初期化済みかどうか
 * @param   void
 * @retval  TRUE  初期化済み
 */
//==============================================================================

BOOL GFL_NET_DWC_IsInit(void)
{
  return (_dWork!=NULL);
}


//==============================================================================
/**
 * インターネットへ接続開始
 * @param   pSaveData      セーブデータへのポインタ
 * @param   heapSize       ヒープサイズ
 * @param   maxConnectNum  最大接続人数
 * @retval  MYDWC_STARTCONNECT_OK … OK
 * @retval  MYDWC_STARTCONNECT_FIRST … 初めて接続する場合。（メッセージ表示の必要有
 * @retval  MYDWC_STARTCONNECT_DIFFERENTDS … 異なるＤＳで接続しようしてる場合。（要警告）
 */
//==============================================================================
int GFL_NET_DWC_startConnect(DWCUserData* pUserData, DWCFriendData* pFriendData)
{
  void* pTemp;
  GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();

  // ヒープ領域からワーク領域を確保。
  GF_ASSERT( _dWork == NULL );
#ifdef DEBUGPRINT_ON
  DWC_SetReportLevel(DWC_REPORTFLAG_ALL);
#else
  DWC_SetReportLevel(0);
#endif

  NET_PRINT("mydwc_start %d %d\n",sizeof(MYDWC_WORK) + 32, pNetInit->maxBeaconNum);

  _dWork = GFL_NET_Align32Alloc(pNetInit->netHeapID, sizeof(MYDWC_WORK));

  {
    int i;
    for(i=0;i<FRIENDLIST_MAXSIZE;i++){
      _dWork->deletedIndex[i] = -1;
    }
  }


  _CHANGE_STATE(MDSTATE_INIT);
//  _dWork->state = MDSTATE_INIT;

  _dWork->vchatcodec = VCHAT_NONE;
  _dWork->friendindex = -1;
  _dWork->maxConnectNum = pNetInit->maxConnectNum;
  _dWork->newFriendConnect = -1;
  _dWork->bVChat = TRUE;
  _dWork->bHeapError = FALSE;

  _dWork->myvchaton = 1;
  _dWork->opvchaton = 1;
  _dWork->myvchat_send = 1;

  _dWork->pUserData = pUserData;
  _dWork->pFriendData = pFriendData;
  // 2008.06.02 tomoya ClosedCallbackで切断処理に遷移するようにするのかをフラグできりかえれるように変更(Wi-Fiクラブ４人募集画面用)
  _dWork->closedflag = TRUE;

#ifdef PM_DEBUG
  DWC_ReportUserData(_dWork->pUserData);
#endif

  {
    int i;
    for( i = 0; i < FRIENDLIST_MAXSIZE; i++ )
    {
      _dWork->friend_status[i] = DWC_STATUS_OFFLINE;
    }
  }

  // ユーザデータの状態をチェック。
  GFL_NET_DWC_showFriendInfo();


  if( !DWC_CheckHasProfile( _dWork->pUserData ) )
  {
    // まだこのデータで一度もＷｉＦｉに繋いでいない。
    return MYDWC_STARTCONNECT_FIRST;
  }

  if( !DWC_CheckValidConsole( _dWork->pUserData ) )
  {
    // 本体情報が違う→違うＤＳで接続しようとしている。
    return 	MYDWC_STARTCONNECT_DIFFERENTDS;
  }



  return MYDWC_STARTCONNECT_OK;
}

//==============================================================================
/**
 * dwc_rap.cが確保した領域を開放する。
 * @param   none
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_free()
{
  if(_dWork){
    GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
    mydwc_releaseRecvBuffAll();
    //        GFL_HEAP_DeleteHeap(pNetInit->wifiHeapID);
    GFL_NET_Align32Free( _dWork  );
    _dWork = NULL;
  }
  DWC_ClearError();
}

//==============================================================================
/**
 * インターネット接続中に毎フレーム呼び出される関数
 * @param   userdata 自分のログインデータへのポインタ
 * @param   list フレンドリストの先頭ポインタ
 * @retval  正…接続完了。０…接続中。負…エラー（エラーコードが返る）
 */
//==============================================================================
int GFL_NET_DWC_connect()
{
  //    NET_PRINT("mydwc_connect %d\n",_dWork->state);
  switch( _dWork->state )
  {
  case MDSTATE_INIT:
    // 初期状態
    {
      GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
      // メモリ確保関数設定
      //DWC_SetMemFunc( mydwc_AllocFunc, mydwc_FreeFunc );
      // ネット接続初期化
      DWC_InitInetEx(&_dWork->stConnCtrl, GFL_DMA_NET_NO, _NETWORK_POWERMODE, _NETWORK_SSL_PRIORITY);

      //DWC_SetAuthServer(GF_DWC_CONNECTINET_AUTH_TYPE);

      // 非同期にネットに接続
      DWC_ConnectInetAsync();

      _CHANGE_STATE(MDSTATE_CONNECTING);
      //_dWork->state = MDSTATE_CONNECTING;
      _dWork->isvchat = 0;
    }

  case MDSTATE_CONNECTING:
    {
      // 安定した状況になるまで待つ。
      if( DWC_CheckInet() )
      {
        if( DWC_GetInetStatus() == DWC_CONNECTINET_STATE_CONNECTED )
        {
          _CHANGE_STATE(MDSTATE_CONNECTED);
          //_dWork->state = MDSTATE_CONNECTED;
        }
        else
        {
          _CHANGE_STATE(MDSTATE_ERROR);
          //_dWork->state = MDSTATE_ERROR;
        }
      }
      else
      {
        // 自動接続処理中
        DWC_ProcessInet();

#ifdef DEBUG_ONLY_FOR_toru_nagihashi
		 		switch ( DWC_GetInetStatus() )
				{
				case DWC_CONNECTINET_STATE_ERROR:
					{
					 	// エラー表示
					 	DWCError err;
					 	int errcode;
					 	err = DWC_GetLastError( &errcode );

						OS_TPrintf("   Error occurred %d %d.\n", err, errcode );
					}
					DWC_ClearError();

					DWC_CleanupInet();
					break;
				case DWC_CONNECTINET_STATE_FATAL_ERROR:
					{
					 	// エラー表示
					 	DWCError err;
					 	int errcode;
					 	err = DWC_GetLastError( &errcode );

					 	OS_Panic("   Error occurred %d %d.\n", err, errcode );
					}
					break;
				}
#endif

        break;
      }
    }

  case MDSTATE_CONNECTED:
    // エラーがおこっていないか判定
    {
      int ret = mydwc_HandleError();
      if( ret != 0 ){
        return ret;
      }
    }
    // オーバーレイしてる場合 DWC_Initを再度CALLしてあげないとここで停止
    // フレンドライブラリ初期化
    DWC_InitFriendsMatch( _dWork->pUserData,
													GAME_PRODUCTID, 
													GAME_SECRET_KEY, 0, 0,
													_dWork->pFriendData, FRIENDLIST_MAXSIZE);

    {// IPLのユーザ名を使ってログイン
      // 自分のユーザ名を圧縮。
      OSOwnerInfo info;
      OS_GetOwnerInfo( &info );
      DWC_LoginAsync( &(info.nickName[0]), NULL, LoginCallback, NULL);
    }
    _CHANGE_STATE(MDSTATE_TRYLOGIN);
    //_dWork->state = MDSTATE_TRYLOGIN;

  case MDSTATE_TRYLOGIN:
    // ログインが完了するまで待つ。
    DWC_ProcessFriendsMatch();
    break;

  case MDSTATE_LOGIN:
    DWC_ProcessFriendsMatch();   // 2006.04.07 k.ohno ログインしただけの状態を持続する為
    _dWork->stepMatchResult = STEPMATCH_CONNECT;
    return STEPMATCH_CONNECT;
  }
  return mydwc_HandleError();
}

//==============================================================================
/**
 * 受信したときのコールバックを設定する関数
 * @param client … クライアントとしてのコールバック関数
 * @param server … サーバとしてのコールバック関数
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_setReceiver( GFL_NET_MYDWCReceiverFunc server, GFL_NET_MYDWCReceiverFunc client )
{
  _dWork->serverCallback = server;
  _dWork->clientCallback = client;
}

//==============================================================================
/**
 * 切断を検知した時に呼ばれる関数をセットする
 * @param 	pFunc  切断時に呼ばれる関数
 * @param 	pWork  ワークエリア
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_SetDisconnectCallback( MYDWCDisconnectFunc pFunc, void* pWork )
{
  _dWork->disconnectCallback =pFunc;
  _dWork->pDisconnectWork =pWork;
}


//==============================================================================
/**
 * @brief   接続のコールバック内で判定する関数セット
 * @param 	pFunc  接続時に呼ばれる関数
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_SetConnectModeCheckCallback( GFL_NET_MYDWCConnectModeCheckFunc pFunc )
{
  _dWork->connectModeCheck = pFunc;
}

//==============================================================================
/**
 * 接続を検知した時に呼ばれる関数をセットする
 * @param 	pFunc  接続時に呼ばれる関数
 * @param 	pWork  ワークエリア
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_SetConnectCallback( MYDWCConnectFunc pFunc, void* pWork )
{
  _dWork->connectCallback =pFunc;
  _dWork->pConnectWork =pWork;
}


//------------------------------------------------------------------------------
/**
 * @brief   マッチメイクで、今できているグループに新たに別のクライアントが接続を開始した時に呼ばれる
 * @param   index  新規接続クライアントの友達リストインデックス。
                   新規接続クライアントが友達でなければ-1となる。
 * @param 	param  	コールバック用パラメータ
 * @retval  none
 */
//------------------------------------------------------------------------------


static void RandomNewClientCallback(int index,void *param)
{
}

//------------------------------------------------------------------------------
/**
 * @brief   新規接続クライアントの参加を決定する最終段階で、アプリケーション側に判断を求めるために呼び出されるコールバックです。
 * @param   newClientUserData
             新規接続クライアントが、
            DWC_ConnectToAnybodyAsync/DWC_ConnectToFriendsAsync/DWC_SetupGameServer/
            DWC_ConnectToGameServerAsync/DWC_ConnectToGameServerByGroupID
            関数のconnectionUserDataに設定した値を格納しているバッファへのポインタ。
            u8[DWC_CONNECTION_USERDATA_LEN]分のサイズ。
 * @param 	param  	コールバック用ワーク
 * @retval  TRUE  	新規接続クライアントを受け入れる。
 * @retval  FALSE 	新規接続クライアントの受け入れを拒否する。
 */
//------------------------------------------------------------------------------


static BOOL attemptCallback(u8 *newClientUserData,void *param)
{
  return _dWork->bConnectEnable;
}

//==============================================================================
/**
 * ランダム対戦を行う関数。ランダムマッチ中は数十フレーム処理が返ってこないことがある。
 * @retval  正…成功。０…失敗。
 */
//==============================================================================
//-----#if TESTOHNO
const static char randommatch_query[] = "%s = \'%s\'";


#define PPW_LOBBY_MATCHMAKING_KEY  "a"  //@@OOかり

int GFL_NET_DWC_StartMatch( u8* keyStr,int numEntry, BOOL bParent, u32 timelimit )
{
  

  
  GF_ASSERT( _dWork != NULL );

  if( _dWork->state != MDSTATE_LOGIN )
  {
    return 0;
  }
  mydwc_releaseRecvBuffAll();


/*   { //@@OO TWLには関数が無かったので削除
 *     int result;
 *     DWCMatchOptMinComplete moc={TRUE, 2, {0,0}, 0};
 *     moc.timeout = timelimit*1000;
 *     //        DWCMatchOptMinComplete moc={TRUE, 2, {0,0}, _RANDOMMATCH_TIMEOUT*1000};
 *     //	    if(!bParent){
 *     //            moc.timeout = 1; // 子機に時間の主導権がないように短く設定する
 *     //        }
 *     result = DWC_SetMatchingOption(DWC_MATCH_OPTION_MIN_COMPLETE,&moc,sizeof(moc));
 *     GF_ASSERT( result == DWC_SET_MATCH_OPT_RESULT_SUCCESS );
 *   }
 */
  GF_ASSERT(DWC_AddMatchKeyString(0,PPW_LOBBY_MATCHMAKING_KEY,(const char *)keyStr)!=0);
  {
    MI_CpuClear8(_dWork->randommatch_query,_MATCHSTRINGNUM);
    sprintf((char*)_dWork->randommatch_query,randommatch_query,PPW_LOBBY_MATCHMAKING_KEY,keyStr);
    GF_ASSERT(GFL_STD_StrLen((const char*)_dWork->randommatch_query) < _MATCHSTRINGNUM);
  }
  if(bParent){
    DWC_AddMatchKeyString(1,(const char*)_dWork->randommatch_query,(const char*)_dWork->randommatch_query);
  }
  {
    int i;
    for(i=0;i<numEntry; i++){
      mydwc_allocRecvBuff(i);
    }
  }

  _CHANGE_STATE(MDSTATE_MATCHING);
 // _dWork->state = MDSTATE_MATCHING;

  MYDWC_DEBUGPRINT("mydwc_startmatch %d ",numEntry);
  _dWork->maxConnectNum = numEntry;

  if(FALSE==DWC_ConnectToAnybodyAsync
    (
      DWC_TOPOLOGY_TYPE_FULLMESH,
      numEntry,
      (const char*)_dWork->randommatch_query,
      ConnectToAnybodyCallback,
      GFL_NET_GetWork(),
      RandomNewClientCallback,
      GFL_NET_GetWork(),
      EvaluateAnybodyCallback,
      GFL_NET_GetWork(),
      attemptCallback,
      (u8*)&_dWork->connectionUserData,
      GFL_NET_GetWork()
      )){
    return 0;
  }
  _dWork->matching_type = MDTYPE_RANDOM;
  // 送信コールバックの設定
  DWC_SetUserSendCallback( SendDoneCallback,NULL );

  // 受信コールバックの設定
  DWC_SetUserRecvCallback( UserRecvCallback,NULL );

  // コネクションクローズコールバックを設定
  DWC_SetConnectionClosedCallback(ConnectionClosedCallback, NULL);

  // タイムアウトコールバックの設定
  DWC_SetUserRecvTimeoutCallback( recvTimeoutCallback,NULL );

  _dWork->sendbufflag = 0;

  _dWork->closedflag = TRUE;	// 080602 tomoya
  return 1;
}
//----#endif //TESTOHNO


//==============================================================================
/**
 * ランダム対戦を行う関数。ランダムマッチ中は数十フレーム処理が返ってこないことがある。
 * @retval  正…成功。０…失敗。
 */
//==============================================================================

int GFL_NET_DWC_StartMatchFilter( char* filterStr,int numEntry,DWCEvalPlayerCallback evalcallback,void* pWork)
{
  GF_ASSERT( _dWork != NULL );
  if( _dWork->state != MDSTATE_LOGIN )
  {
    return 0;
  }
  mydwc_releaseRecvBuffAll();
  {
    int i;
    for(i=0;i<numEntry; i++){
      mydwc_allocRecvBuff(i);
    }
  }

  _CHANGE_STATE(MDSTATE_MATCHING);
  _dWork->maxConnectNum = numEntry;
  if(FALSE==DWC_ConnectToAnybodyAsync
    (
      DWC_TOPOLOGY_TYPE_STAR,
      numEntry,
      (const char*)filterStr,
      ConnectToAnybodyCallback,
      NULL,
      RandomNewClientCallback,
      NULL,
      evalcallback,
      pWork,
      NULL,
      NULL,
      NULL
      )){
    return 0;
  }
  _dWork->matching_type = MDTYPE_RANDOM;
  // 送信コールバックの設定
  DWC_SetUserSendCallback( SendDoneCallback,NULL );

  // 受信コールバックの設定
  DWC_SetUserRecvCallback( UserRecvCallback,NULL );

  // コネクションクローズコールバックを設定
  DWC_SetConnectionClosedCallback(ConnectionClosedCallback, NULL);

  // タイムアウトコールバックの設定
  DWC_SetUserRecvTimeoutCallback( recvTimeoutCallback,NULL );

  _dWork->sendbufflag = 0;

  _dWork->closedflag = TRUE;	// 080602 tomoya
  return 1;
}


// 2006.7.4 yoshihara 追加
static void finishcancel()
{
  if( _dWork->state == MDSTATE_FAIL ){
    _CHANGE_STATE(MDSTATE_FAILFINISH);
    //_dWork->state = MDSTATE_FAILFINISH;
  }
  else
  {
    _CHANGE_STATE(MDSTATE_CANCELFINISH);
  //  _dWork->state = MDSTATE_CANCELFINISH;
  }
}

//==============================================================================
/**
 * ランダム対戦マッチング中に毎フレーム呼ばれる関数。
 * @retval
 	STEPMATCH_CONTINUE…マッチング中
 	STEPMATCH_SUCCESS…成功
 	STEPMATCH_CANCEL…キャンセル
 	STEPMATCH_FAIL  …相手が親をやめたため、接続を中断
 	負…エラー発生
 */
//==============================================================================

int GFL_NET_DWC_stepmatch( int isCancel )
{
  switch ( _dWork->state ){
  case MDSTATE_INIT:
  case MDSTATE_CONNECTING:
  case MDSTATE_CONNECTED:
  case MDSTATE_TRYLOGIN:

    return GFL_NET_DWC_connect();
  case MDSTATE_MATCHING:
    // 現在探索中
    if( isCancel )
    {
      _CHANGE_STATE(MDSTATE_CANCEL);
     // _dWork->state = MDSTATE_CANCEL;
    }
    // 2006.7.4 yoshihara修正 ここから
    // 子機の場合、つなぎにいってる親が現在もサーバモードかどうかチェックする。
    if( _dWork->matching_type == MDTYPE_CHILD )
    {
      if( _dWork->friendindex >= 0 ) // このifは念のため
      {
        if( _dWork->friend_status[_dWork->friendindex] != DWC_STATUS_MATCH_SC_SV )
        {
          MYDWC_DEBUGPRINT("相手がサーバをやめてしまったので、キャンセルします。\n");
          // 既に親でなくなってしまっている。キャンセルへ移項
          _CHANGE_STATE(MDSTATE_FAIL);
         // _dWork->state = MDSTATE_FAIL;
        }
      }
    }
    // 2006.7.4 yoshihara修正 ここまで

    break;

  case MDSTATE_CANCEL:
  case MDSTATE_FAIL:
    {
      DWC_CloseAllConnectionsHard();
      finishcancel();	  // コールバック内でSTATEを変えないように修正した為 RETURNに関係なく状態変更する
    }
    break;    //   06.05.12追加
  case MDSTATE_MATCHED:
    // 完了。
    {
      GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
     if(_dWork->bVChat){
        _DWC_StartVChat(pNetInit->netHeapID);
      }
      _dWork->myvchaton = 0;

      _CHANGE_STATE(MDSTATE_PLAYING);
//      _dWork->state = MDSTATE_PLAYING;
      _dWork->stepMatchResult = STEPMATCH_SUCCESS;
      return _dWork->stepMatchResult;
    }
    break;
  case MDSTATE_CANCELFINISH:
    // ログイン直後の状態に
    _CHANGE_STATE(MDSTATE_LOGIN);
//    _dWork->state = MDSTATE_LOGIN;
    _dWork->sendbufflag = 0;
    _dWork->newFriendConnect = -1;
    MYDWC_DEBUGPRINT("キャンセル処理完了\n");
    _dWork->stepMatchResult = STEPMATCH_CANCEL;
    return STEPMATCH_CANCEL;
  case MDSTATE_FAILFINISH:
    // ログイン直後の状態に
    _CHANGE_STATE(MDSTATE_LOGIN);
//    _dWork->state = MDSTATE_LOGIN;
    _dWork->sendbufflag = 0;
    _dWork->newFriendConnect = -1;
    MYDWC_DEBUGPRINT("フィニッシュ処理完了\n");
    _dWork->stepMatchResult=STEPMATCH_FAIL;
    return STEPMATCH_FAIL;
  case MDSTATE_ERROR:
    return  mydwc_HandleError();
  case MDSTATE_DISCONNECTTING:
    // ボイスチャットの終了を待っている状態
    if( _dWork->isvchat == 0 ) {
      // ボイスチャットの切断完了。
      // 通信自体を切る。
      MYDWC_DEBUGPRINT("ボイスチャットの切断完了。\n");
      DWC_CloseAllConnectionsHard( );
      _CHANGE_STATE(MDSTATE_DISCONNECT);
     // _dWork->state = MDSTATE_DISCONNECT;
      break;
    }
  case MDSTATE_LOGIN:
    {
      int ret = mydwc_step();
      if(0==ret){
        _dWork->stepMatchResult = STEPMATCH_CONNECT;
        return STEPMATCH_CONNECT;
      }
      return ret;
    }
    break;
  default:
    break;
    //			return handleError();
  }
  return mydwc_step();

  //    DWC_ProcessFriendsMatch();  // DWC通信処理更新
  //	return handleError();
}



//==============================================================================
/**
 * サーバにデータ送信を行うことができるか
 * @retval  1 - 成功　 0 - 失敗（送信バッファが詰まっている等）
 */
//==============================================================================
BOOL GFL_NET_DWC_canSendToServer()
{
  return ( DWC_GetMyAID() == 0 || ( _dWork->sendbufflag == 0 && _isSendableReliable() ) );
}

//==============================================================================
/**
 * クライアントにデータ送信を行うことができるか
 * @retval  1 - 成功　 0 - 失敗（送信バッファが詰まっている等）
 */
//==============================================================================
BOOL GFL_NET_DWC_canSendToClient()
{
  return ( _dWork->sendbufflag == 0 && _isSendableReliable() );
}

//==============================================================================
/**
 * サーバにデータ送信を行う関数
 * @param   data - 送信するデータへのポインタ。size - 送信するデータのサイズ
 * @retval  1 - 成功　 0 - 失敗（送信バッファが詰まっている等）
 */
//==============================================================================
int GFL_NET_DWC_sendToServer(void *data, int size)
{
  if( !(size < SIZE_SEND_BUFFER) ){
    return 0;
  }
  if(FALSE == GFL_NET_DWC_canSendToServer()){
    return 0;
  }

  MYDWC_DEBUGPRINT("mydwc_sendToServer(data=%d)\n", *((u32*)data));

  if( DWC_GetMyAID() == 0 )
  {
    // 自分が親
    // コールバック内で書き換えられる可能性を考え、dataをコピーしておく。
    //		void *buf = mydwc_AllocFunc( NULL, size, 32 );
    //		MI_CpuCopy8	( data, buf, size );

    // 自分自身のサーバ受信コールバックを呼び出す。
    if( _dWork->serverCallback != NULL ) _dWork->serverCallback(0, data, size);

    // コールバックを呼び出したらすぐに開放。
    //		mydwc_FreeFunc( NULL, buf, size );

    return 1;
  }
  else
  {
    // 相手が親。相手に対してデータ送信。
    if( _dWork->sendbufflag || !_isSendableReliable() ) // 送信バッファをチェック。
    {
      // 送信バッファがいっぱいなどで送れない。
      return 0;
    }

    // 送信バッファにコピー
    *((u32*)&(_dWork->sendBuffer[0])) = MYDWC_GAME_PACKET | (_dWork->myvchaton << MYDWC_PACKET_VCHAT_SHIFT);
    _dWork->sendBuffer[MYDWC_PACKET_SEQNO_POS] = ++_dWork->myseqno;
    MI_CpuCopy8( data, &(_dWork->sendBuffer[4]), size );
    _dWork->sendbufflag = 1;

    // 親機に向けてのみ送信
    DWC_SendReliableBitmap( 0x01, &(_dWork->sendBuffer[0]), size + 4);
    //		MYDWC_DEBUGPRINT("-");
    //OHNO_PRINT("-");
    return 1;
  }
}

//==============================================================================
/**
 * 親機が子機にデータ送信を行う関数
 * @param   data - 送信するデータへのポインタ。size - 送信するデータのサイズ
 * @retval  1 - 成功　 0 - 失敗（送信バッファが詰まっている等）
 */
//==============================================================================
int GFL_NET_DWC_sendToClient(void *data, int size)
{
  if( !(size < SIZE_SEND_BUFFER) ){
    return 0;
  }
  // 親機しかこの動作は行わないはず。
  if( !(DWC_GetMyAID() == 0) ){
    return 0;
  }

  MYDWC_DEBUGPRINT("sendToClient(data=%d)\n", *((u32*)data));

  {
    // 相手に対してデータ送信。
    if( _dWork->sendbufflag || !_isSendableReliable() ) // 送信バッファをチェック。
    {
      //            MYDWC_DEBUGPRINT("wifi failed %d %d\n",_dWork->sendbufflag,_isSendableReliable());
      // 送信バッファがいっぱいなどで送れない。
      return 0;
    }

    // 送信バッファにコピー
    *((u32*)&(_dWork->sendBuffer[0])) = MYDWC_GAME_PACKET | (_dWork->myvchaton << MYDWC_PACKET_VCHAT_SHIFT);
    _dWork->sendBuffer[MYDWC_PACKET_SEQNO_POS] = ++_dWork->myseqno;
    MI_CpuCopy8( data, &(_dWork->sendBuffer[4]), size );
    _dWork->sendbufflag = 1;

    // 子機に向けて送信
    //   MYDWC_DEBUGPRINT("wifi send %z\n",DWC_GetAIDBitmap());
    if(!DWC_SendReliableBitmap(DWC_GetAIDBitmap(), &(_dWork->sendBuffer[0]), size + 4)){
      MYDWC_DEBUGPRINT("wifi SCfailed %d\n",size);
      // 送信バッファがいっぱいなどで送れない。
      _dWork->sendbufflag = 0;
      return 0;
    }
    //		MYDWC_DEBUGPRINT("-");
    //OHNO_PRINT(".");
  }

  {
    // 自分に対して送信
    //		// コールバック内で書き換えられる可能性を考え、dataをコピーしておく。
    //		void *buf = mydwc_AllocFunc( NULL, size, 32 );
    //		MI_CpuCopy8	( data, buf, size );

    // 自分自身のサーバ受信コールバックを呼び出す。
    if( _dWork->clientCallback != NULL ) _dWork->clientCallback(0, data, size);

    // コールバックを呼び出したらすぐに開放。
    //		mydwc_FreeFunc( NULL, buf, size );
  }

  return 1;
}


//==============================================================================
/**
 * 他の相手に送信を行う関数
 * @param   data - 送信するデータへのポインタ。size - 送信するデータのサイズ
 * @retval  1 - 成功　 0 - 失敗（送信バッファが詰まっている等）
 */
//==============================================================================
int GFL_NET_DWC_SendToOther(void *data, int size)
{
  u16 bitmap;

  if( !(size < SIZE_SEND_BUFFER) ){
    return 0;
  }


  {
    // 相手に対してデータ送信。
    if( _dWork->sendbufflag || !_isSendableReliable() ) // 送信バッファをチェック。
    {
      //            MYDWC_DEBUGPRINT("wifi failed %d %d\n",_dWork->sendbufflag,_isSendableReliable());
      // 送信バッファがいっぱいなどで送れない。
      return 0;
    }

    // 送信バッファにコピー
    *((u32*)&(_dWork->sendBuffer[0])) = MYDWC_GAME_PACKET | (_dWork->myvchaton << MYDWC_PACKET_VCHAT_SHIFT);
    _dWork->sendBuffer[MYDWC_PACKET_SEQNO_POS] = ++_dWork->myseqno;
    MI_CpuCopy8( data, &(_dWork->sendBuffer[4]), size );
    _dWork->sendbufflag = 1;

    bitmap = DWC_GetAIDBitmap();
    if(bitmap != DWC_SendReliableBitmap(bitmap, &(_dWork->sendBuffer[0]), size + 4)){
      MYDWC_DEBUGPRINT("wifi SOFailed %d\n",size);
      // 送信バッファがいっぱいなどで送れない。
      _dWork->sendbufflag = 0;
      return 0;
    }
  }

  // 自分自身の受信コールバックを呼び出す。

  if( _dWork->clientCallback != NULL ){
    _dWork->clientCallback(DWC_GetMyAID() , data, size);
  }
  return 1;
}

/*---------------------------------------------------------------------------*
  ログインコールバック関数
 *---------------------------------------------------------------------------*/
static void LoginCallback(DWCError error, int profileID, void *param)
{
  BOOL result;

  // stUserDataが更新されているかどうかを確認。
  if ( DWC_CheckDirtyFlag( _dWork->pUserData) )
  {
    // 必ずこのタイミングでチェックして、dirty flagが有効になっていたら、
    // DWCUserDataをDSカードのバックアップに保存するようにしてください。
    DWCUserData *userdata = NULL;
    DWC_ClearDirtyFlag(_dWork->pUserData);
    _dWork->saveing = 1;  //セーブ中に1

    MYDWC_DEBUGPRINT("自分のフレンドコードが変更\n");
  }

  if (error == DWC_ERROR_NONE){
    // 認証成功、プロファイルID取得
    result = DWC_UpdateServersAsync(NULL, //（過去との互換性のため、必ずNULL)
                                    UpdateServersCallback, _dWork->pUserData,
                                    FriendStatusCallback, param,
                                    DeleteFriendListCallback, param);

    if (result == FALSE){
      // 呼んでもいい状態（ログインが完了していない状態）で呼んだ時のみ
      // FALSEが返ってくるので、普通はTRUE
      MYDWC_DEBUGPRINT("DWC_UpdateServersAsync error teminated.\n");
      GFL_NET_StateSetError(GFL_NET_ERROR_RESET_SAVEPOINT);
      return;
    }
    // GameSpyサーバ上バディ成立コールバックを登録する
    DWC_SetBuddyFriendCallback(BuddyFriendCallback, NULL);
  }
  else
  {
    // 認証失敗
    _CHANGE_STATE(MDSTATE_ERROR);
   // _dWork->state = MDSTATE_ERROR;
  }
}

/*---------------------------------------------------------------------------*
  タイムアウトコールバック関数
 *---------------------------------------------------------------------------*/
static void recvTimeoutCallback(u8 aid, void* param)
{
  MYDWC_DEBUGPRINT("DWCタイムアウト - %d",aid);
  // コネクションを閉じる
  if( _dWork->timeoutflag )
  {
    MYDWC_DEBUGPRINT("接続を切断します\n");
    DWC_CloseAllConnectionsHard( );
    _dWork->newFriendConnect = -1;
    // タイムアウト
    _CHANGE_STATE(MDSTATE_TIMEOUT);
 //   _dWork->state = MDSTATE_TIMEOUT;
  }
}

/*---------------------------------------------------------------------------*
  GameSpyサーバアップデートコールバック関数
 *---------------------------------------------------------------------------*/
static void UpdateServersCallback(DWCError error, BOOL isChanged, void* param)
{
#pragma unused(param)
  if (error == DWC_ERROR_NONE){
    if (isChanged){
      // 友達リストが変更されていた
    }
    _CHANGE_STATE(MDSTATE_LOGIN);
  //  _dWork->state = MDSTATE_LOGIN;		// ログイン完了
  }
  else {
    // ログイン失敗扱いにしとく？
    _CHANGE_STATE(MDSTATE_ERROR);
 //   _dWork->state = MDSTATE_ERROR;
  }
}


/*---------------------------------------------------------------------------*
  友達状態変化通知コールバック関数
 *---------------------------------------------------------------------------*/
static void FriendStatusCallback(int index, u8 status, const char* statusString, void* param)
{
#pragma unused(param)

  MYDWC_DEBUGPRINT("Friend[%d] changed status -> %d (statusString : %s).\n",
                   index, status, statusString);

}


/*---------------------------------------------------------------------------*
  友達リスト削除コールバック関数
 *---------------------------------------------------------------------------*/
static void DeleteFriendListCallback(int deletedIndex, int srcIndex, void* param)
{
#pragma unused(param)
  int i;
  GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();

  // フレンドリストが削除された。
  MYDWC_DEBUGPRINT("friend[%d] was deleted (equal friend[%d]).\n", deletedIndex, srcIndex);

  for(i = 0;i < FRIENDLIST_MAXSIZE; i++){
    if(_dWork->deletedIndex[i] == -1){
      _dWork->deletedIndex[i] = deletedIndex;
      _dWork->srcIndex[i] = srcIndex;
    }
  }
}


/*---------------------------------------------------------------------------*
  GameSpyバディ成立コールバック関数
 *---------------------------------------------------------------------------*/
static void BuddyFriendCallback(int index, void* param)
{
#pragma unused(param)

  MYDWC_DEBUGPRINT("I was authorized by friend[%d].\n", index);
}

/*---------------------------------------------------------------------------*
  タイムアウト時間のクリア
 *---------------------------------------------------------------------------*/
static void clearTimeoutBuff(void)
{
  MI_CpuClear8(_dWork->sendintervaltime, sizeof(_dWork->sendintervaltime));
}
/*---------------------------------------------------------------------------*
  タイムアウト時間の設定
 *---------------------------------------------------------------------------*/
static void setTimeoutTime(void)
{
  int i;

  for(i = 0 ;i < _WIFI_NUM_MAX; i++){
    DWC_SetRecvTimeoutTime( i, 0 );
  }


  if(DWC_GetMyAID() == _WIFI_PARENT_AID){
    for(i = 0 ;i < _dWork->maxConnectNum; i++){
      if(DWC_GetMyAID() != i){
        if(DWC_GetAIDBitmap() & (0x01<<i) ){
          GF_ASSERT(DWC_SetRecvTimeoutTime( i, MYDWC_TIMEOUTSEC * 1000 ));
          MYDWC_DEBUGPRINT("setTimeOut %d\n",i);
        }
      }
    }
  }
  else{
    GF_ASSERT(DWC_SetRecvTimeoutTime( _WIFI_PARENT_AID, MYDWC_TIMEOUTSEC * 1000 ));
    MYDWC_DEBUGPRINT("setTimeOut 0\n");
  }

  // 080523 tomoya takahashi
  // BTS:通信No.167
  // 親から何か受信しないとtimeoutflagがたたないため、
  // 子はいつまでもタイムアウト状態にならない可能性がある。
  // その為、マッチングが成功したら、直ぐにtimeoutflagが立つように
  // 変更してみる。
  // _dWork->timeoutflag = 0;	<-元々
  _dWork->timeoutflag = 1;
  clearTimeoutBuff();
}


static void setTimerAndFlg(int index)
{
  int i,j;

  OS_TPrintf("接続しました%s%d\n",__FILE__,__LINE__);
//  _dWork->state = MDSTATE_MATCHED;
     _CHANGE_STATE(MDSTATE_MATCHED);

  setTimeoutTime();
}

/*---------------------------------------------------------------------------*
  友達無指定接続コールバック関数
 *---------------------------------------------------------------------------*/
static void ConnectToAnybodyCallback(DWCError error,
             BOOL cancel,
             BOOL self,
             BOOL isServer,
             int index,
             void *param)
{
#pragma unused(param)

  if (error == DWC_ERROR_NONE){
    if (!cancel){
      // 見知らぬ人たちとのコネクション設立完了
      MYDWC_DEBUGPRINT("接続完了 %d\n",index);
      setTimerAndFlg(index);
    }
    else {
      MYDWC_DEBUGPRINT("キャンセル完了\n");
      // ログイン後状態に戻る

      // 2006.7.4 yoshihara 修正
      finishcancel();
    }
  }
  else {
    MYDWC_DEBUGPRINT("マッチング中にエラーが発生しました。 %d\n\n", error);
//    _dWork->state = MDSTATE_ERROR;
     _CHANGE_STATE(MDSTATE_ERROR);
  }
  if( _dWork->connectCallback ){
    // 何人とつながっても一回しか呼ばれないので自分のIDを入れている
    _dWork->connectCallback(DWC_GetMyAID(), _dWork->pConnectWork);
  }
}

/*---------------------------------------------------------------------------*
  友達無指定マッチング時プレイヤー評価コールバック関数
 *---------------------------------------------------------------------------*/
static int  EvaluateAnybodyCallback(int index, void* param)
{
#pragma unused(index,param)

  return 1;
}


/** -------------------------------------------------------------------------
  送信完了コールバック
  ---------------------------------------------------------------------------*/
static void SendDoneCallback( int size, u8 aid,void* param )
{
#pragma unused(size)
  // 送信バッファをあける
  _dWork->sendbufflag = 0;
  _dWork->sendintervaltime[aid] = 0;
  //	MYDWC_DEBUGPRINT( "送信\n" );
  //	MYDWC_DEBUGPRINT( "*" );
}



static void _setOpVchat(u32 topcode)
{

  //  if(!CommLocalIsWiFiQuartetGroup(CommStateGetServiceNo())){
  if( topcode & MYDWC_PACKET_VCHAT_MASK )
  {
    // 相手のボイスチャットはオン
    _dWork->opvchaton = 1;
  }
  else
  {
    // 相手のボイスチャットはオン
    _dWork->opvchaton = 0;
  }
  //    }
}



/** -------------------------------------------------------------------------
  受信完了コールバック
  ---------------------------------------------------------------------------*/
static void UserRecvCallback( u8 aid, u8* buffer, int size,void* param )
{
#pragma unused( aid, buffer, size )

  u32 topcode;
  topcode = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];

  //	MYDWC_DEBUGPRINT("[%d,%d,%d,%d]", buffer[0], buffer[1], buffer[2], buffer[3]);
//  MYDWC_DEBUGPRINT("-受信-\n");

  // 一度受信してはじめてタイムアウトを設定する。
  _dWork->timeoutflag = 1;

  // まず、先頭の４バイトをみて、ゲームの情報かどうかを判断
  if( (topcode & MYDWC_PACKETYPE_MASK) == MYDWC_GAME_PACKET ){
    _setOpVchat( topcode );
    _dWork->opseqno = buffer[MYDWC_PACKET_SEQNO_POS];
  }
  else {

    if( myvct_checkData( aid, buffer,size ) ) return;

    // 無意味な情報（コネクションを保持するためのものと思われる）
    _setOpVchat( topcode );
    return;
  }
  //	MYDWC_DEBUGPRINT( "受信(%d)\n",*((s32*)buffer) );
  //NET_PRINT( "受信(%d)\n",aid );

#if 1
  {
    u16 *temp = (u16*)buffer + 2;
    
    if( DWC_GetMyAID() == 0 )
    {
      // 自分が親の場合…クライントからサーバに対して送られてきたものと判断。
      // サーバ用受信関数を呼び出す。
      if( _dWork->serverCallback != NULL ) _dWork->serverCallback(aid, temp, size-4);
    } else {
      // サーバからクライアントに対して送られてきたものと判断。
      if( _dWork->clientCallback != NULL ){
        _dWork->clientCallback(aid, temp, size-4);
      }
    }
  }
#else
  {
    u16 *temp = (u16*)mydwc_AllocFunc( NULL, size - 4, 4);
    if(temp==NULL){
      return;
    }

    MI_CpuCopy8(buffer + 4, (void*)temp, size - 4);

    if( DWC_GetMyAID() == 0 )
    {
      // 自分が親の場合…クライントからサーバに対して送られてきたものと判断。
      // サーバ用受信関数を呼び出す。
      if( _dWork->serverCallback != NULL ) _dWork->serverCallback(aid, temp, size-4);
    } else {
      // サーバからクライアントに対して送られてきたものと判断。
      if( _dWork->clientCallback != NULL ){
        _dWork->clientCallback(aid, temp, size-4);
      }
    }

    mydwc_FreeFunc(NULL, temp, size - 4);
  }
#endif
}

/*---------------------------------------------------------------------------*
  コネクションクローズコールバック関数
error  	DWCエラー種別。内容の詳細は、DWC_GetLastErrorExを参照してください。
isLocal
TRUE:自分がクローズした。
FALSE:他の誰かがクローズした。 マッチメイク中に応答のないホストを切断する時もTRUEになる。
isServer
TRUE: サーバクライアントマッチメイクのサーバDSがクローズした。自分がサーバDSでisLocal = TRUE の時もTRUEになる。
FALSE:それ以外。サーバクライアントマッチメイク以外では必ずFALSEとなる。
aid 	クローズしたプレイヤーのAID
index 	クローズしたプレイヤーの友達リストインデックス。クローズしたプレイヤーが友達でなければ-1となる。
param 	DWC_SetConnectionClosedCallback関数で指定したコールバック用パラメータ
 *---------------------------------------------------------------------------*/
static void ConnectionClosedCallback(DWCError error,
                                     BOOL isLocal,
                                     BOOL isServer,
                                     u8  aid,
                                     int index,
                                     void* param)
{
#pragma unused(param, index)
  MYDWC_DEBUGPRINT("ConnectionClosedCallback %d %d %d %d %d\n",error,isLocal, isServer,aid,_dWork->closedflag);

  _dWork->sendbufflag = 0;
  _dWork->setupErrorCount = 0;
  _dWork->bConnectCallback = FALSE;

  if (error == DWC_ERROR_NONE){

    //        if((!CommLocalIsWiFiQuartetGroup(CommStateGetServiceNo()) && (DWC_GetNumConnectionHost() == 1))){
    if(((_dWork->closedflag == TRUE) && (DWC_GetNumConnectionHost() == 1))){
      // 全てのコネクションがクローズされた場合

      // キャンセル中ならば、キャンセルが完了した。
      if(	_dWork->state == MDSTATE_CANCEL )
      {
        //  DWC_SetupGameServerのコールバックで呼ばれるため まだ状態がDISCONNECTできていない可能性があるため ここでFINISHにしない
        //				_dWork->state = MDSTATE_CANCELFINISH;
        MYDWC_DEBUGPRINT("MDSTATE_CANCELFINISH\n");
      }
      else {
        //ここは削除
        //                OHNO_PRINT("DWC_CloseAllConnectionsHard");
        //                DWC_CloseAllConnectionsHard( );
        //				_dWork->state = MDSTATE_DISCONNECT;
   //     _dWork->state = MDSTATE_DISCONNECTTING;
     _CHANGE_STATE(MDSTATE_DISCONNECTTING);
 
        MYDWC_DEBUGPRINT("MDSTATE_DISCONNECTTING\n");
      }
      if( _dWork->isvchat )
      {
        // まだボイスチャットライブラリを解放していない場合。
        GFL_NET_DWC_StopVChat();
      }

    }

    if (isLocal){
      MYDWC_DEBUGPRINT("Closed connection to aid %d (friendListIndex = %d) Rest %d.\n",
                       aid, index, DWC_GetNumConnectionHost());
    }
    else {
      MYDWC_DEBUGPRINT("Connection to aid %d (friendListIndex = %d) was closed. Rest %d.\n",
                       aid, index, DWC_GetNumConnectionHost());
    }
  }
  else {
    MYDWC_DEBUGPRINT("Failed to close connections. %d\n\n", error);
  }

  //    mydwc_releaseRecvBuff(aid);

  GFI_NET_HANDLE_Delete(aid);  // ハンドル削除
  if( _dWork->disconnectCallback )
  {
    _dWork->disconnectCallback(aid, _dWork->pDisconnectWork);
  }
}

/*---------------------------------------------------------------------------*
  メモリ確保関数
 *---------------------------------------------------------------------------*/
#if 0
void* mydwc_AllocFunc( DWCAllocType name, u32   size, int align )
{
#pragma unused( name )
  void * ptr;
  OSIntrMode old;
  GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();

#ifdef DEBUGPRINT_ON
  NET_PRINT("HEAP取得(%d, %d) \n", size, align);
#endif

  GF_ASSERT(align <= 32);  // これをこえたら再対応
  old = OS_DisableInterrupts();
  ptr = GFL_NET_Align32Alloc(pNetInit->wifiHeapID, size);
  OS_RestoreInterrupts( old );

  if(ptr == NULL){
    GF_ASSERT(ptr);
    // ヒープが無い場合の修正
    GFL_NET_StateSetError(GFL_NET_ERROR_RESET_SAVEPOINT);
    return NULL;
  }
  return ptr;
}
#endif

/*---------------------------------------------------------------------------*
  メモリ開放関数
 *---------------------------------------------------------------------------*/
void mydwc_FreeFunc( DWCAllocType name, void* ptr,  u32 size  )
{
#pragma unused( name, size )
  OSIntrMode old;
  u16 groupid;

  if ( !ptr ){
    return;  //NULL開放を認める
  }
  old = OS_DisableInterrupts();
  GFL_NET_Align32Free(ptr);
  OS_RestoreInterrupts( old );
}

//==============================================================================
/**
 * エラーが起こっていないかチェックし、エラーに応じて通信処理を閉じる処理をする。
 *  世界交換、世界バトルタワーで呼び出すために多少改造 k.ohno 06.06.08
 * @retval  ０…エラーなし。負…エラー  正 returnエラー
 */
//==============================================================================
int mydwc_HandleError(void)
{
  int errorCode;
  DWCErrorType myErrorType;
  int ret;
  int returnNo = 0;


  ret = DWC_GetLastErrorEx( &errorCode, &myErrorType );


  if( ret != 0 ){
    // 何らかのエラーが発生。
    MYDWC_DEBUGPRINT("error occured!(%d, %d, %d)\n", ret, errorCode, myErrorType);

    // DWC_GetLastErrorExの説明にのっとる  2008.5.23 -> 2010.1.5 nagihashi update
    // 返すものは基本的にerrorCodeであるが
    // エラーコードが 0 の場合やエラー処理タイプが DWC_ETYPE_LIGHT の場合は、ゲーム固有の表示のみなので retを返す
    // heapエラーだったら後でさらに変更
    returnNo = errorCode;
    if((errorCode == 0) || (myErrorType == DWC_ETYPE_LIGHT)){
      returnNo = ret;
    }
    switch(myErrorType)
    {
    case DWC_ETYPE_LIGHT:
      // ゲーム固有の表示のみで、エラーコード表示は必要ない。
      // DWC_ClearError()を呼び出せば、復帰可能。
      DWC_ClearError();
      break;

    case DWC_ETYPE_SHOW_ERROR:  // エラー表示が必要
      DWC_ClearError();
      break;

    case DWC_ETYPE_SHUTDOWN_GHTTP:
      //以下の各ライブラリ解放関数を必要に応じて呼び出してください。
      //DWC_RnkShutdown関数(汎用ランキングライブラリ)
      //エラーコードを表示してください。
      //
      if( ret == DWC_ERROR_GDB_ANY )
      { 
        DWC_GdbShutdown();           // 簡易データベースライブラリの終了
      }

      if( ret == DWC_ERROR_SCL_ANY )
      { 
        DWC_ScShutdown();
        DWC_GdbShutdown();           // 簡易データベースライブラリの終了
      }

      //DWC_RnkShutdown(); //ランキングライブラリは現状使用しないはずなので、呼ばない
      DWC_ClearError();
      break;

    case DWC_ETYPE_SHUTDOWN_ND:
      //DWC_NdCleanupAsync関数を呼び出して、ダウンロードライブラリを終了する必要があります。
      //エラーコードを表示してください。 
      DWC_NdCleanupAsync();
      DWC_ClearError();
      break;

    case DWC_ETYPE_SHUTDOWN_FM:
      // DWC_ShutdownFriendsMatch()を呼び出して、FriendsMatch処理を終了する必要がある。
      // エラーコードの表示が必要。
      // この場合、とりあえずDWC_ETYPE_DISCONNECTと同じ処理をしておく。
    case DWC_ETYPE_DISCONNECT:
      //FriendsMatch処理中ならDWC_ShutdownFriendsMatch()を呼び出し、
      //更にDWC_CleanupInet()で通信の切断も行う必要がある。
      //エラーコードの表示が必要。
      if(_dWork){
        switch( _dWork->state )
        {
        case MDSTATE_TRYLOGIN:
        case MDSTATE_LOGIN:
        case MDSTATE_MATCHING:
        case MDSTATE_CANCEL:
        case MDSTATE_MATCHED:
        case MDSTATE_PLAYING:
        case MDSTATE_CANCELFINISH:
        case MDSTATE_ERROR:
        case MDSTATE_TIMEOUT:
        case MDSTATE_DISCONNECTTING:
        case MDSTATE_DISCONNECT:
          DWC_ShutdownFriendsMatch(  );
        case MDSTATE_INIT:
        case MDSTATE_CONNECTING:
        case MDSTATE_CONNECTED:
          // まだこの時点では、フレンドマッチライブラリは呼ばれていない。
          // WIFI広場の場合外で呼ぶ
          //                    if(CommStateGetServiceNo() != COMM_MODE_LOBBY_WIFI){
          DWC_CleanupInet( );
          //                  }
        }

        DWC_ClearError();
      }
      if(_dWork){
    //    _dWork->state = MDSTATE_ERROR_DISCONNECT;
      _CHANGE_STATE(MDSTATE_ERROR_DISCONNECT);
      }
      break;
    case DWC_ETYPE_FATAL:
      // FatalError相当なので、電源OFFを促す必要がある。
      if(_dWork){
//        _dWork->state = MDSTATE_ERROR_FETAL;
      _CHANGE_STATE(MDSTATE_ERROR_FETAL);
        // このコールバックから処理がかえってこないはず。
        if( _dWork->fetalErrorCallback != NULL ){
          _dWork->fetalErrorCallback( -errorCode );
        }
      }
      break;
    }
  }
  if(_dWork->bHeapError){
    returnNo = ERRORCODE_HEAP;
  }
  if( ret != 0 ){
    GFL_NET_StateSetWifiError( errorCode, myErrorType, ret);
  }
  return returnNo;
}


//Reliable送信可能かどうかをチェックする関数です。

static BOOL _isSendableReliable(void)
{
  int i;
  BOOL bRet=FALSE;

  for(i=0;i< _dWork->maxConnectNum;i++){
    if(DWC_GetMyAID()==i){
      continue;
    }
    if(DWC_IsValidAID(i)){
      bRet=TRUE;  // 送信相手がいた場合TRUE
      if(!DWC_IsSendableReliable(i)){
        return FALSE;
      }
    }
  }
  return bRet;
}

/*
static int _buffer;

static void _sendData(int param)
{
	_buffer = param;
	if( !DWC_IsSendableReliable( _dWork->op_aid ) ) // 送信バッファをチェック。
	{
		DWC_SendReliable( _dWork->op_aid, &(_buffer), 4 );
	}
}
 */


static BOOL sendKeepAlivePacket(int i)
{
  if( (_dWork->sendbufflag == 0) && _isSendableReliable() && (0xfffe & DWC_GetAIDBitmap()) ) // 送信バッファをチェック。
  {
    MYDWC_DEBUGPRINT("SEND KEEP ALIVE PACKET  %d %d %d\n",_dWork->sendbufflag,_isSendableReliable() , DWC_GetAIDBitmap());
    _dWork->sendbufflag = 1;
    *((u32*)&(_dWork->sendBuffer[0])) = MYDWC_KEEPALIVE_PACKET | (_dWork->myvchaton << MYDWC_PACKET_VCHAT_SHIFT);

    DWC_SendReliableBitmap(DWC_GetAIDBitmap(), &(_dWork->sendBuffer[0]), 4);

    _dWork->sendintervaltime[i] = 0;
    //MYDWC_DEBUGPRINT("SEND KEEP ALIVE PACKET\n");
    //OHNO_PRINT("KEEP ALIVE %d\n",sys.vsync_counter);
    return TRUE;
  }
  return FALSE;
}


// 通信確立後、毎フレーム呼び出してください。
//==============================================================================
/**
 * DWC通信処理更新を行う
 * @param none
 * @retval 0…正常, 負…エラー発生 1…タイムアウト 2…相手から切断された
 */
//==============================================================================
static int mydwc_step(void)
{
  int i,ret;

  _FuncNonSave();  //セーブデータの方にフレンドデータを移し変える

  DWC_ProcessFriendsMatch();  // DWC通信処理更新
  mydwc_updateFriendInfo();



  if( _dWork->isvchat )
  {

    if( (_dWork->myvchaton == 1) && (_dWork->opvchaton == 1) && (_dWork->myvchat_send == TRUE) )
    {
      myvct_main(FALSE);
    }
    else
    {
      myvct_main(TRUE);
    }

    if(_dWork->backupBitmap != DWC_GetAIDBitmap()){
      if(!_dWork->pausevchat && _dWork->bVChat){
        if(myvct_AddConference(DWC_GetAIDBitmap(), DWC_GetMyAID())){
          _dWork->backupBitmap = DWC_GetAIDBitmap();
        }
      }
    }
  }

  if( _dWork->state == MDSTATE_TIMEOUT ){  // タイムアウトステートの時は同時にエラーも監視する
    ret = mydwc_HandleError();
    if(ret != 0){
      return ret;
    }
    return STEPMATCH_TIMEOUT;
  }
  if( _dWork->state == MDSTATE_DISCONNECT ) return STEPMATCH_DISCONNECT;

  if( _dWork->state == MDSTATE_MATCHED  ||  _dWork->state == MDSTATE_PLAYING) {
    for(i=0 ; i< _dWork->maxConnectNum; i++){
      if( _dWork->sendintervaltime[i]++ >= KEEPALIVE_TOKEN_TIME && _dWork->sendbufflag == 0) {
        if(sendKeepAlivePacket(i)){
          clearTimeoutBuff();
          break;
        }
      }
    }
  }

  return mydwc_HandleError();
}

//==============================================================================
/**
 * aidを返します。接続するまでは-1を返します。
 * @retval  aid。ただし接続前は0xffffffff
 */
//==============================================================================
u32 GFL_NET_DWC_GetAid(void)
{
  if(_dWork){
    if( _dWork->state == MDSTATE_MATCHED || _dWork->state == MDSTATE_PLAYING || _dWork->state == MDSTATE_DISCONNECTTING )
    {
      return DWC_GetMyAID();
    }
  }
  return 0xffffffff;
}

// ボイスチャットの切断コールバック。
static void vct_endcallback(){
  _dWork->isvchat = 0;
}

//==============================================================================
/**
 * 通信確立後、ボイスチャットを開始します。
 * @param   heapID  メモリ確保するHEAPID
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_StartVChat(void)
{
  _dWork->myvchaton = 1;
}

static void _DWC_StartVChat(int heapID)
{
  int late;
  int num = 1;
  //    BOOL bFourGame = CommLocalIsWiFiQuartetGroup(CommStateGetServiceNo());

  // デバックプリントOFF
#ifndef DEBUGPRINT_ON
  VCT_SetReportLevel( VCT_REPORTLEVEL_NONE );
#else
  VCT_SetReportLevel( VCT_REPORTLEVEL_ALL );

#endif
#if 0
  if(bFourGame){
    num = GFL_NET_GetConnectNum()-1;
    MYDWC_DEBUGPRINT("VCTON人数%d\n",num);
    if(num < 1){
      num = 1;
    }
    _dWork->myvchaton = 1;
    _dWork->opvchaton = 1;
    _dWork->myvchat_send = 1;
  }
#endif
  _dWork->myvchaton = 1;
  _dWork->opvchaton = 1;
  _dWork->myvchat_send = 1;
  
  if( _dWork->isvchat==0 ){
    switch( _dWork->vchatcodec ){
    case VCHAT_G711_ULAW:
      late = VCT_CODEC_G711_ULAW;
      break;
    case VCHAT_2BIT_ADPCM:
      late = VCT_CODEC_2BIT_ADPCM;
      break;
    case VCHAT_3BIT_ADPCM:
      late = VCT_CODEC_3BIT_ADPCM;
      break;
    case VCHAT_4BIT_ADPCM:
      late = VCT_CODEC_4BIT_ADPCM;
      break;
    default:
      //       if(!bFourGame){
      late = VCT_CODEC_4BIT_ADPCM;   //  DP
      //     }
      //   else{
      //     late = VCT_CODEC_3BIT_ADPCM;
      //            }
      break;
    }
    myvct_init( heapID, late, num );
    myvct_setDisconnectCallback( vct_endcallback );
    _dWork->isvchat = 1;
  }
}

//==============================================================================
/**
 * ボイスチャットのコーデックを指定します。
 * コネクション確立前にコーデックを指定しておくと、
 * コネクション確立時に自動的にボイスチャットを開始します。
 * @param コーデックのタイプ
 * @retval none
 */
//==============================================================================
void GFL_NET_DWC_SetVchat(int codec)
{
  _dWork->vchatcodec = codec;

  if( _dWork->isvchat ){
    switch( _dWork->vchatcodec ){
    case VCHAT_G711_ULAW:
      myvct_setCodec( VCT_CODEC_G711_ULAW );
      break;
    case VCHAT_2BIT_ADPCM:
      myvct_setCodec( VCT_CODEC_2BIT_ADPCM );
      break;
    case VCHAT_3BIT_ADPCM:
      myvct_setCodec( VCT_CODEC_3BIT_ADPCM );
      break;
    case VCHAT_4BIT_ADPCM:
      myvct_setCodec( VCT_CODEC_4BIT_ADPCM );
      break;
    case VCHAT_8BIT_RAW:
      myvct_setCodec( VCT_CODEC_8BIT_RAW );
      break;
    }
  }
}

//==============================================================================
/**
 * @brief   ボイスチャットを停止します
 * @param   void
 * @retval  void
 */
//==============================================================================

void GFL_NET_DWC_StopVChat(void)
{
  myvct_free();
  if(_dWork != NULL){
    _dWork->isvchat = 0;
    _dWork->backupBitmap = 0;
    _dWork->myvchaton = 0;
  }
}

//==============================================================================
/**
 * @brief   ボイスチャットの一時停止＋解除を行います
 * @param   bPause   一時停止=TRUE  解除=FALSE
 * @retval  void
 */
//==============================================================================

void GFL_NET_DWC_pausevchat(BOOL bPause)
{
  if(bPause){
    myvct_DelConference(DWC_GetMyAID());
    _dWork->pausevchat = TRUE;
  }
  else{
    _dWork->pausevchat = FALSE;
  }
}



//==============================================================================
/**
 * DWCエラー番号に対応するメッセージのIDを返します。
 * @param code - エラーコード（正の値を入れてください
 * @retval メッセージタイプ
 */
//==============================================================================
int GFL_NET_DWC_ErrorType(int code, int type)
{
  int code100 = code / 100;
  int code1000 = code / 1000;

  if( code == 20101 ) return 1;
  if( code1000 == 23 ) return 1;
  if( code == 20108 ) return 2;
  if( code == 20110 ) return 3;
  if( code100 == 512 ) return 4;
  if( code100 == 500 ) return 5;
  if( code == 51103 ) return 7;
  if( code100 == 510 ) return 6;
  if( code100 == 511 ) return 6;
  if( code100 == 513 ) return 6;
  if( code >= 52000 && code <= 52003 ) return 8;
  if( code >= 52100 && code <= 52103 ) return 8;
  if( code >= 52200 && code <= 52203 ) return 8;
  if( code == 80430 ) return 9;

  if( code1000 == 20 ) return 0;
  if( code100 == 520 ) return 0;
  if( code100 == 521 ) return 0;
  if( code100 == 522 ) return 0;
  if( code100 == 523 ) return 0;
  if( code100 == 530 ) return 0;
  if( code100 == 531 ) return 0;
  if( code100 == 532 ) return 0;

  if( code < 10000 ) return 14; // エラーコードが１００００以下の場合は、エラーコードの表示の必要がない。

  if( code1000 == 31 ) return 12; // ダウンロード失敗
  // その他のエラーはTYPEで判断

  switch(type){
  case DWC_ETYPE_NO_ERROR:
  case DWC_ETYPE_LIGHT:
  case DWC_ETYPE_SHOW_ERROR:
    return 11;
  case DWC_ETYPE_SHUTDOWN_FM:
  case DWC_ETYPE_SHUTDOWN_GHTTP:
  case DWC_ETYPE_SHUTDOWN_ND:
  case DWC_ETYPE_DISCONNECT:
    return 10;
  default:
    GF_ASSERT(0); //DWCが返すtypeが増えているなら見直しが必要
    break;
  case DWC_ETYPE_FATAL:
    return 15;
  }
  return -1;
}

//==============================================================================
/**
 * 通信を切断します。
 * @param sync … 0 = 自分から切断しにいく。1 = 相手が切断するのを待つ。
 * @retval 　　　 1 = 接続処理完了。0 = 切断処理中。
 */
//==============================================================================
int GFL_NET_DWC_disconnect( int sync )
{
  if( sync == 0 ){
    MYDWC_DEBUGPRINT(" mydwc_disconnect state %d \n",_dWork->state);
    switch( _dWork->state )	{
    case MDSTATE_MATCHING:   // k.ohno 06.07.08  追加
    case MDSTATE_MATCHED:
    case MDSTATE_PLAYING:
      if( _dWork->isvchat ){
        MYDWC_DEBUGPRINT("ボイスチャット稼働中 止める\n");
        myvct_endConnection();
      }
      _CHANGE_STATE(MDSTATE_DISCONNECTTING);
//      _dWork->state = MDSTATE_DISCONNECTTING;
      break;
    case MDSTATE_LOGIN:     //親機切断時に動きを合わせるために追加 k.ohno 06.07.04
    case MDSTATE_ERROR_DISCONNECT:
    case MDSTATE_DISCONNECT:
    case MDSTATE_TIMEOUT:
      return 1;
    }
  }
  else {
    switch( _dWork->state ) {
    case MDSTATE_LOGIN:     //親機切断時に動きを合わせるために追加 k.ohno 06.07.04
    case MDSTATE_DISCONNECT:
    case MDSTATE_TIMEOUT:
      return 1;
    }
  }
  return 0;
}

//==============================================================================
/**
 * 通信が切断した後、この関数を呼ぶことで内部状態をログイン直後の状態にします。
 * @param 	nonte
 * @retval  1 = 成功。0 = 失敗。
 */
//==============================================================================
int GFL_NET_DWC_returnLobby()
{
  if( _dWork->state == MDSTATE_DISCONNECT || _dWork->state == MDSTATE_TIMEOUT || _dWork->state == MDSTATE_LOGIN) {
    //        _dWork->op_aid = -1;
    _CHANGE_STATE(MDSTATE_LOGIN);
//    _dWork->state = MDSTATE_LOGIN;
    _dWork->newFriendConnect = -1;
    GFL_NET_DWC_ResetClientBlock();
    return 1;
  }
  return 0;
}

//==============================================================================
/**
 * fetal error発生時に呼ばれる関数、このコールバックの中で処理をとめて下さい。
 * @param 	nonte
 * @retval  1 = 成功。0 = 失敗。
 */
//==============================================================================
void GFL_NET_DWC_setFetalErrorCallback( void (*func)(int) )
{
  if(_dWork){
    _dWork->fetalErrorCallback = func;
  }
}



//-----2006.04.11 k.ohno
//==============================================================================
/**
 * ログアウトする
 * @param 	none
 * @retval  none
 */
//==============================================================================

void GFL_NET_DWC_Logout(void)
{
  if(_dWork){
    MYDWC_DEBUGPRINT("----------------------ok LOGOUT  \n");
    DWC_ShutdownFriendsMatch();
    DWC_CleanupInet();
    GFL_NET_DWC_StopVChat();
    GFL_NET_DWC_free();
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief   １フレームにFRIENDINFO_UPDATA_PERFRAME人分のデータを更新する
 * @param 	none
 * @retval  none
 */
//-------------------------------------------------------------------------------

static void mydwc_updateFriendInfo( void )
{
  int i;

  for(i = 0; i < FRIENDINFO_UPDATA_PERFRAME; i++)
  {
    int index = _dWork->friendupdate_index % FRIENDLIST_MAXSIZE;
    int size;

    if( DWC_IsBuddyFriendData( &(_dWork->pFriendData[index]) ) ){
      _dWork->friend_status[index] = DWC_GetFriendStatusData(&_dWork->pFriendData[ index ],(char*)_dWork->friendinfo[index],&size);
#ifdef PM_DEBUG
      GF_ASSERT( (size <= MYDWC_STATUS_DATA_SIZE_MAX) || (size == -1));
#endif
    }
    _dWork->friendupdate_index = (_dWork->friendupdate_index + 1);
  }
}

//==============================================================================
/**
 * 自分の状態をサーバにアップします。
 * @param 	data - データへのポインタ
 * @param 	size  - データサイズ
 * @retval  成功の可否
 */
//==============================================================================
BOOL GFL_NET_DWC_SetMyInfo( const void *data, int size )
{
  MYDWC_DEBUGPRINT("upload status change(%p, %d)\n", data, size);

  GF_ASSERT(size <= MYDWC_STATUS_DATA_SIZE_MAX);
  return DWC_SetOwnStatusData( data, size );
}

//==============================================================================
/**
 * 友達がサーバにアップしている情報を取得します（ローカルにキャッシュしてあるものを表示します）。
 * @param 	index フレンドリスト上の番号
 * @retval  データへのポインタ。中身は書き換えないで下さい。
 */
//==============================================================================
void* GFL_NET_DWC_GetFriendInfo( int index )
{
  return (void*)&_dWork->friendinfo[index];
}

//==============================================================================
/**
 * 友達がサーバにアップしている情報を取得します（ローカルにキャッシュしてあるものを表示します）。
 * @param 	index フレンドリスト上の番号
 * @retval  友達の状態。DWC_GetFriendStatusDataの返り値と同じ
 */
//==============================================================================
u8 GFL_NET_DWC_getFriendStatus( int index )
{
  return _dWork->friend_status[index];
}


static void SetupGameServerCallback(DWCError error, BOOL cancel, BOOL self, BOOL isServer, int  index, void* param);
static void ConnectToGameServerCallback(DWCError error, BOOL cancel, BOOL self, BOOL isServer, int  index, void* param);
static void NewClientCallback(int index, void* param);

//==============================================================================
/**
 * ゲーム募集・参加を行う関数。
 * @target   負…自分でゲームを開催。０以上…接続しに行く相手の、フレンドリスト上の位置
 * @retval  正…成功。０…失敗。
 */
//==============================================================================
int GFL_NET_DWC_StartGame( int target,int maxnum, BOOL bVCT )
{
  int ans,num = maxnum;

  if(GFL_NET_DWC_GetSaving()){
    return DWCRAP_STARTGAME_FIRSTSAVE;
  }

  if( _dWork->state != MDSTATE_LOGIN ){
    _dWork->setupErrorCount++;
    if(_dWork->setupErrorCount>120){
      return DWCRAP_STARTGAME_FAILED;
    }
    return DWCRAP_STARTGAME_NOTSTATE;
  }
  mydwc_releaseRecvBuffAll();
  _dWork->BlockUse_BackupBitmap = 0; //080703 ohno
  _dWork->closedflag = TRUE;	// 080602	tomoya
  _dWork->friendindex = target;
  _dWork->maxConnectNum = maxnum;
  //    OHNO_PRINT("max %d\n",_dWork->maxConnectNum);
  if(bVCT){
    num=2;
  }
  _dWork->bConnectCallback = TRUE;
  if ( target < 0 ){
    ans = DWC_SetupGameServer(DWC_TOPOLOGY_TYPE_FULLMESH,
                              (u8)num,
                              SetupGameServerCallback, GFL_NET_GetWork(),
                              NewClientCallback, GFL_NET_GetWork(),
                              attemptCallback,
                              (u8*)&_dWork->connectionUserData,GFL_NET_GetWork()
                              );
    _dWork->matching_type = MDTYPE_PARENT;
  } else {
    ans = DWC_ConnectToGameServerAsync(DWC_TOPOLOGY_TYPE_FULLMESH,
                                       target,ConnectToGameServerCallback,NULL,NewClientCallback,GFL_NET_GetWork(),
                                       attemptCallback,
                                       (u8*)&_dWork->connectionUserData,
                                       GFL_NET_GetWork()
                                       );
    _dWork->matching_type = MDTYPE_CHILD;
  }
  if(!ans){
    _dWork->setupErrorCount++;
    if(_dWork->setupErrorCount>120){
      return DWCRAP_STARTGAME_FAILED;
    }
    return DWCRAP_STARTGAME_RETRY;
  }
  _dWork->setupErrorCount = 0;//リセットしておく

  {
    int i;
    for(i=0;i<maxnum; i++){
      mydwc_allocRecvBuff(i);
    }
  }
  _CHANGE_STATE(MDSTATE_MATCHING);
  //_dWork->state = MDSTATE_MATCHING;

  // 送信コールバックの設定
  DWC_SetUserSendCallback( SendDoneCallback,NULL );

  // 受信コールバックの設定
  DWC_SetUserRecvCallback( UserRecvCallback,NULL );

  // コネクションクローズコールバックを設定
  DWC_SetConnectionClosedCallback(ConnectionClosedCallback, NULL);

  // タイムアウトコールバックの設定
  DWC_SetUserRecvTimeoutCallback( recvTimeoutCallback,NULL );

  _dWork->sendbufflag = 0;

  return DWCRAP_STARTGAME_OK;
}

//==============================================================================
/**
 * 今接続している友達のフレンドリスト上の位置を返します。
 * まだ、接続が完了していなくても友達番号を返すことがありますので、接続が
 * 完了したかの判定には使わないでください。
 * @retval  ０以上…友達番号。－１…まだ接続してない。
 */
//==============================================================================
int GFL_NET_DWC_GetFriendIndex(void)
{
  if(_dWork){
    // 今接続している友達のフレンドリスト上の位置を返します。
    return _dWork->friendindex;
  }
  return -1;
}

/*---------------------------------------------------------------------------*
  ゲームサーバ起動コールバック関数
 *---------------------------------------------------------------------------*/
static void SetupGameServerCallback(DWCError error,
                                    BOOL cancel,
                                    BOOL self,
                                    BOOL isServer,
                                    int  index,
                                    void* param)
{
#pragma unused(isServer, param)
  BOOL bFriendOnly = FALSE;

  _dWork->bConnectCallback = FALSE;
  if (error == DWC_ERROR_NONE){
    if (!cancel){
      // ネットワークに新規クライアントが加わった
      MYDWC_DEBUGPRINT("友達が接続してきました。（インデックス＝%d）\n", index);
      // 接続が確立したときのみ、
      // 代入する形に修正
      // 080624
      // BTS通信630の対処	tomoya
      //			_dWork->friendindex = index;

      //            if(CommLocalIsWiFiFriendGroup(CommStateGetServiceNo()) && (index==-1)){
      if(_dWork->bWiFiFriendGroup && (index == -1)){
        bFriendOnly = TRUE;
      }

      if(_dWork->connectModeCheck){
        if(FALSE == _dWork->connectModeCheck(index,GFL_NET_GetWork())){
          OS_TPrintf("切断WIFIP2PModeCheck \n");
          bFriendOnly = TRUE;
        }
      }


      if (_dWork->blockClient || bFriendOnly){
        if(index!=-1){
          DWC_CloseConnectionHardFromServer(index);
/*
このクローズは相手ホストにも通知され、相手ホストではクローズコールバック
DWCConnectionClosedCallbackが呼び出されます。
ただし、このクローズ通知はUDP通信で一度しか送信されないため、
通信路の状況などによっては相手に届かない可能性もあります。
 */
        }
      }

      
      // 接続が確立したときのみ、
      // 代入する形に修正
      // 080624
      // BTS通信630の対処	tomoya
      _dWork->friendindex = index;


      _dWork->BlockUse_BackupBitmap = DWC_GetAIDBitmap();
      if(_dWork->BlockUse_BackupBitmap==0x01){ //自分だけつながったときにはタイムアウトにする
    _CHANGE_STATE(MDSTATE_CANCEL);
    //    _dWork->state = MDSTATE_CANCEL;
        MYDWC_DEBUGPRINT("自分タイムアウト %x\n",_dWork->BlockUse_BackupBitmap);
      }
      else{
        setTimerAndFlg(index);
      }
    }
    else
    {
      if (self){
        // 自分がマッチングをキャンセルした
        MYDWC_DEBUGPRINT("マッチングのキャンセルが完了しました。\n\n");
        // ログイン後状態に戻る
        //				s_dwcstate = MYDWCSTATE_MATCH_CANCELFINISH;
      }
      else {
        // 繋いでいた子機が、接続をキャンセルした。
        MYDWC_DEBUGPRINT("Client (friendListIndex = %d) canceled matching.\n\n", index);
        // 繋いできた子機がいなくなった。2006.7.3 yoshihara
        _dWork->newFriendConnect = -1;
      }
    }
  }
  else {
    // エラーが発生した。エラーコードは、step関数の中で拾う。
    MYDWC_DEBUGPRINT("SetupGame server error occured. %d\n\n", error);

    //        s_dwcstate = MYDWCSTATE_MATCH_CANCELFINISH;
  }
}

/*---------------------------------------------------------------------------*
  ゲームサーバ接続コールバック関数
 *---------------------------------------------------------------------------*/
static void ConnectToGameServerCallback(DWCError error,
                                        BOOL cancel,
                                        BOOL self,
                                        BOOL isServer,
                                        int  index,
                                        void* param)
{
#pragma unused(param)
  _dWork->bConnectCallback = FALSE;
  if (error == DWC_ERROR_NONE){
    if (!cancel){

      if (self){
        // 自分がゲームサーバとそこにできているネットワークへの接続に
        // 成功した場合
        MYDWC_DEBUGPRINT("接続に成功しました\n");
      }
      else {
        // ネットワークに新規クライアントが加わった場合。
        // 二人対戦限定なので、ここにはこないはず。
        MYDWC_DEBUGPRINT("新規につながりました\n");
      }
      setTimerAndFlg(index);
    }
    else {
      if (self){
        // 自分がマッチングをキャンセルした
        MYDWC_DEBUGPRINT("キャンセルが完了しました。\n\n");
        // ログイン後状態に戻る
        //                s_dwcstate = MYDWCSTATE_MATCH_CANCELFINISH;
      }
      else {
        if (isServer){
          // ゲームサーバがマッチングをキャンセルした
          MYDWC_DEBUGPRINT("親が接続をキャンセルしました\n\n");
          // マッチングを終了してログイン後状態に戻る
          //                    s_dwcstate = MYDWCSTATE_MATCH_CANCELFINISH;
        }
        else {
          MYDWC_DEBUGPRINT("子が接続をキャンセルしました\n\n");
          // 他のクライアントがマッチングをキャンセルした。
          // 二人対戦ならここにこないはず。
        }
      }
    }
  }
#ifdef PM_DEBUG
  else {
    // エラーが発生した。エラーコードは、step関数の中で拾う。
    MYDWC_DEBUGPRINT("ConnectGame server error occured. %d\n\n", error);
    {
      int errorCode;
      DWCErrorType myErrorType;
      int ret = DWC_GetLastErrorEx( &errorCode, &myErrorType );
      if( ret != 0 ){
        MYDWC_DEBUGPRINT("error occured!(%d, %d, %d)\n", ret, errorCode, myErrorType);
      }
    }
    // もう一度最初から。おいおい処理
    //        s_dwcstate = MYDWCSTATE_MATCH_ERRORFINISH;
  }
#endif

  //    s_blocking = 0;
}

/*---------------------------------------------------------------------------*
  サーバクライアント型マッチング時の新規接続クライアント通知コールバック関数
 *---------------------------------------------------------------------------*/
static void NewClientCallback(int index, void* param)
{
#pragma unused(param)
  // 新規接続クライアントの接続処理が終わるまでは、
  // コントローラ操作を禁止する
  //    s_blocking = 1;

  _dWork->newFriendConnect = index;

  MYDWC_DEBUGPRINT("新しい人が接続してきました。\n");
  if (index != -1){
    MYDWC_DEBUGPRINT("友達[%d].\n", index);
    //VCTカンファレンス開始

  }
  else {
    MYDWC_DEBUGPRINT("友達ではない.\n");
  }
  if( _dWork->connectCallback ){  //
    _dWork->connectCallback(index, _dWork->pConnectWork);
  }
}

//==============================================================================
/**
 * 現在のフレンドリストを表示します。（デバッグ用）
 * @param 	none
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_showFriendInfo()
{
  int i;

  if( !DWC_CheckHasProfile( _dWork->pUserData ) )
  {
    DWCFriendData token;
    u32 *ptr;

    DWC_CreateExchangeToken( _dWork->pUserData, &token );
    ptr = (u32*)&token;
    MYDWC_DEBUGPRINT("まだ、プロファイルＩＤ取得前\nログインＩＤ:(%d, %d, %d)\n\n", ptr[0], ptr[1], ptr[2] );
  }
  else
  {
    // 接続済み
    DWCFriendData token;
    DWC_CreateExchangeToken( _dWork->pUserData, &token );
    MYDWC_DEBUGPRINT("プロファイルＩＤ:%d \n\n", DWC_GetGsProfileId( _dWork->pUserData, &token ) );
  }

  for( i = 0; i < FRIENDLIST_MAXSIZE; i++ )
  {
    int ret = DWC_GetFriendDataType( &(_dWork->pFriendData[i]) );
    u32 *ptr = (u32*)(&_dWork->pFriendData[i]);
    switch(ret)
    {
    case DWC_FRIENDDATA_LOGIN_ID:
      MYDWC_DEBUGPRINT("%d:ログインＩＤ:(%d, %d, %d)",i, ptr[0], ptr[1], ptr[2] );
      break;

    case DWC_FRIENDDATA_FRIEND_KEY:
      MYDWC_DEBUGPRINT("%d:フレンドコード:(%d)", i, DWC_GetGsProfileId( _dWork->pUserData, &_dWork->pFriendData[i] ) );
      break;

    case DWC_FRIENDDATA_GS_PROFILE_ID:
      MYDWC_DEBUGPRINT("%d:プロファイルＩＤ:(%d)", i, DWC_GetGsProfileId( _dWork->pUserData, &_dWork->pFriendData[ i ]) );
      break;

    case DWC_FRIENDDATA_NODATA:
    default:
      MYDWC_DEBUGPRINT("%d:空", i);
      break;
    }

    if( DWC_IsBuddyFriendData( &(_dWork->pFriendData[i]) ) )
    {
      MYDWC_DEBUGPRINT("(両思い)");
    }
    MYDWC_DEBUGPRINT("\n");
  }
}


// 送信したかどうかを返します
BOOL GFL_NET_DWC_IsSendVoiceAndInc(void)
{
  return myvct_IsSendVoiceAndInc();
}


//==============================================================================
/**
 * ボイスチャット状態かどうかを返します   k.ohno 06.05.23 07.22 フラグを交換
 * @retval  TRUE…ボイスチャット   FALSE…ボイスチャットではない
 */
//==============================================================================
BOOL GFL_NET_DWC_IsVChat(void)
{
  if(_dWork){
    return _dWork->bVChat;
  }
  return FALSE;
}

//==============================================================================
/**
 * 接続してきたかどうかを返します    k.ohno 06.05.24
 * @retval  TRUE…接続開始なのでキー操作をブロック   FALSE…
 */
//==============================================================================
BOOL GFL_NET_DWC_IsNewPlayer(void)
{
  if(_dWork){
    return _dWork->newFriendConnect;
  }
  return FALSE;
}

//==============================================================================
/**
 * 接続してきたかどうかフラグを落とします k.ohno 06.08.04
 * @retval  none
 */
//==============================================================================
void GFL_NET_DWC_ResetNewPlayer(void)
{
  if(_dWork){
    _dWork->newFriendConnect = -1;
  }
}

//==============================================================================
/**
 * VCHATのONOFF     k.ohno 06.05.24
 */
//==============================================================================
void GFL_NET_DWC_SetVChat(BOOL bVChat)
{
  _dWork->bVChat = bVChat;
}

static void sendPacket()
{
  int i;

  if( _dWork->sendbufflag || !_isSendableReliable() ) // 送信バッファをチェック。
  {
    _dWork->sendbufflag = 1;
    *((u32*)&(_dWork->sendBuffer[0])) = MYDWC_KEEPALIVE_PACKET | (_dWork->myvchaton << MYDWC_PACKET_VCHAT_SHIFT);;

    DWC_SendReliableBitmap(DWC_GetAIDBitmap(), &(_dWork->sendBuffer[0]), 4);
    clearTimeoutBuff();
  }
}

void mydwc_VChatPause()
{
  if( _dWork->myvchaton != 0 )
  {
    _dWork->myvchaton = 0;
    sendPacket();
  }
}

void mydwc_VChatRestart()
{
  if( _dWork->myvchaton != 1 )
  {
    _dWork->myvchaton = 1;
    sendPacket();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	VCHATデータ送信設定
 *
 *	@param	flag	TRUE:音声データを送信する	FALSE:音声データを送信しない
 */
//-----------------------------------------------------------------------------
void mydwc_VChatSetSend( BOOL flag )
{
  _dWork->myvchat_send = flag;
}
BOOL mydwc_VChatGetSend( void )
{
  return _dWork->myvchat_send;
}


//  _dWork->blockClient = _BLOCK_CALLBACK;

static void _blockCallback(DWCSuspendResult result, BOOL suspend, void *data)
{
  if(DWC_SUSPEND_SUCCESS == result){
    if(suspend){
      _dWork->blockClient = _BLOCK_CALLBACK;
    }
    else{
      _dWork->blockClient = _BLOCK_NONE;
    }
  }
}

   


//==============================================================================
/**
 * @brief   クライアント接続を一時的に中止します
 * @param   none
 * @retval  none
 */
//==============================================================================
BOOL GFL_NET_DWC_SetClientBlock(void)
{
#if 0
  if(_dWork->blockClient==_BLOCK_NONE){
    _dWork->blockClient = _BLOCK_START;
    DWC_StopSCMatchingAsync(_blockCallback,NULL);
  }
  return (_dWork->blockClient==_BLOCK_CALLBACK);
#endif


  return DWC_RequestSuspendMatchAsync(TRUE,
                                      _blockCallback,
                                      NULL);

}

//==============================================================================
/**
 * @brief   クライアント接続を許可に戻します
 * @param   none
 * @retval  none
 */
//==============================================================================
BOOL GFL_NET_DWC_ResetClientBlock(void)
{
  return DWC_RequestSuspendMatchAsync(FALSE,
                                      _blockCallback,
                                      NULL);
//  _dWork->blockClient = _BLOCK_NONE;
}

//==============================================================================
/**
 * @brief   評価状態中の接続数を返す
 * @param   none
 * @retval  数
 */
//==============================================================================
int GFL_NET_DWC_AnybodyEvalNum(void)
{
  return DWC_GetNumConnectionHost();
}


//----------------------------------------------------------------------------
/**
 *	@brief	通信クローズ処理で、切断処理に遷移するのかのフラグ
 *
 *	@param	flag	フラグ		TRUEなら切断処理に遷移する（初期値TRUE）
 *
 * *Wi-Fiクラブ４人募集画面用に作成
 */
//-----------------------------------------------------------------------------
void GFL_NET_DWC_SetClosedDisconnectFlag( BOOL flag )
{
  _dWork->closedflag = flag;
  MYDWC_DEBUGPRINT( "_dWork->closedflag = %d\n", flag );
}

//==============================================================================
/**
 * @brief   受信バッファ開放
 * @param
 * @retval  none
 */
//==============================================================================

void mydwc_releaseRecvBuff(int aid)
{
  if(_dWork->recvPtr[aid]!=NULL){
    //OHNO_PRINT("_SetRecvBufferメモリ開放 %d\n",aid);
    GFL_NET_Align32Free(_dWork->recvPtr[aid]);
    _dWork->recvPtr[aid]=NULL;
  }
}

//==============================================================================
/**
 * @brief   受信バッファ確保
 * @param
 * @retval  none
 */
//==============================================================================

void mydwc_allocRecvBuff(int i)
{
  GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
//	GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

  mydwc_releaseRecvBuff(i);

  if(_dWork->recvPtr[i]==NULL){
    NET_PRINT("_SetRecvBufferメモリ確保 %d\n",i);
//    _dWork->recvPtr[i] = GFL_NET_Align32Alloc(pNetInit->wifiHeapID, SIZE_RECV_BUFFER);
    _dWork->recvPtr[i] = GFL_NET_Align32Alloc(pNetInit->netHeapID, SIZE_RECV_BUFFER);
    DWC_SetRecvBuffer( i, _dWork->recvPtr[i], SIZE_RECV_BUFFER );
  }
}

//==============================================================================
/**
 * @brief   受信バッファすべて開放
 * @param   TRUEならワールドから確保
 * @retval  none
 */
//==============================================================================

void mydwc_releaseRecvBuffAll(void)
{
  int i;

  for(i = 0 ; i < _WIFI_NUM_MAX ; i++){
    mydwc_releaseRecvBuff(i);
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief	セーブして良いかどうか確認するフラグ
 *	@param	TRUE セーブしてよい
 */
//-----------------------------------------------------------------------------
u8 GFL_NET_DWC_GetSaving(void)
{
  return _dWork->saveing;  //セーブ中に1
}

//----------------------------------------------------------------------------
/**
 *	@brief	セーブして良いかどうか確認するフラグを消す
 */
//-----------------------------------------------------------------------------
void GFL_NET_DWC_ResetSaving(void)
{
  _dWork->saveing = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャンセル処理を行って良いかどうか
 *	@param	FALSE キャンセルしてよい TRUEキャンセル禁止
 */
//-----------------------------------------------------------------------------
BOOL GFL_NET_DWC_CancelDisable(void)
{
  return _dWork->bConnectCallback;
}

//----------------------------------------------------------------------------
/**
 *	@brief	LOGIN状態かどうか
 *	@param	TRUE LOGIN状態である
 */
//-----------------------------------------------------------------------------
BOOL GFL_NET_DWC_IsLogin(void)
{
  if(_dWork){
    return ( _dWork->state == MDSTATE_LOGIN );
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セーブ中でない時の処理
 *	@param	
 */
//-----------------------------------------------------------------------------
static void _FuncNonSave(void)
{
  int i;
  
  
  for(i = 0;i < FRIENDLIST_MAXSIZE; i++){
    if(_dWork->deletedIndex[i] != -1){
      GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
      if(pNetInit->friendDeleteFunc){
        pNetInit->friendDeleteFunc( _dWork->deletedIndex[i], _dWork->srcIndex[i], GFL_NET_GetWork());
      }
      _dWork->deletedIndex[i] = -1;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   GFL_NET_DWC_stepmatchを改造しmain処理としての機能だけを独立
 * @retval  none
 */
//--------------------------------------------------------------
void GFL_NET_DWC_MainStep( void )
{
  GFL_NET_DWC_stepmatch( 0 );
}


//--------------------------------------------------------------
/**
 * @brief   キャンセル状態に移行する
            前回ではVCTの状態を終了する為に頻繁に用いられているが、
            マッチングをキャンセルする為の物なので、全部がキャンセルできるわけではない
 * @retval  キャンセル状態に移行したらTRUE
 */
//--------------------------------------------------------------

BOOL GFL_NET_DWC_SetCancelState(void)
{
  if(_dWork->state == MDSTATE_MATCHING)
  {
    _CHANGE_STATE(MDSTATE_CANCEL);
//    _dWork->state = MDSTATE_CANCEL;
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   ステップマッチの結果を得る
 * @retval  ステップマッチの結果
 	          STEPMATCH_CONTINUE…マッチング中
 	          STEPMATCH_SUCCESS…成功
            STEPMATCH_CANCEL…キャンセル
            STEPMATCH_FAIL  …相手が親をやめたため、接続を中断
 */
//--------------------------------------------------------------

int GFL_NET_DWC_GetStepMatchResult(void)
{
  return _dWork->stepMatchResult;
}

//------------------------------------------------------------------------------
/**
 * @brief   子機がつながってよいかどうかハードレベルで調整する
 * @param   bEnable TRUE=許可
 * @retval  none
 */
//------------------------------------------------------------------------------

void GFL_NET_DWC_SetClinetConnect(BOOL bEnable)
{
  _dWork->bConnectEnable = bEnable;
}


//------------------------------------------------------------------------------
/**
 * @brief   接続する時にわたってくるIDを設定できる
 * @param   data u32型のID
 * @retval  none
 */
//------------------------------------------------------------------------------

void GFL_NET_DWC_SetCconnectionUserData(u32 data)
{
  _dWork->connectionUserData = data;
}






#endif //GFL_NET_WIFI
