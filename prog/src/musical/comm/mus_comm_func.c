//======================================================================
/**
 * @file	mus_comm_func.c
 * @brief	ミュージカル通信機能
 * @author	ariizumi
 * @data	09/05/29
 *
 * モジュール名：MUS_COMM_FUNC
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "net/network_define.h"

#include "mus_comm_func.h"
#include "mus_comm_define.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MUS_COMM_BEACON_MAX (4)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MCST_FLG,
  
  MCST_MAX,
}MUS_COMM_SEND_TYPE;

//通信状態
typedef enum
{
  MCS_NONE,

  MCS_PARENT_REQ_NEGOTIATION,
  MCS_PARENT_WAIT_NEGOTIATION,
  MCS_PARENT_COLLECT,

  MCS_CHILD_SEARCH,
  MCS_CHILD_REQ_NEGOTIATION,
  MCS_CHILD_WAIT_NEGOTIATION,
  MCS_CHILD_CONNECT,
  
}MUS_COMM_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

struct _MUS_COMM_WORK
{
  HEAPID heapId;
  
  BOOL isInitComm;
  BOOL isRefreshUserData;
  u8   befMemberNum;
  
  MUS_COMM_BEACON beacon;
  MUS_COMM_MODE mode;   //親か子か？
  MUS_COMM_STATE commState; //もっと細かい分岐

  GAME_COMM_SYS_PTR gameComm;
  MYSTATUS *myStatus;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void MUS_COMM_FinishNetInitCallback( void* pWork );
static void MUS_COMM_FinishNetTermCallback( void* pWork );

//static const BOOL MUS_COMM_Send_Flag( MUS_COMM_WORK *work );
static void MUS_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static void* MUS_COMM_GetUserData(void* pWork);
static int MUS_COMM_GetUserDataSize(void* pWork);
static void MUS_COMM_CheckUserData( MUS_COMM_WORK *work );

static void*  MUS_COMM_GetBeaconData(void* pWork);
static int MUS_COMM_GetBeaconSize(void *pWork);
static BOOL MUS_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2);

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState );

static const NetRecvFuncTable MusCommRecvTable[MCST_MAX] = 
{
  { MUS_COMM_Post_Flag   ,NULL  },
};

//--------------------------------------------------------------
//	ワーク作成
//--------------------------------------------------------------
MUS_COMM_WORK* MUS_COMM_CreateWork( HEAPID heapId , GAME_COMM_SYS_PTR gameComm , SAVE_CONTROL_WORK *saveCtrl )
{
  MUS_COMM_WORK* work = GFL_HEAP_AllocClearMemory( heapId , sizeof( MUS_COMM_WORK ));
  work->gameComm = gameComm;
  work->mode = MCM_NONE;
  work->isInitComm = FALSE;
  work->myStatus = SaveData_GetMyStatus(saveCtrl);

  MUS_COMM_SetCommState( work , MCS_NONE);

  return work;
}

//--------------------------------------------------------------
// ワーク削除
//--------------------------------------------------------------
void MUS_COMM_DeleteWork( MUS_COMM_WORK* work )
{
  if( work->isInitComm == TRUE )
  {
    GFL_NET_Exit(NULL);
  }

  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
// 通信開始
//--------------------------------------------------------------
void MUS_COMM_InitComm( MUS_COMM_WORK* work )
{
  GFLNetInitializeStruct aGFLNetInit = 
  {
    MusCommRecvTable, //NetSamplePacketTbl,  // 受信関数テーブル
    NELEMS(MusCommRecvTable), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL, ///< ネゴシエーション完了時にコール
    MUS_COMM_GetUserData, // ユーザー同士が交換するデータのポインタ取得関数
    MUS_COMM_GetUserDataSize, // ユーザー同士が交換するデータのサイズ取得関数
    MUS_COMM_GetBeaconData,   // ビーコンデータ取得関数  
    MUS_COMM_GetBeaconSize,   // ビーコンデータサイズ取得関数 
    MUS_COMM_BeacomCompare, // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL, // 通信不能なエラーが起こった場合呼ばれる
    NULL, //FatalError
    NULL, // 通信切断時に呼ばれる関数(終了時
    NULL,                   // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,                   ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL,                   ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,                   ///< wifiフレンドリスト削除コールバック
    NULL,                   ///< DWC形式の友達リスト  
    NULL,                   ///< DWCのユーザデータ（自分のデータ）
    0,                      ///< DWCへのHEAPサイズ
    TRUE,                   ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x333,                  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,         //元になるheapid
    HEAPID_NETWORK,         //通信用にcreateされるHEAPID
    HEAPID_WIFI,            //wifi用にcreateされるHEAPID
    HEAPID_NETWORK,         //
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    MUSICAL_COMM_MEMBER_NUM,        //_MAXNUM,  //最大接続人数
    48,                     //_MAXSIZE, //最大送信バイト数
    MUS_COMM_BEACON_MAX,    //_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,                   // CRC計算
    FALSE,                  // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,  //通信タイプの指定
    TRUE,                   // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_MUSICAL,     //GameServiceID
#if GFL_NET_IRC
    IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  };  
  
  work->befMemberNum = 0;
  work->isRefreshUserData = FALSE;
  
  GFL_NET_Init( &aGFLNetInit , MUS_COMM_FinishNetInitCallback , (void*)work );

}

//--------------------------------------------------------------
// 通信終了
//--------------------------------------------------------------
void MUS_COMM_ExitComm( MUS_COMM_WORK* work )
{
  work->mode = MCM_NONE;
  GFL_NET_Exit(MUS_COMM_FinishNetInitCallback);
}

//--------------------------------------------------------------
// 初期化用コールバック
//--------------------------------------------------------------
static void MUS_COMM_FinishNetInitCallback( void* pWork )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  work->isInitComm = TRUE;
}

//--------------------------------------------------------------
// 終了用コールバック
//--------------------------------------------------------------
static void MUS_COMM_FinishNetTermCallback( void* pWork )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  work->isInitComm = FALSE;
}

//--------------------------------------------------------------
// 通信更新
//--------------------------------------------------------------
void MUS_COMM_UpdateComm( MUS_COMM_WORK* work )
{
  switch( work->commState )
  {
    //親機
  case MCS_PARENT_REQ_NEGOTIATION:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
    {
      MUS_COMM_SetCommState( work , MCS_PARENT_WAIT_NEGOTIATION);
    }
    break;
    
  case MCS_PARENT_WAIT_NEGOTIATION:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE )
      {
        MUS_COMM_SetCommState( work , MCS_PARENT_COLLECT);
      }
    }
    break;
    
  case MCS_PARENT_COLLECT:
    MUS_COMM_CheckUserData( work );
    break;

    //子機
  case MCS_CHILD_SEARCH:
    {
      u16 i=0;
      void* beacon;
      do
      {
        beacon = GFL_NET_GetBeaconData( i );
        i++;
      }while( beacon == NULL && i < MUS_COMM_BEACON_MAX );
      
      if( beacon != NULL )
      {
        u8 *macAdd = GFL_NET_GetBeaconMacAddress( i-1 );
        GFL_NET_ConnectToParent( macAdd );
        MUS_COMM_SetCommState( work , MCS_CHILD_REQ_NEGOTIATION);
      }
    }
    break;

  case MCS_CHILD_REQ_NEGOTIATION:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
    {
      MUS_COMM_SetCommState( work , MCS_CHILD_WAIT_NEGOTIATION);
    }
    break;

  case MCS_CHILD_WAIT_NEGOTIATION:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE )
      {
        MUS_COMM_SetCommState( work , MCS_CHILD_CONNECT);
      }
    }
    break;
  case MCS_CHILD_CONNECT:
    MUS_COMM_CheckUserData( work );
    break;

  }
}

//--------------------------------------------------------------
// 親機通信開始
//--------------------------------------------------------------
void MUS_COMM_StartParent( MUS_COMM_WORK* work )
{
  work->mode = MCM_PARENT;
  MUS_COMM_SetCommState(work , MCS_PARENT_REQ_NEGOTIATION);
  GFL_NET_InitServer();
}

//--------------------------------------------------------------
// 子機通信開始
//--------------------------------------------------------------
void MUS_COMM_StartChild( MUS_COMM_WORK* work )
{
  work->mode = MCM_CHILD;
  MUS_COMM_SetCommState( work , MCS_CHILD_SEARCH);
  GFL_NET_StartBeaconScan();
}

#pragma mark [> userData func
//--------------------------------------------------------------
// ユーザーデータ設定
//--------------------------------------------------------------
static void* MUS_COMM_GetUserData(void* pWork)
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  
  //TODO 念のため名前が入ってないときに落ちないようにしておく
  if( MyStatus_CheckNameClear( work->myStatus ) == TRUE )
  {
    u16 tempName[7] = { L'N',L'o',L'N',L'a',L'm',L'e',0xFFFF };
    MyStatus_SetMyName( work->myStatus , tempName );
  }

  
  return work->myStatus;
}

static int MUS_COMM_GetUserDataSize(void* pWork)
{
  return MyStatus_GetWorkSize();
}

static void MUS_COMM_CheckUserData( MUS_COMM_WORK *work )
{
  //新しいメンバーがいるか？
  if( work->befMemberNum != GFL_NET_GetConnectNum() )
  {
    work->befMemberNum = GFL_NET_GetConnectNum();
    work->isRefreshUserData = TRUE;
  }
}

const BOOL MUS_COMM_IsRefreshUserData( MUS_COMM_WORK *work )
{
  return work->isRefreshUserData;
}
void MUS_COMM_ResetRefreshUserData( MUS_COMM_WORK *work )
{
  work->isRefreshUserData = FALSE;
}

//--------------------------------------------------------------
// マイステータス取得
//--------------------------------------------------------------
MYSTATUS* MUS_COMM_GetPlayerMyStatus( MUS_COMM_WORK* work , u8 idx )
{
  if( work->commState != MCS_NONE )
  {
    const GFL_NETHANDLE *handle = GFL_NET_GetNetHandle( idx );
    if( handle == NULL )
    {
      return NULL;
    }
    else
    {
      return (MYSTATUS*)GFL_NET_HANDLE_GetInfomationData( handle );
    }
  }
  return NULL;
}


#pragma mark [> beacon func
//--------------------------------------------------------------
// ビーコンデータ取得関数  
//--------------------------------------------------------------
static void*  MUS_COMM_GetBeaconData(void* pWork)
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  return (void*)&work->beacon;
}

//--------------------------------------------------------------
// ビーコンデータサイズ取得関数 
//--------------------------------------------------------------
static int MUS_COMM_GetBeaconSize(void *pWork)
{
  return sizeof( MUS_COMM_BEACON );
}

//--------------------------------------------------------------
// 接続チェック用比較
//--------------------------------------------------------------
static BOOL MUS_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2)
{
  if( GameServiceID1 == GameServiceID2 )
  {
    return TRUE;
  }
  return FALSE;
}


#pragma mark [> send/post func
static void MUS_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  
}

#pragma mark [> outer func

GAME_COMM_SYS_PTR MUS_COMM_GetGameComm( MUS_COMM_WORK* work )
{
  return work->gameComm;
}

const BOOL MUS_COMM_IsInitComm( MUS_COMM_WORK* work )
{
  return work->isInitComm;
}

MUS_COMM_MODE MUS_COMM_GetMode( MUS_COMM_WORK* work )
{
  return work->mode;
}

#pragma mark [> other func

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState )
{
  ARI_TPrintf("MusCommState[%d]\n",commState);
  work->commState = commState;
}
