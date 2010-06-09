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
#include "system/net_err.h"

#include "net/network_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "mus_comm_func.h"
#include "mus_comm_define.h"
#include "musical/musical_system.h"
#include "sound/pm_sndsys.h"
#include "savedata/mystatus.h"
#include "musical/stage/sta_snd_def.h"  //拍手音の定義

#include "test/ariizumi/ari_debug.h"
#include "musical/musical_debug_menu.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MUS_COMM_BEACON_MAX (4)
#define MUS_COMM_POKEDATA_SIZE (sizeof(MUSICAL_POKE_PARAM)+POKETOOL_GetWorkSize())
#define MUS_COMM_USERDATA_SIZE (sizeof(MUS_COMM_MISC_DATA)+MyStatus_GetWorkSize()+POKETOOL_GetPPPWorkSize())

#define MUS_COMM_SEND_APPEALBONUS_BUFF (8)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//送信の種類
typedef enum
{
  MCST_FLG = GFL_NET_CMD_MUSICAL,
  MCST_MYSTATUS ,
  MCST_ALL_MYSTATUS ,
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
  MCFT_CONDITION_POINT,
  MCFT_NPC_ARR,
  MCFT_USE_BUTTON_REQ,    //子機が親機へ使用リクエスト
  MCFT_USE_BUTTON,        //親機が子機へ使用通知
  MCFT_APPEAL_BONUS,       //親機が子機へアピールボーナス状態を送信
  
}MUS_COMM_FLAG_TYPE;

//通信状態
typedef enum
{
  MCS_NONE,
  //スクリプトで繋がってくるので、基本接続状態
  //控え室前の同期処理
  MCS_INIT_MUSICAL,
  MCS_SEND_MYSTATUS,
  MCS_SEND_MUSICAL_IDX,
  MCS_WAIT_MYSTATUS_ONE,
  MCS_WAIT_MYSTATUS_ALL,
  
  MCS_START_SEND_PROGRAM,
  MCS_SEND_CONDITION_POINT,
  MCS_SEND_NPC_ARR,
  MCS_SEND_SIZE_PROGRAM,
  MCS_SEND_DATA_PROGRAM,
  MCS_WAIT_SEND_DATA_SCRIPT,
  
  //控え室の同期処理
  MCS_SEND_SIZE_MESSAGE,
  MCS_SEND_SIZE_SCRIPT,
  MCS_SEND_DATA_MESSAGE,
  MCS_SEND_DATA_SCRIPT,
  MCS_SEND_STRM,
  MCS_START_WAIT_POST_ALL,

  //ショー前の同期処理
  MCS_SEND_SELF_POKE,
  MCS_WAIT_POST_POKE,
  MCS_SEND_ALL_POKE,
  MCS_WAIT_POST_ALL_POKE,
  
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
  //NPCでも使用する数値がある
  BOOL                isValid;        //有効なデータか？(ステータス受信時に立てておく
  BOOL                isValidData;    //pokeData受信完了フラグ
  BOOL                isValidStatus;  //MyStatus受信完了フラグ
  BOOL                isReqUseButton;
  u8                  useReqButtonPos;
  u8                  useButtonPos;
  u8                  musicalIdx;
  u8                  apeerPointBonus[MUS_COMM_APPEALBONUS_NUM]; //成功した装備箇所が入る(現状手のみなので2箇所)
  MUS_COMM_GAME_STATE gameState;
  void *pokeData;     //MUSICAL_POKE_PARAMとPOKEMON_PARAMのセット。

  MUS_COMM_MISC_DATA *miscData;
  MYSTATUS *myStatus;
  POKEMON_PASO_PARAM *ppp;
  void *userData;     //開始時に送る物(MYSTATUS,PPP,その他のセット
  
}MUS_COMM_USER_DATA;

struct _MUS_COMM_WORK
{
  HEAPID heapId;
  
  BOOL isErr;
  BOOL isInitMusical;
  BOOL isInitIrc;
  BOOL isInitComm;
  BOOL isRefreshUserData;
  BOOL isStartGame;
  u8   befMemberNum;
  
  COMM_ENTRY_BEACON beacon;
  MUS_COMM_MODE mode;   //親か子か？
  MUS_COMM_STATE commState; //もっと細かい分岐

  GAME_COMM_SYS_PTR gameComm;
  GAMEDATA *gameData;
  MYSTATUS *myStatus;
  
  MUS_COMM_USER_DATA userData[MUSICAL_COMM_MEMBER_NUM];
  MUSICAL_POKE_PARAM *selfMusPoke;  //下にパック前
  void *selfPokeData; //送信用バッファ
  void *allPokeData;  //送受信用バッファ

  //userDataで送る物
  MUS_COMM_MISC_DATA miscData;
  MYSTATUS *selfMyStatus;
  POKEMON_PASO_PARAM *ppp;
  void *selfUserData; //送信用バッファ
  void *allUserData;  //送受信用バッファ
  
  MUSICAL_DISTRIBUTE_DATA *distData;
  u32 conditionArr; //計算後のコンディションポイント
  u32 npcArr;       //NPCの番号
  u8  strmDivIdx;
  BOOL isSendStrmMode;
  BOOL isSendingStrmData;
  BOOL isPostAllUserData;
  BOOL isPostAllPokeData;
  BOOL isFinishFirstInit;

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
  BOOL isReqSendAppealBonus;
  u32  sendAppealBonusData[MUS_COMM_SEND_APPEALBONUS_BUFF]; // 
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

static const BOOL MUS_COMM_Send_MyStatus( MUS_COMM_WORK *work );
static void MUS_COMM_Post_MyStatus( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_MyStatusBuff( int netID, void* pWork , int size );
static const BOOL MUS_COMM_Send_AllMyStatus( MUS_COMM_WORK *work );
static void MUS_COMM_Post_AllMyStatus( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_AllMyStatusBuff( int netID, void* pWork , int size );

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
static BOOL MUS_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2, void* pWork);

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState );
static void MUS_COMM_DebugCheckCRC( void *data , const u32 size , const u8 code );

static const NetRecvFuncTable MusCommRecvTable[] = 
{
  { MUS_COMM_Post_Flag   ,NULL  },
  { MUS_COMM_Post_MyStatus , MUS_COMM_Post_MyStatusBuff },
  { MUS_COMM_Post_AllMyStatus , MUS_COMM_Post_AllMyStatusBuff },
  { MUS_COMM_Post_MusPokeData , MUS_COMM_Post_MusPokeDataBuff },
  { MUS_COMM_Post_AllMusPokeData , MUS_COMM_Post_AllMusPokeDataBuff },
  { MUS_COMM_Post_StrmData , MUS_COMM_Post_StrmDataBuff },
  { MUS_COMM_Post_ProgramData , MUS_COMM_Post_ProgramDataBuff },
  { MUS_COMM_Post_MessageData , MUS_COMM_Post_MessageDataBuff },
  { MUS_COMM_Post_ScriptData , MUS_COMM_Post_ScriptDataBuff },
};

static GFLNetInitializeStruct aGFLNetInitMusical = 
{
  MusCommRecvTable, //NetSamplePacketTbl,  // 受信関数テーブル
  NELEMS(MusCommRecvTable), // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL, ///< ネゴシエーション完了時にコール
  NULL, // ユーザー同士が交換するデータのポインタ取得関数
  NULL, // ユーザー同士が交換するデータのサイズ取得関数
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
  SYACHI_NETWORK_GGID,  //ggid  
  GFL_HEAPID_APP,         //元になるheapid
  HEAPID_NETWORK,         //通信用にcreateされるHEAPID
  HEAPID_WIFI,            //wifi用にcreateされるHEAPID
  HEAPID_IRC,         //
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

#pragma mark [>script func
void MUS_COMM_InitField( HEAPID heapId , GAMEDATA *gameData , GAME_COMM_SYS_PTR gameComm , const BOOL isIrc )
{
  MUS_COMM_WORK* work = GFL_HEAP_AllocClearMemory( HEAPID_PROC , sizeof( MUS_COMM_WORK ));
  work->isInitIrc = isIrc;
  work->gameData = gameData;
  work->gameComm = gameComm;
  work->isErr = FALSE;
  work->isInitMusical = FALSE;

  GameCommSys_Boot( gameComm , GAME_COMM_NO_MUSICAL , work );
}

void MUS_COMM_ExitField( MUS_COMM_WORK *work )
{
  MUS_COMM_ExitMusical( work );
  GameCommSys_ExitReq( work->gameComm );
}

void* MUS_COMM_InitGameComm(int *seq, void *pwk)
{
  MUS_COMM_WORK* work = pwk;
  work->myStatus = GAMEDATA_GetMyStatus( work->gameData );

  if( work->isInitIrc == TRUE )
  {
    aGFLNetInitMusical.bNetType = GFL_NET_TYPE_IRC_WIRELESS;
  }
  else
  {
    aGFLNetInitMusical.bNetType = GFL_NET_TYPE_WIRELESS;
    GFL_NET_Init( &aGFLNetInitMusical , MUS_COMM_FinishNetInitCallback , (void*)work );
  }

  MyStatus_Copy( work->myStatus, &work->beacon.mystatus );
  OS_GetMacAddress( work->beacon.mac_address );
  work->beacon.magic_number = COMM_ENTRY_MAGIC_NUMBER;

  
  return work;

}

BOOL MUS_COMM_ExitGameComm(int *seq, void *pwk, void *pWork)
{
  MUS_COMM_WORK* work = pwk;
  
  switch( *seq )
  {
  case 0:

    //一人の時は即切断。それ以外は同期とって終了コマンド、後に切断。
    if( GFL_NET_GetConnectNum() <= 1 )
    {
      GFL_NET_Exit( NULL );
      return TRUE;
    }
    else
    {
      if( work->isInitIrc == TRUE )
      {
        GFL_NET_DelCommandTable( GFL_NET_CMD_MUSICAL );
      }
      if( work->isInitMusical == TRUE )
      {
        GFL_NET_SetNoChildErrorCheck( FALSE );
        MUS_COMM_SendTimingCommand( work , MUS_COMM_SYNC_EXIT_COMM );
        *seq = 1;
      }
      else
      {
        //ミュージカル前なら受付キャンセルなので
        //タイミング同期をとらない
        *seq = 2;
      }
    }
    break;
  case 1:
    if( MUS_COMM_CheckTimingCommand( work , MUS_COMM_SYNC_EXIT_COMM ) == TRUE )
    {
      *seq = 2;
    }
    break;
  case 2:
    if( work->isInitMusical == FALSE )
    {
      //ミュージカル前なら勝手に切る
      GFL_NET_Exit( NULL );
      return TRUE;
    }
    else
    if( GFL_NET_IsParentMachine() == FALSE )
    {
      if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_CMD_EXIT_REQ,0,NULL) == TRUE )
      {
        return TRUE;
      }
    }
    else
    {
      if( GFL_NET_GetConnectNum() <= 1 )
      {
        GFL_NET_Exit( NULL );
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

BOOL MUS_COMM_ExitWaitGameComm(int *seq, void *pwk, void *pWork)
{
  if( GFL_NET_IsExit() == TRUE )
  {
    GFL_HEAP_FreeMemory( pWork );
    return TRUE;
  }
  return FALSE;
}

void MUS_COMM_UpdateGameComm(int *seq, void *pwk, void *pWork)
{
  MUS_COMM_UpdateComm( pWork );
}


#pragma mark [>func
void MUS_COMM_InitAfterWirelessConnect( MUS_COMM_WORK* work )
{
  if( GFL_NET_IsParentMachine() == TRUE )
  {
    work->mode = MCM_PARENT;
  }
  else
  {
    work->mode = MCM_CHILD;
  }
  
}
void MUS_COMM_InitAfterIrcConnect( MUS_COMM_WORK* work )
{
  GFL_NET_AddCommandTable( GFL_NET_CMD_MUSICAL , MusCommRecvTable , NELEMS(MusCommRecvTable) , work );
  if( GFL_NET_IsParentMachine() == TRUE )
  {
    work->mode = MCM_PARENT;
  }
  else
  {
    work->mode = MCM_CHILD;
  }
  
}

void MUS_COMM_InitMusical( MUS_COMM_WORK* work , MYSTATUS *myStatus , POKEMON_PASO_PARAM *ppp , GAME_COMM_SYS_PTR gameComm , MUSICAL_DISTRIBUTE_DATA *distData , const HEAPID heapId )
{
  u8 i,j;
  MUSICAL_SAVE *musSave =  GAMEDATA_GetMusicalSavePtr( work->gameData );
  work->heapId = heapId;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].isValid = FALSE;
    work->userData[i].musicalIdx = i;
    work->userData[i].gameState = MCGS_NONE;
    work->userData[i].isValidData = FALSE;
    work->userData[i].isValidStatus = FALSE;
    work->userData[i].isReqUseButton = FALSE;
    work->userData[i].useReqButtonPos = MUS_POKE_EQU_INVALID;
    work->userData[i].useButtonPos = MUS_POKE_EQU_INVALID;
    work->userData[i].pokeData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_POKEDATA_SIZE );
    work->userData[i].userData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_USERDATA_SIZE );
    GFL_STD_MemClear( work->userData[i].pokeData , MUS_COMM_POKEDATA_SIZE );
    GFL_STD_MemClear( work->userData[i].userData , MUS_COMM_USERDATA_SIZE );

    work->userData[i].miscData = (void*)((u32)work->userData[i].userData);
    work->userData[i].myStatus = (void*)((u32)work->userData[i].userData + sizeof(MUS_COMM_MISC_DATA));
    work->userData[i].ppp      = (void*)((u32)work->userData[i].userData + sizeof(MUS_COMM_MISC_DATA)+MyStatus_GetWorkSize());
    
    for( j=0;j<MUS_COMM_APPEALBONUS_NUM;j++ )
    {
      work->userData[i].apeerPointBonus[j] = MUS_COMM_APPEALBONUS_INVALID;
    }
  }
  work->miscData.sumPoint = MUSICAL_SAVE_GetSumPoint( musSave );
  work->selfMyStatus = myStatus;
  work->ppp = ppp;
  work->selfUserData = NULL;
  work->selfPokeData = NULL;
  work->allPokeData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_POKEDATA_SIZE*MUSICAL_COMM_MEMBER_NUM );
  work->allUserData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_USERDATA_SIZE*MUSICAL_COMM_MEMBER_NUM );
  work->befMemberNum = 0;
  work->isRefreshUserData = FALSE;
  work->isStartGame = FALSE;
  work->strmDivIdx = 0;
  work->isSendStrmMode = FALSE;
  work->isPostAllUserData = FALSE;
  work->isPostAllPokeData = FALSE;
  work->isFinishFirstInit = FALSE;
  work->isPostProgramSize = FALSE;
  work->isPostMessageSize = FALSE;
  work->isPostProgramData = FALSE;
  work->isPostMessageData = FALSE;
  work->divSendState = MCDS_NONE;
  work->isReqSendState = FALSE;
  work->isReqSendAppealBonus = FALSE;
  work->useButtonAttentionPoke = MUSICAL_COMM_MEMBER_NUM;
  work->isInitMusical = TRUE;
  for( i=0;i<MUS_COMM_SEND_APPEALBONUS_BUFF;i++ )
  {
    work->sendAppealBonusData[i] = 0xFFFF;
  }
  work->distData = distData;
  
  MUS_COMM_SetCommState( work , MCS_INIT_MUSICAL);
  
  
}
void MUS_COMM_ExitMusical( MUS_COMM_WORK* work )
{
  u8 i;
  
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].pokeData != NULL )
    {
      GFL_HEAP_FreeMemory( work->userData[i].userData );
      GFL_HEAP_FreeMemory( work->userData[i].pokeData );
    }
  }
  if( work->selfPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfPokeData );
  }
  if( work->selfUserData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfUserData );
  }
  if( work->allPokeData != NULL )
  {
    GFL_HEAP_FreeMemory( work->allPokeData );
  }
  if( work->allUserData != NULL )
  {
    GFL_HEAP_FreeMemory( work->allUserData );
  }
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
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  {
    static u8 befState = 0xFF;
    if( befState != work->commState )
    {
      ARI_TPrintf("MusCommState[%d]->[%d]\n",befState,work->commState);
      befState = work->commState;
    }
  }
#endif
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
  {
    ARI_TPrintf("mus_com_func ERROR!!!\n");
    work->isErr = TRUE;
    if( GFL_NET_IsExit() == FALSE )
    {
      GFL_NET_Exit( NULL );
    }
  }
  if( work->isErr == TRUE )
  {
    return;
  }

  switch( work->commState )
  {
  case MCS_NONE:
    return;
    //待機状態
    break;

    //スクリプトから来た場合は基本接続状態
  case MCS_INIT_MUSICAL:
    GFL_NET_SetNoChildErrorCheck( TRUE );
    MUS_COMM_SendTimingCommand( work , MUS_COMM_TIMMING_INIT_COMM );
    work->commState = MCS_SEND_MYSTATUS;
    break;
  case MCS_SEND_MYSTATUS:
    if( MUS_COMM_CheckTimingCommand( work , MUS_COMM_TIMMING_INIT_COMM ) == TRUE )
    {
      if( MUS_COMM_Send_MyStatus( work ) == TRUE )
      {
        if( work->mode == MCM_PARENT )
        {
          work->commState = MCS_SEND_MUSICAL_IDX;
        }
        else
        {
          work->commState = MCS_WAIT_MYSTATUS_ALL;
        }
      }
    }
    break;
  case MCS_SEND_MUSICAL_IDX:
    if( MUS_COMM_Send_MusicalIndex( work ) == TRUE )
    {
      work->commState = MCS_WAIT_MYSTATUS_ONE;
    }
    break;
  case MCS_WAIT_MYSTATUS_ONE:
    {
      u8 i;
      u8 postNum = 0;
      for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
      {
        if( work->userData[i].isValidStatus == TRUE )
        {
          postNum++;
        }
      }
      if( postNum == GFL_NET_GetConnectNum() )
      {
        if( MUS_COMM_Send_AllMyStatus( work ) == TRUE )
        {
          work->commState = MCS_WAIT_MYSTATUS_ALL;
        }
      }
    }
    break;
    
  case MCS_WAIT_MYSTATUS_ALL:
    if( work->isPostAllUserData == TRUE )
    {
      //処理なし。外部からの操作で下に進む
    }
    break;
  
  case MCS_START_SEND_PROGRAM:
    if( MUS_COMM_CheckTimingCommand( work , MUS_COMM_SYNC_START_SEND_PROGRAM ) == TRUE )
    {
      if( work->mode == MCM_PARENT )
      {
        work->commState = MCS_SEND_CONDITION_POINT;
      }
      else
      {
        work->commState = MCS_WAIT_SEND_DATA_SCRIPT;
      }
    }
    break;
  case MCS_SEND_CONDITION_POINT:
    {
      if( MUS_COMM_Send_FlagServer( work,
                                    MCFT_CONDITION_POINT,
                                    work->conditionArr,
                                    GFL_NET_SENDID_ALLUSER) == TRUE )
      {
        work->commState = MCS_SEND_NPC_ARR;
      }
    }
    break;
  case MCS_SEND_NPC_ARR:
    {
      if( MUS_COMM_Send_FlagServer( work,
                                    MCFT_NPC_ARR,
                                    work->npcArr,
                                    GFL_NET_SENDID_ALLUSER) == TRUE )
      {
        work->commState = MCS_SEND_SIZE_PROGRAM;
      }
    }
    break;
  case MCS_SEND_SIZE_PROGRAM:
    if( MUS_COMM_Send_ProgramSize(work) == TRUE )
    {
      work->commState = MCS_SEND_DATA_PROGRAM;
    }
    break;

  case MCS_SEND_DATA_PROGRAM:
    if( work->isPostProgramSize == TRUE )
    {
      if( MUS_COMM_Send_ProgramData(work) == TRUE )
      {
        work->commState = MCS_WAIT_SEND_DATA_SCRIPT;
      }
    }
    break;


  case MCS_WAIT_SEND_DATA_SCRIPT:
    if( work->isPostProgramData == TRUE )
    {
      //処理なし。外部からの操作で下に進む
    }
    
    break;

  case MCS_SEND_SIZE_MESSAGE:
    if( MUS_COMM_CheckTimingCommand( work , MUS_COMM_SYNC_START_SEND_SCRIPT ) == TRUE )
    {
      if( work->mode == MCM_PARENT )
      {
        if( MUS_COMM_Send_MessageSize(work) == TRUE )
        {
          work->commState = MCS_SEND_SIZE_SCRIPT;
        }
      }
      else
      {
        work->commState = MCS_START_WAIT_POST_ALL;
      }
    }
    break;
  case MCS_SEND_SIZE_SCRIPT:
    if( work->isPostMessageSize == TRUE )
    {
      if( MUS_COMM_Send_ScriptSize(work) == TRUE )
      {
        work->commState = MCS_SEND_DATA_MESSAGE;
      }
    }
    break;

  case MCS_SEND_DATA_MESSAGE:
    if( work->isPostScriptSize == TRUE )
    {
      if( MUS_COMM_Send_MessageData(work) == TRUE )
      {
        work->commState = MCS_SEND_DATA_SCRIPT;
      }
    }
    break;
  case MCS_SEND_DATA_SCRIPT:
    if( work->isPostMessageData == TRUE )
    {
      if( MUS_COMM_Send_ScriptData(work) == TRUE )
      {
        work->commState = MCS_SEND_STRM;
      }
    }
    break;
  case MCS_SEND_STRM:
    //外からストリームの開始をたたく
    //MUS_COMM_Start_SendStrmData( work );
    work->commState = MCS_START_WAIT_POST_ALL;
    break;
  
  case MCS_START_WAIT_POST_ALL:
    if( work->isPostScriptData == TRUE &&
        MUS_COMM_CheckFinishSendStrm(work) == TRUE )
    {
      //処理なし。外部からの操作で下に進む
    }
    break;

  case MCS_SEND_SELF_POKE:
    if( MUS_COMM_Send_MusPokeData( work , work->selfMusPoke ) == TRUE )
    {
      if( work->mode == MCM_PARENT )
      {
        work->commState = MCS_WAIT_POST_POKE;
      }
      else
      {
        work->commState = MCS_WAIT_POST_ALL_POKE;
      }
    }
    break;
  case MCS_WAIT_POST_POKE:
    {
      u8 i;
      u8 postNum = 0;
      for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
      {
        if( work->userData[i].isValidData == TRUE )
        {
          postNum++;
        }
      }
      if( postNum == GFL_NET_GetConnectNum() )
      {
        if( MUS_COMM_Send_AllMusPokeData( work ) == TRUE )
        {
          work->commState = MCS_WAIT_POST_ALL_POKE;
        }
      }
    }
    break;
    
  case MCS_WAIT_POST_ALL_POKE:
    if( work->isPostAllPokeData == TRUE )
    {
      //処理なし
    }
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
  //アピールボーナス送信
  if( work->isReqSendAppealBonus == TRUE )
  {
    u8 i;
    BOOL isEmpty = TRUE;
    for( i=0;i<MUS_COMM_SEND_APPEALBONUS_BUFF;i++ )
    {
      if( work->sendAppealBonusData[i] != 0xFFFF )
      {
        const BOOL ret = MUS_COMM_Send_FlagServer( work , 
                                      MCFT_APPEAL_BONUS ,
                                      work->sendAppealBonusData[i],
                                      GFL_NET_SENDID_ALLUSER );
        if( ret == TRUE )
        {
          work->sendAppealBonusData[i] = 0xFFFF;
        }
        isEmpty = FALSE;
      }
    }
    if( isEmpty == TRUE )
    {
      work->isReqSendAppealBonus = FALSE;
    }
  }
  MUS_COMM_Update_SendStrmData( work );
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
  
  //念のため名前が入ってないときに落ちないようにしておく
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
  if( work->isErr == TRUE )
  {
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
    if( work->userData[idx].isValidStatus == TRUE )
    {
      return work->userData[idx].myStatus;
    }
  }
  return NULL;
}

//--------------------------------------------------------------
// ppp取得
//--------------------------------------------------------------
POKEMON_PASO_PARAM* MUS_COMM_GetPlayerPPP( MUS_COMM_WORK* work , u8 idx )
{
  if( work->commState != MCS_NONE )
  {
    if( work->userData[idx].isValidStatus == TRUE )
    {
      return work->userData[idx].ppp;
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
  return sizeof( COMM_ENTRY_BEACON );
}

//--------------------------------------------------------------
// 接続チェック用比較
//--------------------------------------------------------------
static BOOL MUS_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2, void* pWork)
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
      //これは控え室前なのでHEAPID_MUSICAL_STRMは無い！
      work->distData->programData = GFL_HEAP_AllocMemory( HEAPID_PROC , pkt->value );
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
      work->distData->midiData = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STRM , pkt->value );
      work->distData->midiDataSize = pkt->value;
    }
    
    break;
  case MCFT_CONDITION_POINT:
    work->conditionArr = pkt->value;
    break;
  case MCFT_NPC_ARR:
    work->npcArr = pkt->value;
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
  
  case MCFT_APPEAL_BONUS:
    {
      u8 i;
      const u8 seType = ((pkt->value&0x00FF0000)>>16);
      const u8 idx    = ((pkt->value&0x0000FF00)>>8);
      const u8 pos    = ((pkt->value&0x000000FF));
      for( i=0;i<MUS_COMM_APPEALBONUS_NUM;i++ )
      {
        if( work->userData[idx].apeerPointBonus[i] == MUS_COMM_APPEALBONUS_INVALID )
        {
          work->userData[idx].apeerPointBonus[i] = pos;
          break;
        }
      }
      if( seType == 1 )
      {
        PMSND_PlaySE( STA_SE_CLAP_1 );
      }
      else
      if( seType == 2 )
      {
        PMSND_PlaySE( STA_SE_CLAP_2 );
      }
      ARI_TPrintf("PostAppealBonus:[%d][%d][%d]\n",seType,idx,pos);
    }
    break;
  }
  
}


//--------------------------------------------------------------
// MyStatus送受信
//--------------------------------------------------------------
static const BOOL MUS_COMM_Send_MyStatus( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send UserData \n");
  if( work->selfUserData != NULL )
  {
    GFL_HEAP_FreeMemory( work->selfUserData );
  }
  work->selfUserData = GFL_HEAP_AllocMemory( work->heapId , MUS_COMM_USERDATA_SIZE );
  {
    u32 myStatusAdr = (u32)work->selfUserData+sizeof(MUS_COMM_MISC_DATA);
    u32 pppAdr      = (u32)work->selfUserData+sizeof(MUS_COMM_MISC_DATA)+MyStatus_GetWorkSize();
    GFL_STD_MemCopy( &work->miscData   , work->selfUserData , sizeof(MUS_COMM_MISC_DATA) );
    GFL_STD_MemCopy( work->selfMyStatus, (void*)myStatusAdr , MyStatus_GetWorkSize() );
    GFL_STD_MemCopy( work->ppp         , (void*)pppAdr      , POKETOOL_GetPPPWorkSize() );
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)

  ARI_TPrintf("-----Send User Data-----\n");
  ARI_TPrintf("[%3d]",PPP_Get(work->ppp,ID_PARA_monsno,NULL));
  ARI_TPrintf("[%5d]",MyStatus_GetID_Low(work->selfMyStatus));
  ARI_TPrintf("[%5d]",work->miscData.sumPoint);
  ARI_TPrintf("\n");
  ARI_TPrintf("-----Send User Data-----\n");
#endif
  }

  {
    
    GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
    BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_MYSTATUS , MUS_COMM_USERDATA_SIZE , 
                              work->selfUserData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send UserData is failued!!\n");
    }
    return ret;
  }
}
static void MUS_COMM_Post_MyStatus( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  work->userData[netID].isValidStatus = TRUE;
  work->userData[netID].isValid = TRUE;
  ARI_TPrintf("MusComm Finish Post UserData[%d].\n",netID);

}

static u8*    MUS_COMM_Post_MyStatusBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Start Post UserData[%d].\n",netID);
  work->userData[netID].isValidStatus = FALSE;
  return (u8*)work->userData[netID].userData;
}

//--------------------------------------------------------------
// 全員分のMyStatus送受信
//--------------------------------------------------------------
const BOOL MUS_COMM_Send_AllMyStatus( MUS_COMM_WORK *work  )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    u32 startAdr = (u32)work->allUserData+MUS_COMM_USERDATA_SIZE*i;
    GFL_STD_MemCopy( work->userData[i].userData , (void*)startAdr , MUS_COMM_USERDATA_SIZE );
  }
  
  ARI_TPrintf("Send AllUserData \n");
  {
    GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    BOOL ret = GFL_NET_SendDataEx( parentHandle , GFL_NET_SENDID_ALLUSER , 
                              MCST_ALL_MYSTATUS , MUS_COMM_USERDATA_SIZE*MUSICAL_COMM_MEMBER_NUM , 
                              work->allUserData , TRUE , FALSE , TRUE );
    if( ret == FALSE )
    {
      ARI_TPrintf("Send AllUserData is failued!!\n");
    }
    return ret;
  }
}
static void MUS_COMM_Post_AllMyStatus( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  u8 i;
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post AllUserData.\n");
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    u32 startAdr = (u32)work->allUserData+MUS_COMM_USERDATA_SIZE*i;
    GFL_STD_MemCopy( (void*)startAdr , work->userData[i].userData , MUS_COMM_USERDATA_SIZE );
    
    
    if( MyStatus_CheckNameClear( work->userData[i].myStatus ) == FALSE )
    {
      work->userData[i].isValid = TRUE;
      work->userData[i].isValidStatus = TRUE;
    }
    else
    {
      work->userData[i].isValid = FALSE;
      work->userData[i].isValidStatus = FALSE;
    }
  }
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  ARI_TPrintf("-----Post All User Data-----\n");
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    ARI_TPrintf("[%d]",i);
    if( work->userData[i].isValid == TRUE )
    {
      ARI_TPrintf(":[%3d]",PPP_Get(work->userData[i].ppp,ID_PARA_monsno,NULL));
      ARI_TPrintf("[%5d]",MyStatus_GetID_Low(work->userData[i].myStatus));
      ARI_TPrintf("[%5d]",work->userData[i].miscData->sumPoint);
      ARI_TPrintf("\n");
    }
    else
    {
      ARI_TPrintf("NPC\n");
    }
  }
  ARI_TPrintf("-----Post All User Data-----\n");
#endif
  work->isPostAllUserData = TRUE;

}
static u8*    MUS_COMM_Post_AllMyStatusBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].isValidStatus = FALSE;
  }
  work->isPostAllUserData = FALSE;
  ARI_TPrintf("MusComm Start Post AllUserData.\n");
  return work->allUserData;
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
    if( MyStatus_CheckNameClear( work->userData[i].myStatus ) == FALSE )
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
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  ARI_TPrintf("-----Post All Poke Data-----\n");
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    MUSICAL_POKE_PARAM *musPoke = work->userData[i].pokeData;
    ARI_TPrintf("[%d]",i);
    if( work->userData[i].isValid == TRUE )
    {
      u8 j;
      ARI_TPrintf(":[%3d]|",PP_Get(musPoke->pokePara,ID_PARA_monsno,NULL));
      for( j=0 ; j<MUS_POKE_EQUIP_MAX ; j++ )
      {
        ARI_TPrintf("[%3d]",musPoke->equip[j].itemNo);
      }
      ARI_TPrintf("\n");
    }
    else
    {
      ARI_TPrintf("NPC\n");
    }
  }
  ARI_TPrintf("-----Post All User Data-----\n");
#endif
  work->isPostAllPokeData = TRUE;

}
static u8*    MUS_COMM_Post_AllMusPokeDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    work->userData[i].isValidData = FALSE;
  }
  work->isPostAllPokeData = FALSE;
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
      if( work->userData[i].isValidData == TRUE )
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
                                      work->distData->midiDataSize , 
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
          if( (work->strmDivIdx)*MUS_COMM_DIV_SIZE >= work->distData->midiDataSize )
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
  
  void *startAdr = (void*)(((u32)work->distData->midiData)+(idx*MUS_COMM_DIV_SIZE));
  u16 dataSize;
  if( (idx+1)*MUS_COMM_DIV_SIZE > work->distData->midiDataSize )
  {
    dataSize = work->distData->midiDataSize%MUS_COMM_DIV_SIZE;
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

  ARI_TPrintf("MusComm Finish Post StrmData[%d][%d][%d].\n",netID,work->strmDivIdx,work->distData->midiDataSize);
  if( (work->strmDivIdx)*MUS_COMM_DIV_SIZE >= work->distData->midiDataSize )
  {
    //データを各ポインタに設定
    MUS_DIST_MIDI_HEADER *midHeader = work->distData->midiData;
    work->distData->midiSeqData  = (void*)((u32)work->distData->midiData + sizeof(MUS_DIST_MIDI_HEADER) );
    work->distData->midiBnkData  = (void*)((u32)work->distData->midiData + sizeof(MUS_DIST_MIDI_HEADER) + midHeader->seqSize );
    work->distData->midiWaveData = (void*)((u32)work->distData->midiData + sizeof(MUS_DIST_MIDI_HEADER) + midHeader->seqSize + midHeader->bankSize );

    ARI_TPrintf("MusComm FinishPostMidiData[%d][%d][%d].\n",midHeader->seqSize,midHeader->bankSize,midHeader->waveSize);
  }


}
static u8*    MUS_COMM_Post_StrmDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  void *startAdd = (void*)(((u32)work->distData->midiData)+(work->strmDivIdx*MUS_COMM_DIV_SIZE));
  
  ARI_TPrintf("MusComm Start Post StrmData[%d][%d][%d].\n",netID,work->strmDivIdx,size);
  return startAdd;
}

const BOOL MUS_COMM_CheckFinishSendStrm( MUS_COMM_WORK *work )
{
  if( (work->distData->midiDataSize/MUS_COMM_DIV_SIZE) < work->strmDivIdx )
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
  MUS_COMM_DebugCheckCRC( work->distData->programData , work->distData->programDataSize , 2 );

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

  MUS_COMM_DebugCheckCRC( work->distData->programData , work->distData->programDataSize , 12 );
}

static u8* MUS_COMM_Post_ProgramDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Start Post ProgramData.\n");
  return work->distData->programData;
}

const BOOL MUS_COMM_Send_MessageData( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send MessageData \n");
  MUS_COMM_DebugCheckCRC( work->distData->messageData , work->distData->messageDataSize , 1 );
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

  MUS_COMM_DebugCheckCRC( work->distData->messageData , work->distData->messageDataSize , 11 );
}

static u8*    MUS_COMM_Post_MessageDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Start Post MessageData.\n");
  return work->distData->messageData;
}

const BOOL MUS_COMM_Send_ScriptData( MUS_COMM_WORK *work )
{
  ARI_TPrintf("Send ScriptData \n");
  MUS_COMM_DebugCheckCRC( work->distData->scriptData , work->distData->scriptDataSize , 0 );
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
  }
}

static void MUS_COMM_Post_ScriptData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Finish Post ScriptData.\n");
  work->isPostScriptData = TRUE;

  MUS_COMM_DebugCheckCRC( work->distData->scriptData , work->distData->scriptDataSize , 10 );
}

static u8*    MUS_COMM_Post_ScriptDataBuff( int netID, void* pWork , int size )
{
  MUS_COMM_WORK *work = (MUS_COMM_WORK*)pWork;
  ARI_TPrintf("MusComm Start Post ScriptData.\n");
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
#if PM_DEBUG
  MUSICAL_DEBUG_MENU_WORK *debWork = MUSICAL_DEBUG_GetWork();
  if( debWork != NULL && 
      debWork->enableArr == TRUE )
  {
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      idxArr[i] = debWork->arr[i];
    }
  }
  else
#endif
  {
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

u8 MUS_COMM_GetMusicalIndex( MUS_COMM_WORK* work , const u8 idx)
{
  return work->userData[idx].musicalIdx;
}

const MUS_COMM_MISC_DATA* MUS_COMM_GetUserMiscData( MUS_COMM_WORK* work , const u8 idx )
{
  return work->userData[idx].miscData;
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

const BOOL MUS_COMM_IsPostAllMyStatus( MUS_COMM_WORK* work )
{
  return work->isPostAllUserData;
}

const BOOL MUS_COMM_IsPostAllPoke( MUS_COMM_WORK* work )
{
  return work->isPostAllPokeData;
}

void MUS_COMM_StartSendProgram_Data( MUS_COMM_WORK* work , u32 conArr , u32 npcArr )
{
  MUS_COMM_SendTimingCommand( work , MUS_COMM_SYNC_START_SEND_PROGRAM );
  work->commState = MCS_START_SEND_PROGRAM;
  work->conditionArr = conArr;
  work->npcArr = npcArr;
}
void MUS_COMM_StartSendProgram_Script( MUS_COMM_WORK* work )
{
  MUS_COMM_SendTimingCommand( work , MUS_COMM_SYNC_START_SEND_SCRIPT );
  work->commState = MCS_SEND_SIZE_MESSAGE;
}

void MUS_COMM_StartSendPoke( MUS_COMM_WORK* work , MUSICAL_POKE_PARAM *musPoke)
{
  work->commState = MCS_SEND_SELF_POKE;
  work->selfMusPoke = musPoke;
}

u32 MUS_COMM_GetConditionPointArr( MUS_COMM_WORK* work )
{
  return work->conditionArr;
}
u32 MUS_COMM_GetNpcArr( MUS_COMM_WORK* work )
{
  return work->npcArr;
}

const u8 MUS_COMM_GetAppealBonus( MUS_COMM_WORK* work , const u8 pokeIdx , const u8 dataIdx )
{
  return work->userData[pokeIdx].apeerPointBonus[dataIdx];
}

//NPC用グッズ仕様リクエスト
void MUS_COMM_SetReqUseItem_NPC( MUS_COMM_WORK* work , const u8 musIdx , const u8 pos )
{
  u8 i;
  for( i=0;i<MUSICAL_COMM_MEMBER_NUM;i++ )
  {
    if( work->userData[i].musicalIdx == musIdx )
    {
      work->userData[i].isReqUseButton = TRUE;
      work->userData[i].useReqButtonPos = pos;
    }
  }
}

#pragma mark [>outer func (button)
//ボタン関係
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

void MUS_COMM_ReqSendAppealBonusPoke( MUS_COMM_WORK* work , const u8 idx , const u8 pos , const u8 seType)
{
  u8 i;
  for( i=0;i<MUS_COMM_SEND_APPEALBONUS_BUFF;i++ )
  {
    if( work->sendAppealBonusData[i] == 0xFFFF )
    {
      work->sendAppealBonusData[i] = (seType<<16) + (idx<<8) + pos;
      work->isReqSendAppealBonus = TRUE;
      return;
    }
  }
}

#pragma mark [> other func

inline static void MUS_COMM_SetCommState( MUS_COMM_WORK *work , const MUS_COMM_STATE commState )
{
  ARI_TPrintf("MusCommState[%d]\n",commState);
  work->commState = commState;
}

GFLNetInitializeStruct* MUS_COMM_GetNetInitStruct(void)
{
  return &aGFLNetInitMusical;
}

static void MUS_COMM_DebugCheckCRC( void *data , const u32 size , const u8 code )
{
#if 0
  u16 i;
  u16 cnt = 0;
  u32 sum = 0;
  OS_TFPrintf(3,"MUS_COMM Start  check CRC[%d]\n",code);
  OS_TFPrintf(1,"MUS_COMM Start  check CRC[%d]\n",code);
  for( i=0;i<size/2;i++ )
  {
    u16 *val = (u16*)((u32)data + i*2);
    sum += *val;
    cnt++;
    if( cnt == 255 )
    {
      OS_TFPrintf(3,"[%10d]\n",sum );
      cnt = 0;
      sum = 0;
    }
  }
  cnt = 5;
  for( i=0;i<size;i++ )
  {
    u8 *val1 = (u8*)((u32)data + i);
    OS_TFPrintf(1,"%02x",*val1 );
    cnt++;
    
    if( cnt == 409 )
    {
      OS_TFPrintf(1,"\n" );
      cnt = 0;
    }
  }

  OS_TFPrintf(3,"[%10d]\n",sum );
  OS_TFPrintf(3,"(%d)\n",SVC_GetCRC16( 0 , data , size ));
  OS_TFPrintf(1,"\n(%d)\n",SVC_GetCRC16( 0 , data , size ));
  

  OS_TFPrintf(3,"MUS_COMM Finish check CRC[%d]\n",code);
#endif
}