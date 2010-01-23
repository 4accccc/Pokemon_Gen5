//======================================================================
/**
 * @file	ctvt_comm.c
 * @brief	通信TVTシステム：通信管
 * @author	ariizumi
 * @data	09/12/17
 *
 * モジュール名：CTVT_COMM
 */
//======================================================================
#include <gflib.h>

#include <twl/ssp/ARM9/jpegenc.h>
#include <twl/ssp/ARM9/jpegdec.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "net/network_define.h"
#include "net/wih_dwc.h"
#include "net/ctvt_beacon_local.h"

#include "ctvt_comm.h"
#include "ctvt_camera.h"
#include "ctvt_talk.h"
#include "ctvt_mic.h"
#include "enc_adpcm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
//JPG変換係
#define CTVT_COMM_JPG_OUT_TYPE (SSP_JPEG_OUTPUT_YUV444)
#define CTVT_COMM_JPG_OUT_OPT  (SSP_JPEG_RGB555)
#define CTVT_COMM_JPG_QUALITY  (70)

//お絵描きバッファ
#define CTVT_COMM_DRAW_BUF_NUM (5)

#define CTVT_COMM_ADD_COMMAND_SYNC (8)
#define CTVT_COMM_RELEASE_COMMAND_SYNC (9)
#define CTVT_COMM_SCAN_BEACON_NUM (16)

//新規接続時のお絵描き共有
#define CTVT_COMM_SHARE_DRAW_BUFF (0)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CCS_NONE,
  CCS_INIT_COMM,
  CCS_INIT_COMM_WAIT,

  CCS_REQ_NEGOTIATION,
  CCS_WAIT_NEGOTIATION,

  CCS_ADD_COMMAND,
  CCS_ADD_COMMAND_SYNC_WAIT,
  
  CCS_CONNECT,
  
  CCS_DISCONNECT,

  CCS_RELEASE_COMMAND,
  CCS_RELEASE_COMMAND_SYNC_WAIT,
  
  CCS_DISCONNECT_WAIT,
  CCS_FINISH,
}CTVT_COMM_STATE;

typedef enum
{
  CCSU_NONE,

  CCSU_REQ_PHOTO,
  CCSU_POST_PHOTO_WAIT,
  CCSU_POST_PHOTO,

}CTVT_COMM_STATE_USER;

//通信の送信タイプ
typedef enum
{
  CCST_SEND_FLG = GFL_NET_CMD_COMM_TVT,
  CCST_SEND_WAVE,
  CCST_SEND_DRAW,
  CCST_SEND_MEMBER_DATA,
#if CTVT_COMM_SHARE_DRAW_BUFF
  CCST_SEND_DRAW_BUFF,
#endif //CTVT_COMM_SHARE_DRAW_BUFF
  CCST_MAX,
}CTVT_COMM_SEND_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//ユーザーデータ
typedef struct
{
  BOOL isEnable;
  BOOL isEnableData;
  BOOL isSelf;
  BOOL reqTalk;
  u8   bufferNo;
  
  CTVT_COMM_STATE_USER state;
  CTVT_COMM_MEMBER_DATA data;
}CTVT_MEMBER_STATE;

//ワーク
struct _CTVT_COMM_WORK
{
  BOOL isLowerDataInit;
  u8 connectNum;
  CTVT_COMM_STATE state;
  
  CTVT_COMM_BEACON beacon;
  CTVT_COMM_MEMBER_DATA selfData;
  
  //メンバーの管理用
  CTVT_MEMBER_STATE member[CTVT_MEMBER_NUM];
  void *postPhotoBuf[CTVT_MEMBER_NUM-1];
  u8    PhotobufUseBit;
  
  CTVT_COMM_INIT_MODE mode;
  CTVT_COMM_INIT_MODE nextMode;
  u8 targetMacAddress[6];
  
  //自身の管理用
  u8 reqCheckBit;
  u8 photoReqBit;
  BOOL isSendWait;
  void *encWorkBuf;
  void *encBuffer;
  void *sendBuffer;
  BOOL isCommWave;   //Wave送受信中
  BOOL isFinishPostWave;   //Wave送受信中
  u16  waveSize;
  int  waveSpeed;
  void *postWaveBuf;  //Wave受信バッファ
  void *decodeWaveBuf;      //デコード後データ
  
  u8   drawBufNum;
  DRAW_SYS_PEN_INFO   drawBuf[CTVT_COMM_DRAW_BUF_NUM];
  //親機から子機へ共有する処理
  u8  talkMember;
  
  //親機専用処理
  BOOL updateReqTalk;
  BOOL reqSendMemberData;
  BOOL reqSendDoubleFlg;
  BOOL sendDoubleFlgValue;
#if CTVT_COMM_SHARE_DRAW_BUFF
  BOOL reqSendDrawBuffer;
  BOOL sendingDrawBuffer;
  BOOL postDrawBuffer;
#endif //CTVT_COMM_SHARE_DRAW_BUFF
  
  BOOL updateTalkMember;
  u8 tempTalkMember;
  
  COMM_TVT_WORK *parentWork;  //受信コールバックで使うため
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_COMM_InitComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_UpdateComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_UpdateScan( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void*  CTVT_COMM_GetBeaconData(void* pWork);
static int CTVT_COMM_GetBeaconSize(void *pWork);
static BOOL CTVT_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2);

static void CTVT_COMM_RefureshCommState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

static void CTVT_COMM_ClearMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state );
static void CTVT_COMM_UpdateMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state , const u8 idx );

static void CTVT_COMM_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    CTVT_COMM_Post_WaveData_Buff( int netID, void* pWork , int size );
static void CTVT_COMM_Post_WaveData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static const BOOL CTVT_COMM_SendDrawData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_PostDrawData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static const BOOL CTVT_COMM_SendMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_PostMemberData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*  CTVT_COMM_PostMemberData_Buff( int netID, void* pWork , int size );

#if CTVT_COMM_SHARE_DRAW_BUFF
static const BOOL CTVT_COMM_SendDrawBuff( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_PostDrawBuff( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*  CTVT_COMM_Post_DrawBuff_Buff( int netID, void* pWork , int size );
#endif //CTVT_COMM_SHARE_DRAW_BUFF

static const NetRecvFuncTable CTVT_COMM_RecvTable[] = 
{
  { CTVT_COMM_PostFlg   ,NULL  },
  { CTVT_COMM_Post_WaveData   ,CTVT_COMM_Post_WaveData_Buff  },
  { CTVT_COMM_PostDrawData   ,NULL  },
  { CTVT_COMM_PostMemberData   , CTVT_COMM_PostMemberData_Buff },
#if CTVT_COMM_SHARE_DRAW_BUFF
  { CTVT_COMM_PostDrawBuff   , CTVT_COMM_Post_DrawBuff_Buff },
#endif CTVT_COMM_SHARE_DRAW_BUFF
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_COMM_WORK* CTVT_COMM_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_COMM_WORK* commWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_COMM_WORK) );
  
  commWork->parentWork = work;
  commWork->state = CCS_NONE;
  commWork->connectNum = 1;
  commWork->photoReqBit = 0;
  commWork->mode = CCIM_NONE;
  commWork->nextMode = CCIM_NONE;
  
  for( i=0;i<CTVT_MEMBER_NUM;i++ )
  {
    CTVT_COMM_ClearMemberState( work , commWork , &commWork->member[i] );
  }
  for( i=0;i<CTVT_MEMBER_NUM-1;i++ )
  {
    commWork->postPhotoBuf[i] = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  }
  {
    const u32 encWorkSize = SSP_GetJpegEncoderBufferSize( 128,192,CTVT_COMM_JPG_OUT_TYPE,CTVT_COMM_JPG_OUT_OPT );
    commWork->encWorkBuf = GFL_NET_Align32Alloc( heapId , encWorkSize );
  }
  commWork->encBuffer = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  commWork->sendBuffer = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  commWork->PhotobufUseBit = 0;
  commWork->reqCheckBit = 0;
  commWork->isSendWait = FALSE;
  
  commWork->isCommWave = FALSE;
  commWork->isFinishPostWave = FALSE;
  commWork->postWaveBuf = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_COMM_WAVE_HEADER)+CTVT_SEND_WAVE_SIZE_ONE_REAL );
  commWork->decodeWaveBuf = GFL_HEAP_AllocClearMemory( heapId , CTVT_SEND_WAVE_SIZE );
  commWork->drawBufNum = 0;
  
  commWork->talkMember = CTVT_COMM_INVALID_MEMBER;
  
  commWork->updateReqTalk = FALSE;
  commWork->reqSendMemberData = FALSE;
  commWork->reqSendDoubleFlg = FALSE;
  commWork->sendDoubleFlgValue = FALSE;
#if CTVT_COMM_SHARE_DRAW_BUFF
  commWork->reqSendDrawBuffer = FALSE;
  commWork->sendingDrawBuffer = FALSE;
  commWork->postDrawBuffer = FALSE;
#endif //CTVT_COMM_SHARE_DRAW_BUFF
  commWork->updateTalkMember = FALSE;
  commWork->tempTalkMember = CTVT_COMM_INVALID_MEMBER;
  
  {
    //ビーコンの初期化
    u8 i,j;
    const COMM_TVT_INIT_WORK *initWork = COMM_TVT_GetInitWork( work );
    MYSTATUS *myStatus = GAMEDATA_GetMyStatus( initWork->gameData );
    
    MyStatus_Copy( myStatus , &commWork->beacon.myStatus );
    
    commWork->beacon.connectNum = 1;
    for( i=0;i<3;i++ )
    {
      for( j=0;j<6;j++ )
      {
        commWork->beacon.callTarget[i][j] = 0xFF;
      }
    }
    
#if (defined(SDK_TWL))
    if( OS_IsRunOnTwl() == TRUE &&
        OS_IsRestrictPhotoExchange() == FALSE )
#else
    if( TRUE )
#endif
    {
      commWork->beacon.canUseCamera = 1;
    }
    else
    {
      commWork->beacon.canUseCamera = 0;
    }
  }
  
  //自身のデータセット
  {
    const COMM_TVT_INIT_WORK *initWork = COMM_TVT_GetInitWork( work );
    MYSTATUS *myStatus = GAMEDATA_GetMyStatus( initWork->gameData );
    
    MyStatus_Copy( myStatus , (MYSTATUS*)&commWork->selfData.myStatusBuff );
    
    commWork->selfData.canUseCamera = commWork->beacon.canUseCamera;
    
    
  }
  
  commWork->isLowerDataInit = TRUE;
  GFL_NET_LDATA_InitSystem( heapId );
  
  return commWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_COMM_TermSystem( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  u8 i;
  
  GFL_NET_LDATA_ExitSystem();
  GFL_HEAP_FreeMemory( commWork->decodeWaveBuf );
  GFL_HEAP_FreeMemory( commWork->postWaveBuf );
  GFL_NET_Align32Free( commWork->encBuffer );
  GFL_NET_Align32Free( commWork->sendBuffer );
  GFL_NET_Align32Free( commWork->encWorkBuf );
  for( i=0;i<CTVT_MEMBER_NUM-1;i++ )
  {
    GFL_NET_Align32Free( commWork->postPhotoBuf[i] );
  }
  GFL_HEAP_FreeMemory( commWork );
  
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void CTVT_COMM_Main( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
#if DEBUG_ONLY_FOR_ariizumi_nobuhiko&0
  {
    static CTVT_COMM_STATE befState = CCS_NONE;
    if( befState != commWork->state )
    {
      OS_TFPrintf(3,"CommState[%d]->[%d]\n",befState,commWork->state);
      befState = commWork->state;
    }
  }
#endif //DEBUG_ONLY_FOR_ariizumi_nobuhiko
  switch( commWork->state )
  {
  case CCS_NONE:
    if( commWork->nextMode != CCIM_NONE )
    {
      if( commWork->nextMode == CCIM_CONNECTED )
      {
        GFL_NET_SetWifiBothNet(FALSE);
      
        commWork->state = CCS_ADD_COMMAND;
        commWork->mode = commWork->nextMode;
      }
      else
      {
        commWork->state = CCS_INIT_COMM;
      }
    }
    break;
  case CCS_INIT_COMM:
    CTVT_COMM_InitComm( work , commWork );
    commWork->state = CCS_INIT_COMM_WAIT;
    break;
  case CCS_INIT_COMM_WAIT:
    if( GFL_NET_IsInit() == TRUE )
    {
      switch( commWork->nextMode )
      {
      case CCIM_SCAN:
        GFL_NET_StartBeaconScan();
        commWork->state = CCS_CONNECT;
        break;
      case CCIM_PARENT:
        GFL_NET_InitServer();
        commWork->state = CCS_REQ_NEGOTIATION;
        break;
      case CCIM_CHILD:
        GFL_NET_InitClientAndConnectToParent( commWork->targetMacAddress );
        commWork->state = CCS_REQ_NEGOTIATION;
        break;
      }
      commWork->mode = commWork->nextMode;
    }
    break;
    
  case CCS_REQ_NEGOTIATION:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
    {
      commWork->state = CCS_WAIT_NEGOTIATION;
    }
    break;

  case CCS_WAIT_NEGOTIATION:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE )
      {
        commWork->state = CCS_CONNECT;
      }
    }
    break;
    
  case CCS_ADD_COMMAND:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_AddCommandTable( GFL_NET_CMD_COMM_TVT , CTVT_COMM_RecvTable , NELEMS(CTVT_COMM_RecvTable) , commWork );
      GFL_NET_TimingSyncStart( selfHandle , CTVT_COMM_ADD_COMMAND_SYNC );
      commWork->state = CCS_ADD_COMMAND_SYNC_WAIT;
    }
    break;
  case CCS_ADD_COMMAND_SYNC_WAIT:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_IsTimingSync( selfHandle , CTVT_COMM_ADD_COMMAND_SYNC ) == TRUE )
      {
        commWork->state = CCS_CONNECT;
      }
    }
    break;

    
  case CCS_CONNECT:
    {
      CTVT_COMM_UpdateComm( work , commWork );
      if( commWork->mode != commWork->nextMode )
      {
        commWork->state = CCS_DISCONNECT;
      }
      if( commWork->mode == CCIM_SCAN )
      {
        CTVT_COMM_UpdateScan( work , commWork );
      }
    }
    break;
    
  case CCS_DISCONNECT:
    if( commWork->connectNum <= 1 )
    {
      GFL_NET_Exit(NULL);
      commWork->state = CCS_DISCONNECT_WAIT;
    }
    else
    {
      //if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_CMD_EXIT_REQ,0,NULL) == TRUE )
      GFL_NET_Exit(NULL);
      {
        commWork->state = CCS_DISCONNECT_WAIT;
      }
    }
    break;
    
  case CCS_DISCONNECT_WAIT:
    if( GFL_NET_IsExit() == TRUE )
    {
      if( commWork->nextMode == CCIM_END )
      {
        commWork->state = CCS_FINISH;
      }
      else
      {
        commWork->state = CCS_INIT_COMM;
      }
    }
    break;
    
  case CCS_RELEASE_COMMAND:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_TimingSyncStart( selfHandle , CTVT_COMM_RELEASE_COMMAND_SYNC );
      commWork->state = CCS_RELEASE_COMMAND_SYNC_WAIT;
    }
    break;

  case CCS_RELEASE_COMMAND_SYNC_WAIT:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_IsTimingSync( selfHandle , CTVT_COMM_RELEASE_COMMAND_SYNC ) == TRUE )
      {
        GFL_NET_DelCommandTable( GFL_NET_CMD_COMM_TVT );
        commWork->state = CCS_FINISH;
      }
    }
    break;

  }
  
}

#pragma mark [>comm system
//--------------------------------------------------------------
//	通信初期化
//--------------------------------------------------------------
static void CTVT_COMM_InitComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  GFLNetInitializeStruct aGFLNetInit = 
  {
    CTVT_COMM_RecvTable, //NetSamplePacketTbl,  // 受信関数テーブル
    NELEMS(CTVT_COMM_RecvTable), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL, ///< ネゴシエーション完了時にコール
    NULL, // ユーザー同士が交換するデータのポインタ取得関数
    NULL, // ユーザー同士が交換するデータのサイズ取得関数
    CTVT_COMM_GetBeaconData,   // ビーコンデータ取得関数  
    CTVT_COMM_GetBeaconSize,   // ビーコンデータサイズ取得関数 
    CTVT_COMM_BeacomCompare, // ビーコンのサービスを比較して繋いで良いかどうか判断する
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
    0x222,                  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,         //元になるheapid
    HEAPID_NETWORK,         //通信用にcreateされるHEAPID
    HEAPID_WIFI,            //wifi用にcreateされるHEAPID
    HEAPID_NETWORK,         //
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    4,                      //_MAXNUM,  //最大接続人数
    110,                    //_MAXSIZE, //最大送信バイト数
    CTVT_COMM_BEACON_NUM,   //_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,                   // CRC計算
    FALSE,                  // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,  //通信タイプの指定
    TRUE,                   // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_COMM_TVT,     //GameServiceID
#if GFL_NET_IRC
    IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    0,    ///< MP通信時に親の送信量を増やしたい場合サイズ指定 そうしない場合０
    0,    ///< dummy
  };
  const HEAPID heapId = COMM_TVT_GetHeapId( work );

  GFL_NET_Init( &aGFLNetInit , NULL , (void*)commWork );
  GFL_NET_SetNoChildErrorCheck( FALSE );
}

//--------------------------------------------------------------
//	通信終了
//--------------------------------------------------------------
void CTVT_COMM_ExitComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  if( commWork->connectNum <= 1 )
  {
    if( commWork->mode == CCIM_CONNECTED )
    {
      commWork->state = CCS_FINISH;
      GFL_NET_DelCommandTable( GFL_NET_CMD_COMM_TVT );
    }
    else
    {
      GFL_NET_Exit(NULL);
      commWork->state = CCS_DISCONNECT_WAIT;
    }
  }
  else
  {
    if( commWork->mode == CCIM_CONNECTED )
    {
      commWork->state = CCS_RELEASE_COMMAND;
    }
    else
    {
      commWork->state = CCS_DISCONNECT;
    }
  }
  commWork->nextMode = CCIM_END;
}
const BOOL CTVT_COMM_IsExit( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  if( commWork->state == CCS_FINISH )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	モード変更
//--------------------------------------------------------------
void CTVT_COMM_SetMode( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_COMM_INIT_MODE mode )
{
  commWork->nextMode = mode;
}

//--------------------------------------------------------------
//	マックアドレス設定
//--------------------------------------------------------------
void CTVT_COMM_SetMacAddress( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 *address )
{
  u8 i;
  for( i=0;i<6;i++ )
  {
    commWork->targetMacAddress[i] = address[i];
  }
}

//--------------------------------------------------------------
//	ネットが始まってるか？
//--------------------------------------------------------------
const BOOL CTVT_COMM_IsInitNet( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  if( commWork->state == CCS_CONNECT )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
// ビーコンデータ取得関数  
//--------------------------------------------------------------
static void*  CTVT_COMM_GetBeaconData(void* pWork)
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  return (void*)&commWork->beacon;
}

//--------------------------------------------------------------
// ビーコンデータサイズ取得関数 
//--------------------------------------------------------------
static int CTVT_COMM_GetBeaconSize(void *pWork)
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  return sizeof( CTVT_COMM_BEACON );
}

//--------------------------------------------------------------
// ビーコンデータ比較関数 
//--------------------------------------------------------------
static BOOL CTVT_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2)
{
  if( GameServiceID2 == WB_NET_COMM_TVT ||
      GameServiceID2 == WB_NET_FIELDMOVE_SERVICEID)
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
// 通信中更新メイン
//--------------------------------------------------------------
static void CTVT_COMM_UpdateComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  //ステートの更新
  {
    const u8 connectNum = GFL_NET_GetConnectNum();
    if( commWork->connectNum != connectNum )
    {
      CTVT_COMM_RefureshCommState( work , commWork );
      COMM_TVT_RedrawName( work );
      if( commWork->connectNum < connectNum )
      {
        //新メンバー追加
        if( selfId == 0 )
        {
          //画面の拡縮設定同期
          commWork->reqSendDoubleFlg = TRUE;
          commWork->sendDoubleFlgValue = COMM_TVT_IsDoubleMode(work);
#if CTVT_COMM_SHARE_DRAW_BUFF
          commWork->reqSendDrawBuffer = TRUE;
          COMM_TVT_SetSusspendDraw( work , TRUE );
#endif //CTVT_COMM_SHARE_DRAW_BUFF
        }
      }
      commWork->reqSendMemberData = TRUE;
      commWork->connectNum = connectNum;
      commWork->beacon.connectNum = connectNum;
    }
  }
  
  //メンバーの更新
  {
    u8 i;
    for( i=0;i<CTVT_MEMBER_NUM;i++ )
    {
      CTVT_COMM_UpdateMemberState( work , commWork , &commWork->member[i] , i );
    }
    
  }

  //自身の更新
  {
    if( commWork->connectNum > 1 )
    {
      //写真
      if( commWork->isSendWait == TRUE )
      {
        if( GFL_NET_LDATA_IsFinishSend() == TRUE )
        {
          commWork->isSendWait = FALSE;
        }
      }
      else
      if( (commWork->reqCheckBit & commWork->photoReqBit) == commWork->reqCheckBit&&
          commWork->reqCheckBit != 0 )
      {
        if( COMM_TVT_GetSusspend( work ) == FALSE )
        {
          CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
          void *selfBuf = CTVT_CAMERA_GetSelfBuffer( work , camWork );
          const u32 bufSize = CTVT_CAMERA_GetBufferSize( work , camWork );
          u32 dataSize;
          GFL_STD_MemCopy32( (void*)selfBuf , (void*)commWork->encBuffer , bufSize );
          {
            dataSize = SSP_StartJpegEncoder(  commWork->encBuffer , 
                                              commWork->sendBuffer , 
                                              bufSize ,
                                              commWork->encWorkBuf , 
                                              CTVT_CAMERA_GetPhotoSizeX(work,camWork) ,
                                              CTVT_CAMERA_GetPhotoSizeY(work,camWork) ,
                                              CTVT_COMM_JPG_QUALITY ,
                                              CTVT_COMM_JPG_OUT_TYPE ,
                                              CTVT_COMM_JPG_OUT_OPT );
          }
          if( dataSize != 0 )
          {
            GFL_NET_LDATA_SetSendData( commWork->sendBuffer , dataSize , commWork->reqCheckBit , FALSE );
            //CTVT_TPrintf("Send[%d]!!\n",commWork->reqCheckBit);
            commWork->photoReqBit = 0;
            commWork->isSendWait = TRUE;
          }
          else
          {
            CTVT_TPrintf("JpegEncode Error!!\n");
          }
        }
      }
      
      //お絵描き
      if( commWork->drawBufNum > 0 )
      {
        const BOOL ret = CTVT_COMM_SendDrawData( work , commWork );
        if( ret == TRUE )
        {
          commWork->drawBufNum = 0;
        }
      }
      
      if( commWork->reqSendMemberData == TRUE )
      {
        const BOOL ret = CTVT_COMM_SendMemberData( work , commWork );
        if( ret == TRUE )
        {
          commWork->reqSendMemberData = FALSE;
        }
      }
    }
  }
  
  //親機の処理
  if( selfId == 0 )
  {
#if CTVT_COMM_SHARE_DRAW_BUFF
    //お絵描きバッファの送信
    if( commWork->reqSendDrawBuffer == TRUE )
    {
      const BOOL ret = CTVT_COMM_SendDrawBuff( work , commWork );
      if( ret == TRUE )
      {
        commWork->reqSendDrawBuffer = FALSE;
        commWork->sendingDrawBuffer = TRUE;
        commWork->postDrawBuffer = FALSE;
      }
    }
    if( commWork->sendingDrawBuffer == TRUE )
    {
      if( commWork->postDrawBuffer == TRUE )
      {
        DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( work );
        const u16 buffNo = DRAW_SYS_GetBufferTopPos( drawSys );
        const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_DRAW_BUFFER_NO , buffNo );
        if( ret == TRUE )
        {
          commWork->sendingDrawBuffer = FALSE;
          commWork->postDrawBuffer = FALSE;
        }
      }
    }
#endif //CTVT_COMM_SHARE_DRAW_BUFF

    if( commWork->reqSendDoubleFlg == TRUE )
    {
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_CHANGE_DOUBLE , commWork->sendDoubleFlgValue );
      if( ret == TRUE )
      {
        commWork->reqSendDoubleFlg = FALSE;
      }
    }
    
    if( commWork->updateReqTalk == TRUE )
    {
      u8 i;
      commWork->tempTalkMember = CTVT_COMM_INVALID_MEMBER;
      for( i=0;i<CTVT_MEMBER_NUM;i++ )
      {
        if( commWork->member[i].reqTalk == TRUE )
        {
          if( commWork->tempTalkMember == CTVT_COMM_INVALID_MEMBER )
          {
            commWork->tempTalkMember = i;
          }
          commWork->member[i].reqTalk = FALSE;
        }
      }
      if( commWork->tempTalkMember != commWork->talkMember )
      {
        commWork->updateTalkMember = TRUE;
      }
      commWork->updateReqTalk = FALSE;
    }
    
    if( commWork->updateTalkMember == TRUE )
    {
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_TALK_MEMBER , commWork->tempTalkMember );
      if( ret == TRUE )
      {
        commWork->updateTalkMember = FALSE;
      }
    }
  }
}

//--------------------------------------------------------------
// フィールドビーコンスキャン
//--------------------------------------------------------------
static void CTVT_COMM_UpdateScan( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  #if 0
  u8 i;
  WIH_DWC_MainLoopScanBeaconData();
  for( i=0;i<CTVT_COMM_SCAN_BEACON_NUM;i++ )
  {
    if( WIH_DWC_IsEnableBeaconData(i) == TRUE )
    {
      const WMBssDesc* beacon = WIH_DWC_GetBeaconData(i);
      const _GF_BSS_DATA_INFO* gfInfo = (_GF_BSS_DATA_INFO*)beacon->gameInfo.userGameInfo;

      OS_TPrintf("[%x:%x:%x:%x:%x:%x]\n",
                      beacon->bssid[0],
                      beacon->bssid[1],
                      beacon->bssid[2],
                      beacon->bssid[3],
                      beacon->bssid[4],
                      beacon->bssid[5]);
    }
  }
  #endif
}

#pragma mark [>comm util
//--------------------------------------------------------------
// 通信情報更新
//--------------------------------------------------------------
static void CTVT_COMM_RefureshCommState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  u8 i;
  u8 connectNum = 0;
  BOOL isUpdateState = FALSE;
  const u8 selfIdx = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  
  for( i=0;i<CTVT_MEMBER_NUM;i++ )
  {
    if( GFL_NET_IsConnectMember(i) == TRUE )
    {
      if( commWork->member[i].isEnable == FALSE )
      {
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        commWork->member[i].isEnable = TRUE;
        if( i == selfIdx )
        {
          //自分だった。
          commWork->member[i].isSelf = TRUE;
          CTVT_TPrintf("Join user[%d](self)\n",i);
        }
        else
        {
          u8 j;
          //空き受信バッファの検索
          for( j=0;j<CTVT_MEMBER_NUM-1;j++ )
          {
            if( (commWork->PhotobufUseBit & 1<<j) == 0 )
            {
              GFL_NET_LDATA_CreatePostBuffer_SetAddres( CTVT_BUFFER_SCR_SIZE/2 , i , heapId , commWork->postPhotoBuf[j] );
              commWork->PhotobufUseBit |= 1<<j;
              commWork->member[i].bufferNo = j;
              break;
            }
          }
          commWork->reqCheckBit |= (1<<i);
          commWork->member[i].state = CCSU_REQ_PHOTO;
          CTVT_TPrintf("Join user[%d] postBuf[%d]\n",i,j);
        }
        CTVT_CAMERA_SetNewMember( work , camWork , i );
        isUpdateState = TRUE;
      }
      connectNum++;
    }
    else
    {
      if( commWork->member[i].isEnable == TRUE )
      {
        commWork->member[i].isEnable = FALSE;
        if( commWork->member[i].isSelf == FALSE )
        {
          GFL_NET_LDATA_DeletePostBuffer( i );
        //バッファのクリア？
        //commWork->member[i].bufferNo
          commWork->PhotobufUseBit -= 1<<commWork->member[i].bufferNo;
        }
        isUpdateState = TRUE;
        commWork->reqCheckBit -= (1<<i);
        CTVT_TPrintf("Leave user[%d]\n",i);
        CTVT_COMM_ClearMemberState( work , commWork , &commWork->member[i] );
      }
    }
  }
  
  if( isUpdateState == TRUE )
  {
    COMM_TVT_SetConnectNum( work , connectNum );
    if( connectNum == 2 && selfIdx >= 1 )
    {
      //2人モードになったらインデックスは０か１
      COMM_TVT_SetSelfIdx( work , 1 );
    }
    else
    {
      COMM_TVT_SetSelfIdx( work , selfIdx );
    }
  }
  {
    CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(work);
    CTVT_CAMERA_SetWaitAllRefreshFlg( work , camWork );
  }
}

#pragma mark [>member
//--------------------------------------------------------------
// メンバー情報クリア
//--------------------------------------------------------------
static void CTVT_COMM_ClearMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state )
{
  state->isSelf = FALSE;
  state->isEnable = FALSE;
  state->isEnableData = FALSE;
  state->reqTalk = FALSE;
  state->bufferNo = 0xFF;
  state->state = CCSU_NONE;
}
//--------------------------------------------------------------
// メンバー更新
//--------------------------------------------------------------
static void CTVT_COMM_UpdateMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state , const u8 idx )
{
  CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
  if( state->isEnable == FALSE )
  {
    //不在
    return;
  }
  if( state->isSelf == TRUE )
  {
    //自分
    return;
  }
  
  switch( state->state )
  {
  case CCSU_REQ_PHOTO:
    {
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_REQ_PHOTO , 0 );
      if( ret == TRUE )
      {
        state->state = CCSU_POST_PHOTO_WAIT;
      }
    }
    break;
  case CCSU_POST_PHOTO_WAIT:
    if( GFL_NET_LDATA_IsFinishPost( idx ) == TRUE )
    {
      void* buffer = CTVT_CAMERA_GetBuffer( work , camWork , idx );
      s16 sizeX = CTVT_CAMERA_GetPhotoSizeX(work,camWork);
      s16 sizeY = CTVT_CAMERA_GetPhotoSizeY(work,camWork);
      SSP_StartJpegDecoder( GFL_NET_LDATA_GetPostData(idx) ,
                            GFL_NET_LDATA_GetPostDataSize(idx) ,
                            buffer ,
                            &sizeX ,
                            &sizeY ,
                            0 );
      CTVT_CAMERA_SetRefreshFlg( work , camWork , idx );
      state->state = CCSU_REQ_PHOTO;
      //CTVT_TPrintf("Post[%d]!!\n",idx);
    }
    break;
  case CCSU_POST_PHOTO:
    break;
  }
}

#pragma mark [>send/post

const void CTVT_COMM_SendFlgReq_Double( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const BOOL flg )
{
  if( CTVT_COMM_SendFlg( work , commWork , CCFT_CHANGE_DOUBLE , flg ) == FALSE )
  {
    commWork->reqSendDoubleFlg = TRUE;
    commWork->sendDoubleFlgValue = flg;
  }
}


typedef struct
{
  u32 value;
  u8 flg;
}CTVT_COMM_FLG_PACKET;
//--------------------------------------------------------------
// フラグ送信
//--------------------------------------------------------------
const BOOL CTVT_COMM_SendFlg( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 flg , const u32 value )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  CTVT_COMM_FLG_PACKET pkt;
  pkt.flg = flg;
  pkt.value = value;
  
  if( flg != CCFT_REQ_PHOTO )
  {
    CTVT_TPrintf("Send Flg[%d:%d].\n",pkt.flg,pkt.value);
  }
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_FLG , sizeof( CTVT_COMM_FLG_PACKET ) , 
      (void*)&pkt , TRUE , FALSE , FALSE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send Flg failue!\n");
    }
    return ret;
  }
  
}

//--------------------------------------------------------------
// フラグ受信
//--------------------------------------------------------------
static void CTVT_COMM_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  CTVT_COMM_FLG_PACKET *pkt = (CTVT_COMM_FLG_PACKET*)pData;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if( pkt->flg != CCFT_REQ_PHOTO )
  {
    CTVT_TPrintf("Post Flg [%d][%d:%d].\n",netID,pkt->flg,pkt->value);
  }
  switch( pkt->flg )
  {
  //COMMが使う
  case CCFT_REQ_PHOTO:
    if( netID != selfId )
    {
      commWork->photoReqBit |= 1<<netID;
      //CTVT_TPrintf("ReqBit [%d][%d].\n",commWork->photoReqBit,commWork->reqCheckBit);
    }
    break;
    
  //COMMが使う＆親機が子に通知する用
  case CCFT_TALK_MEMBER:
    commWork->talkMember = pkt->value;
    CTVT_TPrintf("TalkMember [%d].\n",pkt->value);
    break;
    
  case CCFT_DRAW_BUFFER_NO:
    {
      DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( commWork->parentWork );
      DRAW_SYS_SetRedrawBuffer( drawSys , pkt->value );
      COMM_TVT_SetSusspendDraw( commWork->parentWork , FALSE );
    }
    break;
    
  case CCFT_CHANGE_DOUBLE:
    {
      CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(commWork->parentWork);
      CTVT_CAMERA_SetWaitAllRefreshFlg( commWork->parentWork , camWork );
      COMM_TVT_SetDoubleMode_Flag( commWork->parentWork , pkt->value );
    }
    break;
  //外で使う
  case CCFT_REQ_TALK:  //会話要求通知
    if( selfId == 0 )
    {
      commWork->member[netID].reqTalk = TRUE;
      commWork->updateReqTalk = TRUE;
    }
    break;

  case CCFT_FINISH_TALK:  //会話終了通知
    if( netID != selfId )
    {
      CTVT_MIC_WORK *micWork = COMM_TVT_GetMicWork(commWork->parentWork);
      CTVT_MIC_PlayWave( micWork , commWork->postWaveBuf , commWork->waveSize , 127 , commWork->waveSpeed );
    }
    if( selfId == 0 )
    {
      commWork->member[netID].reqTalk = FALSE;
      commWork->updateReqTalk = TRUE;
    }
    break;
    
  case CCFT_CANCEL_TALK:  //会話取り消し通知 //何かしらうまくいかなかったとき
    if( netID == commWork->talkMember )
    {
      commWork->updateReqTalk = TRUE;
    }
    break;
    
  case CCFT_FINISH_PARENT:
    COMM_TVT_SetFinishReq( commWork->parentWork , TRUE );
    break;
  }
}

//--------------------------------------------------------------
// Wave送信
//--------------------------------------------------------------
const BOOL CTVT_COMM_SendWave( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , void* sendData )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  const CTVT_COMM_WAVE_HEADER *header = sendData;
  
  CTVT_TPrintf("Send Wave[%d][%x].\n",header->dataNo,header->recSize);
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_WAVE , sizeof( CTVT_COMM_WAVE_HEADER )+header->encSize , 
      sendData , TRUE , FALSE , TRUE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send Wave failue!\n");
    }
    else
    {
      commWork->isCommWave = TRUE;
    }
    return ret;
  }
}
//--------------------------------------------------------------
// Wave受信バッファ
//--------------------------------------------------------------
static u8*    CTVT_COMM_Post_WaveData_Buff( int netID, void* pWork , int size )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  commWork->isCommWave = TRUE;

  GFL_STD_MemClear32( (void*)commWork->postWaveBuf , (CTVT_SEND_WAVE_SIZE_ONE_REAL) + sizeof(CTVT_COMM_WAVE_HEADER) );
  return commWork->postWaveBuf;
}

//--------------------------------------------------------------
// Wave受信
//--------------------------------------------------------------
static void CTVT_COMM_Post_WaveData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  CTVT_MIC_WORK *micWork = COMM_TVT_GetMicWork(commWork->parentWork);
  const CTVT_COMM_WAVE_HEADER *header = commWork->postWaveBuf;
  void *waveBuffet = (void*)((u32)commWork->postWaveBuf+sizeof(CTVT_COMM_WAVE_HEADER));

  CTVT_TPrintf("Post Wave[%d]\n",header->dataNo );
  
  if( netID != GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() ) )
  {
    if( header->dataNo == 0 )
    {
      ENC_ADPCM_ResetParam();
      GFL_STD_MemClear32( (void*)commWork->decodeWaveBuf , CTVT_SEND_WAVE_SIZE );
    }
    
    {
      void *setBuffer = (void*)((u32)commWork->decodeWaveBuf + header->dataNo*CTVT_SEND_WAVE_SIZE_ONE );
      const u32 decSize = CTVT_MIC_DecodeData( micWork , waveBuffet , setBuffer , header->encSize );
    }

    if( header->isLast == TRUE )
    {
      commWork->waveSpeed = 0x8000 - (header->pitch*0x4000/CTVT_TALK_SLIDER_MOVE_Y);
      commWork->isFinishPostWave = TRUE;
      commWork->waveSize = header->recSize;
    }
    
  }
  commWork->isCommWave = FALSE;
  
}

//--------------------------------------------------------------
// お絵描き送信
//--------------------------------------------------------------
static const BOOL CTVT_COMM_SendDrawData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  
  CTVT_TPrintf("Send Draw[%d].\n",commWork->drawBufNum );
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_DRAW , sizeof(DRAW_SYS_PEN_INFO)*commWork->drawBufNum , 
      (void*)commWork->drawBuf , TRUE , TRUE , FALSE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send Draw failue!\n");
    }
    return ret;
  }
}

//--------------------------------------------------------------
// お絵描き受信
//--------------------------------------------------------------
static void CTVT_COMM_PostDrawData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys(commWork->parentWork);
  const DRAW_SYS_PEN_INFO *drawBuf = pData;
  const u8 num = size/sizeof(DRAW_SYS_PEN_INFO);
  u8 i;

  CTVT_TPrintf("Post Draw[%d]\n",num );
  for( i=0;i<num;i++ )
  {
    DRAW_SYS_SetPenInfo( drawSys , &drawBuf[i] );
  }
  
}

//--------------------------------------------------------------
// メンバーデータ送信(名前とか
//--------------------------------------------------------------
static const BOOL CTVT_COMM_SendMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  void *sendBuf = CTVT_COMM_GetSelfMemberData( work , commWork );
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  
  CTVT_TPrintf("Send MemberData.\n");
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_MEMBER_DATA , sizeof(CTVT_COMM_MEMBER_DATA) , 
      (void*)sendBuf , TRUE , TRUE , TRUE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send MemberData failue!\n");
    }
    return ret;
  }
  return FALSE;
}

static void CTVT_COMM_PostMemberData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  CTVT_TPrintf("Post MemberData[%d].\n",netID);
  if( commWork->member[netID].isEnableData == FALSE )
  {
    commWork->member[netID].isEnableData = TRUE;
    COMM_TVT_RedrawName(commWork->parentWork);
  }
}

static u8*  CTVT_COMM_PostMemberData_Buff( int netID, void* pWork , int size )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  return (u8*)CTVT_COMM_GetMemberData( commWork->parentWork , commWork , netID );
}


#if CTVT_COMM_SHARE_DRAW_BUFF
//--------------------------------------------------------------
// お絵描きバッファ送信(接続時、絵を共有
//--------------------------------------------------------------
static const BOOL CTVT_COMM_SendDrawBuff( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( work );
  void *sendBuf = DRAW_SYS_GetBufferAddress( drawSys );
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  
  CTVT_TPrintf("Send DrawBuff.\n");
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_DRAW_BUFF , sizeof(DRAW_SYS_PEN_INFO)*CTVT_DRAW_BUFFER_NUM , 
      (void*)sendBuf , TRUE , TRUE , TRUE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send DrawBuff failue!\n");
    }
    return ret;
  }
  /*
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      CCST_SEND_DRAW_BUFF , 256*192*2 , 
      (void*)G2_GetBG2ScrPtr() , TRUE , TRUE , TRUE );
    
    if( ret == FALSE )
    {
      CTVT_TPrintf("Send DrawBuff failue!\n");
    }
    return ret;
  }
  */
  return FALSE;
}

static void CTVT_COMM_PostDrawBuff( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  
  commWork->postDrawBuffer = TRUE;
}

static u8*  CTVT_COMM_Post_DrawBuff_Buff( int netID, void* pWork , int size )
{
  CTVT_COMM_WORK *commWork = (CTVT_COMM_WORK*)pWork;
  
  DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( commWork->parentWork );
  COMM_TVT_SetSusspendDraw( commWork->parentWork , TRUE );
  
  return DRAW_SYS_GetBufferAddress( drawSys );
//  return G2_GetBG2ScrPtr();
}
#endif //CTVT_COMM_SHARE_DRAW_BUFF

#pragma mark [>outer func
const u8 CTVT_COMM_GetSelfNetId( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
}
const u8 CTVT_COMM_GetTalkMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return commWork->talkMember;
}
const BOOL CTVT_COMM_GetCommWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return commWork->isCommWave;
}
const BOOL CTVT_COMM_ReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return commWork->isFinishPostWave;
}
void CTVT_COMM_ResetReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  commWork->isFinishPostWave = FALSE;
}

CTVT_COMM_BEACON* CTVT_COMM_GetCtvtBeaconData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return &commWork->beacon;
}

const BOOL CTVT_COMM_IsEnableMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx )
{
  return commWork->member[idx].isEnableData;
}

CTVT_COMM_MEMBER_DATA* CTVT_COMM_GetMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx )
{
  return &commWork->member[idx].data;
}

CTVT_COMM_MEMBER_DATA* CTVT_COMM_GetSelfMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  return &commWork->selfData;
}

const BOOL CTVT_COMM_IsEnableMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx )
{
  return commWork->member[idx].isEnable;
}

//お絵描きバッファ取得
DRAW_SYS_PEN_INFO* CTVT_COMM_GetDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , BOOL *isFull )
{
  if( commWork->drawBufNum == CTVT_COMM_DRAW_BUF_NUM )
  {
    *isFull = TRUE;
    return &commWork->drawBuf[CTVT_COMM_DRAW_BUF_NUM-1];
  }
  *isFull = FALSE;
  return &commWork->drawBuf[commWork->drawBufNum];
}

//お絵描きバッファ追加通知
void CTVT_COMM_AddDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  if( commWork->drawBufNum < CTVT_COMM_DRAW_BUF_NUM )
  {
    commWork->drawBufNum++;
  }
}
