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
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "mus_comm_func.h"
#include "mus_comm_define.h"
#include "musical/musical_system.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MUS_COMM_BEACON_MAX (4)
#define MUS_COMM_POKEDATA_SIZE (sizeof(MUSICAL_POKE_PARAM)+POKETOOL_GetWorkSize())

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//送信の種類
typedef enum
{
  MCST_FLG = GFL_NET_CMD_MUSICAL,
  MCST_POKEDATA ,
  MCST_ALL_POKEDATA ,
  MCST_STRMDATA ,
  MCST_PROGRAMDATA ,
  MCST_MESSAGEDATA ,
  MCST_SCRIPTDATA ,
  
  MCST_MAX,
}MUS_COMM_SEND_TYPE;

//Flg送信の種類
typedef enum
{
  MCFT_START_GAME,
  MCFT_MUSICAL_IDX,
  MCFT_GAME_STATE,
  MCFT_GAME_STATE_ALL,
  MCFT_PROGRAM_SIZE,
  MCFT_MESSAGE_SIZE,
  MCFT_SCRIPT_SIZE,
  MCFT_STRM_SIZE,
  MCFT_USE_BUTTON_REQ,    //子機が親機へ使用リクエスト
  MCFT_USE_BUTTON,        //親機が子機へ使用通知
  
}MUS_COMM_FLAG_TYPE;

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

typedef enum
{
  MCDS_NONE,
  MCDS_START,
  MCDS_WAIT_SIZEDATA,
  MCDS_SEND,
  MCDS_WAIT_POST,
  
}MUS_COMM_DIVSEND_STATE;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  BOOL                isValidData;  //pokeData受信完了フラグ
  BOOL                isReqUseButton;
  u8                  useReqButtonPos;
  u8                  useButtonPos;
  u8                  musicalIdx;
  MUS_COMM_GAME_STATE gameState;
  void *pokeData;     //MUSICAL_POKE_PARAMとPOKEMON_PARAMのセット。
}MUS_COMM_USER_DATA;

struct _MUS_COMM_WORK
{
  HEAPID heapId;
  
  BOOL isInitComm;
  BOOL isRefreshUserData;
  BOOL isStartGame;
  u8   befMemberNum;
  
  MUS_COMM_BEACON beacon;
  MUS_COMM_MODE mode;   //親か子か？
  MUS_COMM_STATE commState; //もっと細かい分岐

  GAME_COMM_SYS_PTR gameComm;
  MYSTATUS *myStatus;
  
  MUS_COMM_USER_DATA userData[MUSICAL_COMM_MEMBER_NUM];
  void *selfPokeData; //送信用バッファ
  void *allPokeData;  //送受信用バッファ
  
  MUSICAL_DISTRIBUTE_DATA *distData;
  u8  strmDivIdx;
  BOOL isSendStrmMode;
  BOOL isSendingStrmData;
  BOOL isPostProgramSize;
  BOOL isPostMessageSize;
  BOOL isPostScriptSize;
  BOOL isPostProgramData;
  BOOL isPostMessageData;
  BOOL isPostScriptData;
  MUS_COMM_DIVSEND_STATE divSendState;
  
  u8 useButtonAttentionPoke;
  //親機専用
  BOOL isReqSendState;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void MUS_COMM_FinishNetInitCallback( void* pWork );
static void MUS_COMM_FinishNetTermCallback( void* pWork );

static const BOOL MUS_COMM_Send_Flag( MUS_COMM_WORK* work , const u8 flag , u32 value , NetID target );
static const BOOL MUS_COMM_Send_FlagServer( MUS_COMM_WORK* work , const u8 flag , u32 value , NetID target );
static void MUS_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static void MUS_COMM_Post_MusPokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_MusPokeDataBuff( int netID, void* pWork , int size );
static void MUS_COMM_Post_AllMusPokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_AllMusPokeDataBuff( int netID, void* pWork , int size );

static void MUS_COMM_Update_SendStrmData( MUS_COMM_WORK *work );
static void MUS_COMM_Post_StrmData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_StrmDataBuff( int netID, void* pWork , int size );

const BOOL MUS_COMM_Send_ProgramData( MUS_COMM_WORK *work );
static void MUS_COMM_Post_ProgramData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_ProgramDataBuff( int netID, void* pWork , int size );
const BOOL MUS_COMM_Send_MessageData( MUS_COMM_WORK *work );
static void MUS_COMM_Post_MessageData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_MessageDataBuff( int netID, void* pWork , int size );
const BOOL MUS_COMM_Send_ScriptData( MUS_COMM_WORK *work );
static void MUS_COMM_Post_ScriptData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_ScriptDataBuff( int netID, void* pWork , int size );

static void* MUS_COMM_GetUserData(void* pWork);
static int MUS_COMM_GetUserDataSize(void* pWork);
static void MUS_COMM_CheckUserData( MUS_COMM_WORK *work );

static void*  MUS_COMM_GetBeaconData(void* pWork);
static int MUS_COMM_GetBeaconSize(void *pWork);
static BOOL MUS_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2);

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState );

static const NetRecvFuncTable MusCommRecvTable[] = 
{
  { MUS_COMM_Post_Flag   ,NULL  },
  { MUS_COMM_Post_MusPokeData , MUS_COMM_Post_MusPokeDataBuff },
  { MUS_COMM_Post_AllMusPokeData , MUS_COMM_Post_AllMusPokeDataBuff },
  { MUS_COMM_Post_StrmData , MUS_COMM_Post_StrmDataBuff },
  { MUS_COMM_Post_ProgramData , MUS_COMM_Post_ProgramDataBuff },
  { MUS_COMM_Post_MessageData , MUS_COMM_Post_MessageDataBuff },
  { MUS_COMM_Post_ScriptData , MUS_COMM_Post_ScriptDataBuff },
};

//--------------------------------------------------------------
//	ワーク作成
//--------------------------------------------------------------
MUS_COMM_WORK* MUS_COMM_CreateWork( HEAPID heapId , GAME_COMM_SYS_PTR gameComm , SAVE_CONTROL_WORK *saveCtrl , MUSICAL_DISTRIBUTE_DATA *distData )
{
  u8 i;
  MUS_COMM_WORK* work = GFL_HEAP_AllocClearMemory( heapId , sizeof( MUS_COMM_WORK ));
  
  work->gameComm = gameComm;
  work->mode = MCM_NONE;
  work->isInitComm = FALSE;
  work->selfPokeData = NULL;
  work->allPokeData = NULL;
  work->myStatus = SaveData_GetMyStatus(saveCtrl);
  work->distData = distData;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].pokeData = NULL;
  }

  MUS_COMM_SetCommState( work , MCS_NONE);

  return work;
}

//--------------------------------------------------------------
// ワーク削除
//--------------------------------------------------------------
void MUS_COMM_DeleteWork( MUS_COMM_WORK* work )
{
  u8 i;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].pokeData != NULL )
    {
      GFL_HEAP_FreeMemory( work->userData[i].pokeData );
    }
  }
  if( work->selfPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfPokeData );
  }
  if( work->allPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->allPokeData );
  }

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
    24,                     //_MAXSIZE, //最大送信バイト数
    MUS_COMM_BEACON_MAX,    //_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,                   // CRC計算
    TRUE,                  // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,  //通信タイプの指定
    TRUE,                   // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_MUSICAL,     //GameServiceID
#if GFL_NET_IRC
    IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    512 - 4*24,//MP親最大サイズ 512まで
    0,//dummy
  };  
  
  u8 i;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].musicalIdx = i;
    work->userData[i].gameState = MCGS_NONE;
    work->userData[i].isValidData = FALSE;
    work->userData[i].isReqUseButton = FALSE;
    work->userData[i].useReqButtonPos = MUS_POKE_EQU_INVALID;
    work->userData[i].useButtonPos = MUS_POKE_EQU_INVALID;
    if( work->userData[i].pokeData != NULL )
    {
      GFL_HEAP_FreeMemory( work->userData[i].pokeData );
    }
    work->userData[i].pokeData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_POKEDATA_SIZE );
  }
  if( work->selfPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfPokeData );
  }
  work->selfPokeData = NULL;

  if( work->allPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->allPokeData );
  }
  work->allPokeData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_POKEDATA_SIZE*MUSICAL_COMM_MEMBER_NUM );
  work->befMemberNum = 0;
  work->isRefreshUserData = FALSE;
  work->isStartGame = FALSE;
  work->strmDivIdx = 0;
  work->isSendStrmMode = FALSE;
  work->isPostProgramSize = FALSE;
  work->isPostMessageSize = FALSE;
  work->isPostProgramData = FALSE;
  work->isPostMessageData = FALSE;
  work->divSendState = MCDS_NONE;
  work->isReqSendState = FALSE;
  work->useButtonAttentionPoke = MUSICAL_COMM_MEMBER_NUM;
  
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
  if( work->isReqSendState == TRUE )
  {
    BOOL ret;
    //親が子機に全員のステートを送る
    u32 value = 0;
    value += work->userData[0].gameState;
    value += work->userData[1].gameState<<4;
    value += work->userData[2].gameState<<8;
    value += work->userData[3].gameState<<12;
    
    ret = MUS_COMM_Send_FlagServer( work , MCFT_GAME_STATE_ALL , value , GFL_NET_SENDID_ALLUSER );    
    if( ret == TRUE )
    {
      work->isReqSendState = FALSE;
    }
  }
  //ボタン使用リクエスト送信
  {
    u32 sendBit = 0;
    u8 i;
    u8 useNum = 0;
    u8 useArr[MUSICAL_COMM_MEMBER_NUM];
    for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
    {
      if( work->userData[i].isReqUseButton == TRUE )
      {
        useArr[useNum] = i;
        useNum++;
      }
      sendBit += (work->userData[i].useReqButtonPos<<(i*4));
    }
    if( useNum > 0 )
    {
      BOOL ret;
      u8 target = useArr[ GFL_STD_MtRand0(useNum)];
      sendBit += (target<<MUSICAL_COMM_MEMBER_NUM*4);
      ret = MUS_COMM_Send_FlagServer( work , MCFT_USE_BUTTON , sendBit , GFL_NET_SENDID_ALLUSER );    
      if( ret == TRUE )
      {
        for( i=0;i<useNum;i++ )
        {
          work->userData[useArr[i]].isReqUseButton = FALSE;
          work->userData[useArr[i]].useReqButtonPos = MUS_POKE_EQU_INVALID;
        }
      }
    }
  }
  MUS_COMM_Update_SendStrmData( work );
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

//--------------------------------------------------------------
// ゲーム開始(新規接続禁止とフラグの送信
//--------------------------------------------------------------
const BOOL MUS_COMM_StartGame( MUS_COMM_WORK* work )
{
  if( work->mode != MCM_PARENT )
  {
    GF_ASSERT_MSG(0, "MusComm mode is not parent[%d]\n",work->mode );
    return FALSE;
  }
  {
    GFL_NETHANDLE *handle = GFL_NET_HANDLE_GetCurrentHandle();
    GFL_NET_SetClientConnect( handle , FALSE );
   
    return MUS_COMM_Send_FlagServer( work , MCFT_START_GAME , 0 , GFL_NET_SENDID_ALLUSER );
  }
}
const BOOL MUS_COMM_IsStartGame( MUS_COMM_WORK* work )
{
  return work->isStartGame;
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

const BOOL MUS_COMM_SetCommGameState( MUS_COMM_WORK *work , MUS_COMM_GAME_STATE state )
{
  return MUS_COMM_Send_Flag( work , MCFT_GAME_STATE , state , GFL_NET_SENDID_ALLUSER );
}


//--------------------------------------------------------------
// タイミングコマンド
//--------------------------------------------------------------
void MUS_COMM_SendTimingCommand( MUS_COMM_WORK *work , const u8 no )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  GFL_NET_HANDLE_TimeSyncStart( selfHandle , no , WB_NET_MUSICAL );
  ARI_TPrintf("MusComm Send timming command[%d]\n",no);
}

const BOOL MUS_COMM_CheckTimingCommand( MUS_COMM_WORK *work , const u8 no )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  if( GFL_NET_HANDLE_IsTimeSync( selfHandle , no , WB_NET_MUSICAL ) == TRUE )
  {
    ARI_TPrintf("MusComm Sync timming command[%d]\n",no);
    return TRUE;
  }
  return FALSE;
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
//--------------------------------------------------------------
// フラグ送受信
//--------------------------------------------------------------
typedef struct
{
  u8 flg;
  u32 value;  //ストリームのサイズがあるのでu32
}MUS_COMM_FLG_PACKET;

static const BOOL MUS_COMM_Send_Flag( MUS_COMM_WORK* work , const u8 flag , u32 value , NetID target )
{
  BOOL ret;
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  MUS_COMM_FLG_PACKET pkt;
  pkt.flg = flag;
  pkt.value = value;
  ARI_TPrintf("Send Flg[%d:%d].\n",pkt.flg,pkt.value);

  ret = GFL_NET_SendDataEx( selfHandle , target , 
                            MCST_FLG , sizeof( MUS_COMM_FLG_PACKET ) , 
                            (void*)&pkt , TRUE , FALSE , FALSE );
  if( ret == FALSE )
  {
    ARI_TPrintf("Send Flg is failued!!\n");
  }
  return ret;
}
static const BOOL MUS_COMM_Send_FlagServer( MUS_COMM_WORK* work , const u8 flag , u32 value , NetID target )
{
  BOOL ret;
  //GFL_NETHANDLE *parentHandle = GFL_NET_HANDLE_GetCurrentHandle();
  GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
  MUS_COMM_FLG_PACKET pkt;
  pkt.flg = flag;
  pkt.value = value;
  ARI_TPrintf("Send Flg Server[%d:%d].\n",pkt.flg,pkt.value);

  ret = GFL_NET_SendDataEx( parentHandle , target , 
                            MCST_FLG , sizeof( MUS_COMM_FLG_PACKET ) , 
                            (void*)&pkt , TRUE , FALSE , FALSE );
  if( ret == FALSE )
  {
    ARI_TPrintf("Send Flg is failued!!\n");
  }
  return ret;
}



static void MUS_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  MUS_COMM_FLG_PACKET *pkt = (MUS_COMM_FLG_PACKET*)pData;

  ARI_TPrintf("Post Flg[%d:%d(0x%x)] To [%d].\n",pkt->flg,pkt->value,pkt->value,netID);
  switch( pkt->flg )
  {
  case MCFT_START_GAME:
    work->isStartGame = TRUE;
    break;
  
  case MCFT_MUSICAL_IDX:
    {
      u8 i;
      u32 bitMask = 0xF;
      ARI_TPrintf("MusicalIndex:");
      for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
      {
        work->userData[i].musicalIdx = (pkt->value&bitMask)>>(i*4);
        ARI_TPrintf("[%d]",work->userData[i].musicalIdx);
        bitMask = bitMask<<4;
      }
      ARI_TPrintf("\n");
    }
    break;

  case MCFT_GAME_STATE:
    work->userData[netID].gameState = pkt->value;
    if( work->mode == MCM_PARENT )
    {
      work->isReqSendState = TRUE;
    }
    break;
  
  case MCFT_GAME_STATE_ALL:
    if( work->mode != MCM_PARENT )
    {
      work->userData[0].gameState = (pkt->value&0x000F);
      work->userData[1].gameState = (pkt->value&0x00F0)>>4;
      work->userData[2].gameState = (pkt->value&0x0F00)>>8;
      work->userData[3].gameState = (pkt->value&0xF000)>>12;
    }
    break;

  case MCFT_PROGRAM_SIZE:
    if( work->mode == MCM_CHILD )
    {
      work->distData->programData = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STRM , pkt->value );
      work->distData->programDataSize = pkt->value;
    }
    work->isPostProgramSize = TRUE;
    break;
    
  case MCFT_MESSAGE_SIZE:
    if( work->mode == MCM_CHILD )
    {
      work->distData->messageData = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STRM , pkt->value );
      work->distData->messageDataSize = pkt->value;
    }
    work->isPostMessageSize = TRUE;
    break;
    
  case MCFT_SCRIPT_SIZE:
    if( work->mode == MCM_CHILD )
    {
      work->distData->scriptData = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STRM , pkt->value );
      work->distData->scriptDataSize = pkt->value;
    }
    work->isPostScriptSize = TRUE;
    break;
    
  case MCFT_STRM_SIZE:
    if( work->mode == MCM_PARENT )
    {
      work->divSendState = MCDS_SEND;
    }
    else
    {
      work->distData->strmData = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STRM , pkt->value );
      work->distData->strmDataSize = pkt->value;
    }
    
    break;
    
  case MCFT_USE_BUTTON_REQ:
    if( work->mode == MCM_PARENT )
    {
      work->userData[netID].isReqUseButton = TRUE;
      work->userData[netID].useReqButtonPos = pkt->value;
    }
    break;
  
  case MCFT_USE_BUTTON:
    {
      u8 i;
      u32 bitMask = 0xF;
      ARI_TPrintf("PostUseButton:");
      for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
      {
        const u8 bit = (pkt->value & bitMask)>>(i*4);
        ARI_TPrintf("[%d]",bit);
        if( bit != MUS_POKE_EQU_INVALID )
        {
          work->userData[i].useButtonPos = bit;
        }
        bitMask = bitMask<<4;
      }
      work->useButtonAttentionPoke = (pkt->value & 0xF0000)>>16;
      ARI_TPrintf(":[%d]\n",work->useButtonAttentionPoke);
    }
    break;
  }
  
}
//--------------------------------------------------------------
// ポケモンデータ送受信
//--------------------------------------------------------------
const BOOL MUS_COMM_Send_MusPokeData( MUS_COMM_WORK *work , MUSICAL_POKE_PARAM *musPoke )
{
  if( work->selfPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfPokeData );
  }
  work->selfPokeData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_POKEDATA_SIZE );
  {
    u32 pokeParaAdr = (u32)work->selfPokeData+sizeof(MUSICAL_POKE_PARAM);
    GFL_STD_MemCopy( musPoke , work->selfPokeData , sizeof(MUSICAL_POKE_PARAM) );
    GFL_STD_MemCopy( musPoke->pokePara , (void*)pokeParaAdr , POKETOOL_GetWorkSize() );
  }
  
  ARI_TPrintf("Send PokeData \n");
  {
    
    GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_POKEDATA , MUS_COMM_POKEDATA_SIZE , 
                              work->selfPokeData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send PokeData is failued!!\n");
    }
    return ret;
  }
}
static void MUS_COMM_Post_MusPokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  work->userData[netID].isValidData = TRUE;
  ARI_TPrintf("MusComm Finish Post PokeData[%d].\n",netID);
  
  {
    u32 pokeParaAdr = (u32)work->userData[netID].pokeData+sizeof(MUSICAL_POKE_PARAM);
    MUSICAL_POKE_PARAM *musPoke = work->userData[netID].pokeData;
    musPoke->pokePara = (void*)pokeParaAdr;
  }

}
static u8*    MUS_COMM_Post_MusPokeDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Start Post PokeData[%d].\n",netID);
  work->userData[netID].isValidData = FALSE;
  return work->userData[netID].pokeData;
}

//--------------------------------------------------------------
// 全員分のポケモンデータ送受信
//--------------------------------------------------------------
const BOOL MUS_COMM_Send_AllMusPokeData( MUS_COMM_WORK *work  )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    u32 startAdr = (u32)work->allPokeData+MUS_COMM_POKEDATA_SIZE*i;
    GFL_STD_MemCopy( work->userData[i].pokeData , (void*)startAdr , MUS_COMM_POKEDATA_SIZE );
  }
  
  ARI_TPrintf("Send AllPokeData \n");
  {
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_ALL_POKEDATA , MUS_COMM_POKEDATA_SIZE*MUSICAL_COMM_MEMBER_NUM , 
                              work->allPokeData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send AllPokeData is failued!!\n");
    }
    return ret;
  }
}
static void MUS_COMM_Post_AllMusPokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  u8 i;
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post AllPokeData.\n");
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( i < GFL_NET_GetConnectNum() )
    {
      u32 pokeParaAdr;
      MUSICAL_POKE_PARAM *musPoke;
      u32 startAdr = (u32)work->allPokeData+MUS_COMM_POKEDATA_SIZE*i;
      GFL_STD_MemCopy( (void*)startAdr , work->userData[i].pokeData , MUS_COMM_POKEDATA_SIZE );
      pokeParaAdr = (u32)work->userData[i].pokeData+sizeof(MUSICAL_POKE_PARAM);
      musPoke = work->userData[i].pokeData;
      musPoke->pokePara = (void*)pokeParaAdr;
      work->userData[i].isValidData = TRUE;
    }
    else
    {
      work->userData[i].isValidData = FALSE;
    }
  }

}
static u8*    MUS_COMM_Post_AllMusPokeDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].isValidData = FALSE;
  }
  ARI_TPrintf("MusComm Start Post AllPokeData.\n");
  return work->allPokeData;
}

MUSICAL_POKE_PARAM* MUS_COMM_GetMusPokeParam( MUS_COMM_WORK *work , const u8 musicalIdx )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].musicalIdx == musicalIdx )
    {
      if( work->userData[i].gameState != MCGS_NONE &&
          work->userData[i].isValidData == TRUE )
      {
        return work->userData[i].pokeData;
      }
    }
  }
  return NULL;
}

//--------------------------------------------------------------
// 全員ポケデータ届いたか？
//--------------------------------------------------------------
const BOOL MUS_COMM_CheckAllPostPokeData( MUS_COMM_WORK *work )
{
  u8 i;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].gameState != MCGS_NONE &&
        work->userData[i].isValidData == FALSE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

#pragma mark [> strm func
//--------------------------------------------------------------
// ストリーミングデータ送信
//--------------------------------------------------------------
#define MUS_COMM_DIV_SIZE (0x3800)
void MUS_COMM_Start_SendStrmData( MUS_COMM_WORK *work )
{
  if( work->isSendStrmMode == FALSE )
  {
    work->isSendStrmMode = TRUE;
    work->strmDivIdx = 0;
    work->divSendState = MCDS_START;
    work->isSendingStrmData = FALSE;
  }
}

static void MUS_COMM_Update_SendStrmData( MUS_COMM_WORK *work )
{
  if( work->isSendStrmMode == TRUE )
  {
    switch(work->divSendState)
    {
    case MCDS_START:
      {
        const BOOL ret = MUS_COMM_Send_FlagServer( work , MCFT_STRM_SIZE , 
                                      work->distData->strmDataSize , 
                                      GFL_NET_SENDID_ALLUSER );
        if( ret == TRUE )
        {
          work->divSendState = MCDS_WAIT_SIZEDATA;
        }
      }
      break;
    
    case MCDS_WAIT_SIZEDATA:
      
      break;

    case MCDS_SEND:
      if( MUS_COMM_Send_StrmData( work , work->strmDivIdx ) == TRUE )
      {
        work->isSendingStrmData = TRUE;
        work->divSendState = MCDS_WAIT_POST;
      }
      break;

    case MCDS_WAIT_POST:
      {
        if( work->isSendingStrmData == FALSE )
        {
          if( (work->strmDivIdx)*MUS_COMM_DIV_SIZE >= work->distData->strmDataSize )
          {
            work->isSendStrmMode = FALSE;
          }
          else
          {
            //work->strmDivIdx++;
            work->divSendState = MCDS_SEND;
          }
        }
      }
      break;
    }
  }
}

const BOOL MUS_COMM_Send_StrmData( MUS_COMM_WORK *work , const u8 idx )
{
  
  void *startAdr = (void*)(((u32)work->distData->strmData)+(idx*MUS_COMM_DIV_SIZE));
  u16 dataSize;
  if( (idx+1)*MUS_COMM_DIV_SIZE > work->distData->strmDataSize )
  {
    dataSize = work->distData->strmDataSize%MUS_COMM_DIV_SIZE;
  }
  else
  {
    dataSize = MUS_COMM_DIV_SIZE;
  }
  
  ARI_TPrintf("Send StrmData[%d] \n",idx);
  {
    
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    //GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_STRMDATA , dataSize , 
                              startAdr , FALSE , TRUE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send PokeData is failued!!\n");
    }
    return ret;
  }
}
static void MUS_COMM_Post_StrmData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  work->strmDivIdx++;
  work->isSendingStrmData = FALSE;
  ARI_TPrintf("MusComm Finish Post StrmData[%d][%d][%d].\n",netID,work->strmDivIdx,work->distData->strmDataSize);

}
static u8*    MUS_COMM_Post_StrmDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  void *startAdd = (void*)(((u32)work->distData->strmData)+(work->strmDivIdx*MUS_COMM_DIV_SIZE));
  
  ARI_TPrintf("MusComm Start Post StrmData[%d][%d][%d].\n",netID,work->strmDivIdx,size);
  return startAdd;
}

const BOOL MUS_COMM_CheckFinishSendStrm( MUS_COMM_WORK *work )
{
  if( (work->distData->strmDataSize/MUS_COMM_DIV_SIZE) < work->strmDivIdx )
  {
    return TRUE;
  }
  return FALSE;
}

#pragma mark [> programData & msgData & scriptData func

const BOOL MUS_COMM_Send_ProgramSize( MUS_COMM_WORK *work )
{
  const BOOL ret = MUS_COMM_Send_FlagServer( work , MCFT_PROGRAM_SIZE , 
                                work->distData->programDataSize , 
                                GFL_NET_SENDID_ALLUSER );
  return ret;
}

const BOOL MUS_COMM_Send_MessageSize( MUS_COMM_WORK *work )
{
  const BOOL ret = MUS_COMM_Send_FlagServer( work , MCFT_MESSAGE_SIZE , 
                                work->distData->messageDataSize , 
                                GFL_NET_SENDID_ALLUSER );
  return ret;
}

const BOOL MUS_COMM_Send_ScriptSize( MUS_COMM_WORK *work )
{
  const BOOL ret = MUS_COMM_Send_FlagServer( work , MCFT_SCRIPT_SIZE , 
                                work->distData->scriptDataSize , 
                                GFL_NET_SENDID_ALLUSER );
  return ret;
}

const BOOL MUS_COMM_Send_ProgramData( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send ProgramData \n");
  {
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_PROGRAMDATA , work->distData->programDataSize , 
                              work->distData->programData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send ProgramData is failued!!\n");
    }
    return ret;
  }
}

static void MUS_COMM_Post_ProgramData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post ProgramData.\n");
  work->isPostProgramData = TRUE;
}

static u8* MUS_COMM_Post_ProgramDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  return work->distData->programData;
}

const BOOL MUS_COMM_Send_MessageData( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send MessageData \n");
  {
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_MESSAGEDATA , work->distData->messageDataSize , 
                              work->distData->messageData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send MessageData is failued!!\n");
    }
    return ret;
  }}

static void MUS_COMM_Post_MessageData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post MessageData.\n");
  work->isPostMessageData = TRUE;
}

static u8*    MUS_COMM_Post_MessageDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  return work->distData->messageData;
}

const BOOL MUS_COMM_Send_ScriptData( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send ScriptData \n");
  {
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_SCRIPTDATA , work->distData->scriptDataSize , 
                              work->distData->scriptData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send ScriptData is failued!!\n");
    }
    return ret;
  }}

static void MUS_COMM_Post_ScriptData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post ScriptData.\n");
  work->isPostScriptData = TRUE;
}

static u8*    MUS_COMM_Post_ScriptDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  return work->distData->scriptData;
}

const BOOL MUS_COMM_IsPostProgramSize( const MUS_COMM_WORK *work )
{
  return work->isPostProgramSize;
}
const BOOL MUS_COMM_IsPostMessageSize( const MUS_COMM_WORK *work )
{
  return work->isPostMessageSize;
}
const BOOL MUS_COMM_IsPostScriptSize( const MUS_COMM_WORK *work )
{
  return work->isPostScriptSize;
}
const BOOL MUS_COMM_IsPostProgramData( const MUS_COMM_WORK *work )
{
  return work->isPostProgramData;
}
const BOOL MUS_COMM_IsPostMessageData( const MUS_COMM_WORK *work )
{
  return work->isPostMessageData;
}
const BOOL MUS_COMM_IsPostScriptData( const MUS_COMM_WORK *work )
{
  return work->isPostScriptData;
}

#pragma mark [> outer func

const BOOL MUS_COMM_Send_MusicalIndex( MUS_COMM_WORK* work )
{
  u8 idxArr[MUSICAL_COMM_MEMBER_NUM];
  u8 i,j;
  u32 sendBit = 0;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    idxArr[i] = i;
  }
  for( j=0;j<10;j++ )
  {
    for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
    {
      u8 swapIdx = GFL_STD_MtRand0(MUSICAL_COMM_MEMBER_NUM);
      u8 temp = idxArr[i];
      idxArr[i] = idxArr[swapIdx];
      idxArr[swapIdx] = temp;
    }
  }
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    sendBit += idxArr[i]<<(4*i);
  }
  
  return MUS_COMM_Send_FlagServer( work , MCFT_MUSICAL_IDX , sendBit , GFL_NET_SENDID_ALLUSER );
  
}

const BOOL MUS_COMM_Send_UseButtonFlg( MUS_COMM_WORK* work , u8 pos )
{
  return MUS_COMM_Send_Flag( work , MCFT_USE_BUTTON_REQ , pos , GFL_NET_SENDID_ALLUSER );
}

u8 MUS_COMM_GetSelfMusicalIndex( MUS_COMM_WORK* work )
{
  const u8 selfNetId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  return work->userData[selfNetId].musicalIdx;
}


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

u8 MUS_COMM_GetUseButtonPos( MUS_COMM_WORK* work , const u8 musIdx )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].musicalIdx == musIdx )
    {
      return work->userData[i].useButtonPos;
    }
  }
  return MUS_POKE_EQU_INVALID;
}

void MUS_COMM_ResetUseButtonPos( MUS_COMM_WORK* work , const u8 musIdx )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].musicalIdx == musIdx )
    {
      work->userData[i].useButtonPos = MUS_POKE_EQU_INVALID;
    }
  }
}
u8 MUS_COMM_GetUseButtonAttention( MUS_COMM_WORK* work )
{
  if( work->useButtonAttentionPoke < MUSICAL_COMM_MEMBER_NUM )
  {
    return work->userData[work->useButtonAttentionPoke].musicalIdx;
  }
  else
  {
    return MUSICAL_COMM_MEMBER_NUM;
  }
}

void MUS_COMM_ResetUseButtonAttention( MUS_COMM_WORK* work )
{
  work->useButtonAttentionPoke = MUSICAL_COMM_MEMBER_NUM;
}

#pragma mark [> other func

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState )
{
  ARI_TPrintf("MusCommState[%d]\n",commState);
  work->commState = commState;
}
