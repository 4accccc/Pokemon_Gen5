//======================================================================
/**
 * @file  mb_comm_sys.c
 * @brief マルチブート 通信システム
 * @author  ariizumi
 * @data  09/11/13
 *
 * モジュール名：MB_COMM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "net/network_define.h"

#include "multiboot/mb_comm_sys.h"
#include "multiboot/mb_local_def.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define

#if 1 //デバッグ出力切り替え

#ifndef MULTI_BOOT_MAKE
#include "test/ariizumi/ari_debug.h"
#define MB_COMM_TPrintf(...) (void)((MB_TPrintf(__VA_ARGS__)))
#else
#define MB_COMM_TPrintf(...) (void)((MB_TPrintf(__VA_ARGS__)))
#endif //MULTI_BOOT_MAKE
#else

#define MB_COMM_TPrintf(...) (void)((MB_TPrintf(__VA_ARGS__)))
#endif//デバッグ出力切り替え

#define MB_COMM_PPP_PACK_SIZE (POKETOOL_GetPPPWorkSize()*MB_CAP_POKE_NUM + sizeof(MB_COMM_PPP_PACK_HEADER))
//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MCS_NONE,
  MCS_REQ_NEGOTIATION,
  MCS_WAIT_NEGOTIATION,
  MCS_WAIT_CAN_SEND,
  
  //ココから下はつながっている状態のSTATEとする
  MCS_CONNECT,
  
  MCS_MAX,
}MB_COMM_STATE;

//送信の種類
typedef enum
{
  MCST_FLG = GFL_NET_CMD_MULTIBOOT,
  MCST_INIT_DATA,
  MCST_POKE_DATA,
  
  MCST_MAX,
}MB_COMM_SEND_TYPE;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  u8 num;
  u8 pad[3];
}MB_COMM_PPP_PACK_HEADER;

struct _MB_COMM_WORK
{
  HEAPID heapId;
  
  MB_COMM_STATE state;
  
  MB_COMM_CHILD_STATE newChildState;
  MB_COMM_CHILD_STATE childState;
  
  void  *pppPackData;  //ヘッダ＋PPP6個分のバッファ(実送信は必要分！
  //子機のみ
  MB_COMM_INIT_DATA *initData;
  
  //確認フラグ系
  u16    isPostInitData:1;
  u16    isPostPokeData:1;

  u16    isPost_PostPokeData:1;

  u16    isReadyChildStatSave:1;
  u16    isFinishChildFirstSave:1;
  u16    isFinishChildSecondSave:1;
  u16    isFinishChildSave:1;
  u16    isPermitStartSave:1;
  u16    isPermitFirstSave:1;
  u16    isPermitSecondSave:1;
  u16    isPermitFinishSave:1;
  u16    isInitLowerData:1;
  
  u8      saveWaitCnt;
};


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

static void*  MB_COMM_GetBeaconDataDummy(void* pWork);
static int    MB_COMM_GetBeaconSizeDummy(void* pWork);

static void MB_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static void MB_COMM_Post_InitData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_InitDataBuff( int netID, void* pWork , int size );
static void MB_COMM_Post_GameData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_GameDataBuff( int netID, void* pWork , int size );
static void MB_COMM_Post_PokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    MUS_COMM_Post_PokeDataBuff( int netID, void* pWork , int size );

static const NetRecvFuncTable MultiBootCommPostTable[] = {
  { MB_COMM_Post_Flag     , NULL },
  { MB_COMM_Post_InitData , MUS_COMM_Post_InitDataBuff },
  { MB_COMM_Post_PokeData , MUS_COMM_Post_PokeDataBuff },
};

//--------------------------------------------------------------
//  ワークの作成
//--------------------------------------------------------------
MB_COMM_WORK* MB_COMM_CreateSystem( const HEAPID heapId )
{
  MB_COMM_WORK* commWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_COMM_WORK ) );
  
  commWork->heapId = heapId;
  commWork->state = MCS_NONE;
  commWork->initData = NULL;
  commWork->isInitLowerData = FALSE;

  commWork->childState = MCCS_NONE;
  commWork->newChildState = MCCS_NONE;
  
  commWork->pppPackData = GFL_HEAP_AllocClearMemory( commWork->heapId , MB_COMM_PPP_PACK_SIZE );
  return commWork;
}

//--------------------------------------------------------------
//  ワークの開放
//--------------------------------------------------------------
void MB_COMM_DeleteSystem( MB_COMM_WORK* commWork )
{
  if( commWork->initData != NULL )
  {
    GFL_HEAP_FreeMemory( commWork->initData );
  }
  if( commWork->isInitLowerData == TRUE )
  {
    GFL_NET_LDATA_ExitSystem();
  }

  GFL_HEAP_FreeMemory( commWork->pppPackData );
  GFL_HEAP_FreeMemory( commWork );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
void MB_COMM_UpdateSystem( MB_COMM_WORK* commWork )
{
  static int befState = 0xFFFF;
  if( befState != commWork->state )
  {
    MB_COMM_TPrintf("MBCommState[%d]->[%d]\n",befState,commWork->state);
    befState = commWork->state;
  }
  
  switch( commWork->state )
  {
  case MCS_NONE:
    break;
  case MCS_REQ_NEGOTIATION:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
    {
      commWork->state = MCS_WAIT_NEGOTIATION;
    }
    break;
  case MCS_WAIT_NEGOTIATION:
    {
      GFL_NETHANDLE *handle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( handle ) == TRUE )
      {
        commWork->state = MCS_WAIT_CAN_SEND;
      }
    }
    break;
  case MCS_WAIT_CAN_SEND:
    {
      GFL_NETHANDLE *handle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_IsSendEnable( handle ) == TRUE )
      {
        commWork->state = MCS_CONNECT;
        GFL_NET_SetNoChildErrorCheck( TRUE );
      }
    }
    break;
  case MCS_CONNECT:
    //この状態をMB_COMM_IsSendEnableのチェックに使ってる。
    break;
    
  }
  
  if( commWork->childState != commWork->newChildState &&
      commWork->newChildState != MCCS_NONE )
  {
    const BOOL ret = MB_COMM_Send_Flag( commWork , MCFT_CHILD_STATE , commWork->newChildState );
    if( ret == TRUE )
    {
      commWork->childState = commWork->newChildState;
    }
  }
}

//--------------------------------------------------------------
//  通信の開始
//--------------------------------------------------------------
void MB_COMM_InitComm( MB_COMM_WORK* commWork )
{
  GFLNetInitializeStruct aGFLNetInit = {
    MultiBootCommPostTable, //NetSamplePacketTbl,  // 受信関数テーブル
    NELEMS(MultiBootCommPostTable), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL,    ///< ネゴシエーション完了時にコール
    NULL, // ユーザー同士が交換するデータのポインタ取得関数
    NULL, // ユーザー同士が交換するデータのサイズ取得関数
    MB_COMM_GetBeaconDataDummy,  // ビーコンデータ取得関数  
    MB_COMM_GetBeaconSizeDummy,  // ビーコンデータサイズ取得関数 
    NULL, // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL, // 通信不能なエラーが起こった場合呼ばれる 
    NULL,  //Fatalエラーが起こった場合
    NULL,  // 通信切断時に呼ばれる関数
    NULL, // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト  
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    MB_DEF_GGID,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    0,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_WIFI,  //赤外線用にcreateされるHEAPID
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    2,//_MAXNUM,  //最大接続人数
    48,//_MAXSIZE,  //最大送信バイト数
    2,//_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,   // CRC計算
    TRUE,    // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,    //  NET通信タイプ ← wifi通信を行うかどうか
    FALSE,    // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_MULTIBOOT_SERVICEID,//GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    512 - 24*2,//MP親最大サイズ 512まで
    0,//dummy
  };

  aGFLNetInit.baseHeapID = commWork->heapId;
  GFL_NET_Init( &aGFLNetInit , NULL , (void*)commWork );
  
  commWork->isPostInitData = FALSE;
  commWork->isReadyChildStatSave = FALSE;
  commWork->isFinishChildFirstSave = FALSE;
  commWork->isFinishChildSecondSave = FALSE;
  commWork->isFinishChildSave = FALSE;
  commWork->isPermitStartSave = FALSE;
  commWork->isPermitFirstSave = FALSE;
  commWork->isPermitSecondSave = FALSE;
  commWork->isPermitFinishSave = FALSE;
  commWork->isInitLowerData = TRUE;

  GFL_NET_LDATA_InitSystem( commWork->heapId );
}

//--------------------------------------------------------------
// 通信終了
//--------------------------------------------------------------
void MB_COMM_ExitComm( MB_COMM_WORK* commWork )
{
  GFL_NET_Exit(NULL);
  GFL_NET_LDATA_ExitSystem();
  commWork->isInitLowerData = FALSE;
}

//--------------------------------------------------------------
// 初期化完了チェック
//--------------------------------------------------------------
const BOOL MB_COMM_IsInitComm( MB_COMM_WORK* commWork )
{
  return GFL_NET_IsInit();
}

//--------------------------------------------------------------
// 終了処理完了チェック
//--------------------------------------------------------------
const BOOL MB_COMM_IsFinishComm( MB_COMM_WORK* commWork )
{
  return GFL_NET_IsExit();
}

//--------------------------------------------------------------
// 親機通信開始
//--------------------------------------------------------------
void MB_COMM_InitParent( MB_COMM_WORK* commWork )
{
  GFL_NET_InitServer();
  commWork->state = MCS_REQ_NEGOTIATION;

}

//--------------------------------------------------------------
// 子機通信開始
//--------------------------------------------------------------
void MB_COMM_InitChild( MB_COMM_WORK* commWork , u8 *macAddress )
{
  GFL_NET_InitClientAndConnectToParent( macAddress );
  commWork->state = MCS_REQ_NEGOTIATION;

  GFL_NET_LDATA_CreatePostBuffer( 0x78000 , 0 , HEAPID_MULTIBOOT_DATA );
}

#pragma mark [>Afetr Connect
MB_COMM_INIT_DATA* MB_COMM_GetInitData( MB_COMM_WORK* commWork )
{
  return commWork->initData;
}

#pragma mark [>Check func
const BOOL MB_COMM_IsSendEnable( const MB_COMM_WORK* commWork )
{
  if( commWork->state >= MCS_CONNECT )
  {
    return TRUE;
  }
  return FALSE;
}

const BOOL MB_COMM_IsPostInitData( const MB_COMM_WORK* commWork )
{
  return commWork->isPostInitData;
}

const BOOL MB_COMM_IsPostGameData( const MB_COMM_WORK* commWork )
{
  return GFL_NET_LDATA_IsFinishPost( 0 );
}

//--------------------------------------------------------------
// 子機ステートの設定・取得
//--------------------------------------------------------------
void MB_COMM_SetChildState( MB_COMM_WORK* commWork , MB_COMM_CHILD_STATE state )
{
  //差があったら更新時に自動で送る
  commWork->newChildState = state;
}
const MB_COMM_CHILD_STATE MB_COMM_GetChildState( const MB_COMM_WORK* commWork )
{
  return commWork->childState;
}

//--------------------------------------------------------------
// 保存系ステートの主得
//--------------------------------------------------------------
void MB_COMM_ResetFlag( MB_COMM_WORK* commWork )
{
  commWork->isPostPokeData = FALSE;

  commWork->isPost_PostPokeData = FALSE;

  commWork->isReadyChildStatSave = FALSE;
  commWork->isFinishChildFirstSave = FALSE;
  commWork->isFinishChildSecondSave = FALSE;
  commWork->isFinishChildSave = FALSE;
  commWork->isPermitStartSave = FALSE;
  commWork->isPermitFirstSave = FALSE;
  commWork->isPermitSecondSave = FALSE;
  commWork->isPermitFinishSave = FALSE;
  
  commWork->saveWaitCnt = 0;
  
}
const BOOL MB_COMM_GetIsReadyChildStartSave( const MB_COMM_WORK* commWork )
{
  return commWork->isReadyChildStatSave;
}
const BOOL MB_COMM_GetIsFinishChildFirstSave( const MB_COMM_WORK* commWork )
{
  return commWork->isFinishChildFirstSave;
}
const BOOL MB_COMM_GetIsFinishChildSecondSave( const MB_COMM_WORK* commWork )
{
  return commWork->isFinishChildFirstSave;
}
const BOOL MB_COMM_GetIsFinishChildSave( const MB_COMM_WORK* commWork )
{
  return commWork->isFinishChildSave;
}
const BOOL MB_COMM_GetIsPermitStartSave( const MB_COMM_WORK* commWork )
{
  return commWork->isPermitStartSave;
}
const BOOL MB_COMM_GetIsPermitFirstSave( const MB_COMM_WORK* commWork )
{
  return commWork->isPermitFirstSave;
}
const BOOL MB_COMM_GetIsPermitSecondSave( const MB_COMM_WORK* commWork )
{
  return commWork->isPermitSecondSave;
}
const BOOL MB_COMM_GetIsPermitFinishSave( const MB_COMM_WORK* commWork )
{
  return commWork->isPermitFinishSave;
}
//同期セーブのため、許可が出てから同じフレーム待ってセーブ
const u8 MB_COMM_GetSaveWaitTime( const MB_COMM_WORK* commWork )
{
  return commWork->saveWaitCnt;
}

#pragma mark [>SendDataFunc
//--------------------------------------------------------------
// でかいサイズ送信用
//--------------------------------------------------------------
void MB_COMM_InitSendGameData( MB_COMM_WORK* commWork , void* gameData , u32 size )
{
  MB_TPrintf("Send GameData.\n");
  GFL_NET_LDATA_SetSendData( gameData , size , 0x02 , TRUE );
}

//--------------------------------------------------------------
// 送受信用ポケデータクリア
//--------------------------------------------------------------
void MB_COMM_ClearSendPokeData( MB_COMM_WORK* commWork )
{
  u8 i;
  MB_COMM_PPP_PACK_HEADER *packHeader = commWork->pppPackData;
  
  packHeader->num = 0;
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    POKEMON_PASO_PARAM *ppp = (POKEMON_PASO_PARAM*)((u32)commWork->pppPackData + POKETOOL_GetPPPWorkSize()*i + sizeof(MB_COMM_PPP_PACK_HEADER));
    PPP_Clear( ppp );
  }

  commWork->isPostPokeData = FALSE;
  commWork->isPost_PostPokeData = FALSE;
}

//--------------------------------------------------------------
// 送信用ポケデータ追加
//--------------------------------------------------------------
void MB_COMM_AddSendPokeData( MB_COMM_WORK* commWork , const POKEMON_PASO_PARAM *ppp )
{
  u8 i;
  MB_COMM_PPP_PACK_HEADER *packHeader = commWork->pppPackData;
  GF_ASSERT( packHeader->num < MB_CAP_POKE_NUM );
  
  {
    void* pppAdr = (void*)((u32)commWork->pppPackData + POKETOOL_GetPPPWorkSize()*packHeader->num + sizeof(MB_COMM_PPP_PACK_HEADER));

    GFL_STD_MemCopy( ppp , pppAdr , POKETOOL_GetPPPWorkSize() );
  }
  packHeader->num++;
}

//--------------------------------------------------------------
// ポケ受信確認
//--------------------------------------------------------------
const BOOL MB_COMM_IsPostPoke( MB_COMM_WORK* commWork )
{
  return commWork->isPostPokeData;
}

//--------------------------------------------------------------
// 受信ポケモン数取得
//--------------------------------------------------------------
const u8 MB_COMM_GetPostPokeNum( MB_COMM_WORK* commWork )
{
  MB_COMM_PPP_PACK_HEADER *packHeader = commWork->pppPackData;
  
  return packHeader->num;
}

//--------------------------------------------------------------
// 受信ポケモン取得
//--------------------------------------------------------------
const POKEMON_PASO_PARAM* MB_COMM_GetPostPokeData( MB_COMM_WORK* commWork , const u8 idx )
{
  MB_COMM_PPP_PACK_HEADER *packHeader = commWork->pppPackData;
  return (POKEMON_PASO_PARAM*)((u32)commWork->pppPackData + POKETOOL_GetPPPWorkSize()*idx + sizeof(MB_COMM_PPP_PACK_HEADER));
}

//--------------------------------------------------------------
// ポケ受信＿受信確認確認
//--------------------------------------------------------------
const BOOL MB_COMM_IsPost_PostPoke( MB_COMM_WORK* commWork )
{
  return commWork->isPost_PostPokeData;
}

#pragma mark [>SendFunc
//--------------------------------------------------------------
// フラグ送信
//--------------------------------------------------------------
typedef struct
{
  u32 value;
  u8 type;
  u8 pad[3];
}MB_COMM_FLG_PACKET;

const BOOL MB_COMM_Send_Flag( MB_COMM_WORK *commWork , const MB_COMM_FLG_TYPE type , const u32 value )
{
  BOOL ret;
  GFL_NETHANDLE *selfHandle;
  MB_COMM_FLG_PACKET pkt;

  if( GFL_NET_IsParentMachine() == TRUE )
  {
    selfHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
  }
  else
  {
    selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  }

  pkt.type = type;
  pkt.value = value;
  MB_COMM_TPrintf("Send Flg[%d:%d].\n",pkt.type,pkt.value);

  ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
                            MCST_FLG , sizeof( MB_COMM_FLG_PACKET ) , 
                            (void*)&pkt , TRUE , FALSE , FALSE );
  if( ret == FALSE )
  {
    MB_COMM_TPrintf("Send Flg is failued!!\n");
  }
  return ret;
}

//--------------------------------------------------------------
// フラグ受信
//--------------------------------------------------------------
static void MB_COMM_Post_Flag( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MB_COMM_WORK *commWork = (MB_COMM_WORK*)pWork;
  MB_COMM_FLG_PACKET *pkt = (MB_COMM_FLG_PACKET*)pData;

  MB_COMM_TPrintf("Post Flg[%d:%d(0x%x)] To [%d].\n",pkt->type,pkt->value,pkt->value,netID);

  switch( pkt->type )
  {
  // 親→子----------------------------------
  case MCFT_POST_POKE:
    commWork->isPost_PostPokeData = TRUE;
    break;
  case MCFT_PERMIT_START_SAVE:
    commWork->isPermitStartSave = TRUE;
    break;
  case MCFT_PERMIT_FIRST_SAVE:
    commWork->isPermitFirstSave = TRUE;
    commWork->saveWaitCnt = pkt->value;
    break;
  case MCFT_PERMIT_SECOND_SAVE:
    commWork->isPermitSecondSave = TRUE;
    commWork->saveWaitCnt = pkt->value;
    break;
  case MCFT_PERMIT_FINISH_SAVE:
    commWork->isPermitFinishSave = TRUE;
    break;

  // 子→親----------------------------------
  case MCFT_CHILD_STATE:
    commWork->childState = pkt->value;
    break;

  case MCFT_READY_START_SAVE:
    commWork->isReadyChildStatSave = TRUE;
    break;
  case MCFT_FINISH_FIRST_SAVE:
    commWork->isFinishChildFirstSave = TRUE;
    break;
  case MCFT_FINISH_SECOND_SAVE:
    commWork->isFinishChildSecondSave = TRUE;
    break;
  case MCFT_FINISH_SAVE:
    commWork->isFinishChildSave = TRUE;
    break;
  }

}

//--------------------------------------------------------------
// 初期化用データ送信
//--------------------------------------------------------------
const BOOL MB_COMM_Send_InitData( MB_COMM_WORK *commWork , MB_COMM_INIT_DATA *initData )
{
  BOOL ret;
  GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
  //GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  MB_COMM_TPrintf("Send InitData.\n");

  //子機にだけ送れば良い
  ret = GFL_NET_SendDataEx( parentHandle , 1 , 
                            MCST_INIT_DATA , sizeof( MB_COMM_INIT_DATA ) , 
                            (void*)initData , TRUE , FALSE , TRUE );
  if( ret == FALSE )
  {
    MB_COMM_TPrintf("Send InitData is failued!!\n");
  }
  return ret;
  
}

//--------------------------------------------------------------
// 初期化用データ受信
//--------------------------------------------------------------
static void MB_COMM_Post_InitData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MB_COMM_WORK *commWork = (MB_COMM_WORK*)pWork;
  MB_COMM_TPrintf("Post InitData.\n");
  commWork->isPostInitData = TRUE;
  
}
static u8*    MUS_COMM_Post_InitDataBuff( int netID, void* pWork , int size )
{
  MB_COMM_WORK *commWork = (MB_COMM_WORK*)pWork;
  if( commWork->initData == NULL )
  {
    commWork->initData = GFL_HEAP_AllocClearMemory( commWork->heapId , sizeof( MB_COMM_INIT_DATA ) );
  }
  return (u8*)commWork->initData;
}

//--------------------------------------------------------------
// ポケモンデータ送信
//--------------------------------------------------------------
const BOOL MB_COMM_Send_PokeData( MB_COMM_WORK *commWork )
{
  BOOL ret;
  MB_COMM_PPP_PACK_HEADER *packHeader = commWork->pppPackData;
  const u32 sendSize = packHeader->num * POKETOOL_GetPPPWorkSize() + sizeof(MB_COMM_PPP_PACK_HEADER);
  //GFL_NETHANDLE *parentHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  MB_COMM_TPrintf("Send PokeData.\n");

  //親機にだけ送れば良い
  ret = GFL_NET_SendDataEx( selfHandle , 0 , 
                            MCST_POKE_DATA , sendSize , 
                            commWork->pppPackData , TRUE , FALSE , TRUE );
  if( ret == FALSE )
  {
    MB_COMM_TPrintf("Send PokeData is failued!!\n");
  }
  return ret;
  
}
//--------------------------------------------------------------
// 初期化用データ受信
//--------------------------------------------------------------
static void MB_COMM_Post_PokeData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  MB_COMM_WORK *commWork = (MB_COMM_WORK*)pWork;
  MB_COMM_TPrintf("Post PokeData.\n");
  commWork->isPostPokeData = TRUE;
  
}
static u8*    MUS_COMM_Post_PokeDataBuff( int netID, void* pWork , int size )
{
  MB_COMM_WORK *commWork = (MB_COMM_WORK*)pWork;
  return (u8*)commWork->pppPackData;
}


//--------------------------------------------------------------
/**
 *  各種ビーコン用コールバック関数(ダミー
 */
//--------------------------------------------------------------
void* MB_COMM_GetBeaconDataDummy(void* pWork)
{
  static u8 dummy[2];
  return (void*)&dummy;
}
int MB_COMM_GetBeaconSizeDummy(void* pWork)
{
  return sizeof(u8)*2;
}
