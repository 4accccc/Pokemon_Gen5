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

#include "ctvt_comm.h"
#include "ctvt_camera.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
//JPG変換係
#define CTVT_COMM_JPG_OUT_TYPE (SSP_JPEG_OUTPUT_YUV444)
#define CTVT_COMM_JPG_OUT_OPT  (SSP_JPEG_RGB555)
#define CTVT_COMM_JPG_QUALITY  (70)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CCS_NONE,
  CCS_INIT_COMM,
  CCS_INIT_COMM_WAIT,

  CCS_CONNECT,
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
  
  CCST_MAX,
}CTVT_COMM_SEND_TYPE;
//フラグのタイプ
typedef enum
{
  CCFT_REQ_PHOTO,
  
  CCFT_MAX,
}CTVT_COMM_FLAG_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//ユーザーデータ
typedef struct
{
  BOOL isEnable;
  BOOL isSelf;
  u8   bufferNo;
  
  CTVT_COMM_STATE_USER state;
}CTVT_MEMBER_STATE;

//ビーコンデータ
typedef struct
{
  u32 dummy;
}CTVT_COMM_BEACON;

//ワーク
struct _CTVT_COMM_WORK
{
  u8 connectNum;
  CTVT_COMM_STATE state;
  
  CTVT_COMM_BEACON beacon;
  
  //メンバーの管理用
  CTVT_MEMBER_STATE member[CTVT_MEMBER_NUM];
  void *postPhotoBuf[CTVT_MEMBER_NUM-1];
  u8    PhotobufUseBit;
  
  //自身の管理用
  u8 reqCheckBit;
  u8 photoReqBit;
  BOOL isSendWait;
  void *encWorkBuf;
  void *encBuffer;
  void *sendBuffer;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_COMM_InitComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void CTVT_COMM_UpdateComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
static void*  CTVT_COMM_GetBeaconData(void* pWork);
static int CTVT_COMM_GetBeaconSize(void *pWork);
static BOOL CTVT_COMM_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2);

static void CTVT_COMM_RefureshCommState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

static void CTVT_COMM_ClearMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state );
static void CTVT_COMM_UpdateMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state , const u8 idx );

static const BOOL CTVT_COMM_SendFlg( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 flg , const u32 value );
static void CTVT_COMM_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static const NetRecvFuncTable CTVT_COMM_RecvTable[] = 
{
  { CTVT_COMM_PostFlg   ,NULL  },
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_COMM_WORK* CTVT_COMM_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_COMM_WORK* commWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_COMM_WORK) );
  
  commWork->state = CCS_INIT_COMM;
  commWork->connectNum = 1;
  commWork->photoReqBit = 0;
  
  for( i=0;i<CTVT_MEMBER_NUM;i++ )
  {
    CTVT_COMM_ClearMemberState( work , commWork , &commWork->member[i] );
  }
  for( i=0;i<CTVT_MEMBER_NUM-1;i++ )
  {
    commWork->postPhotoBuf[i] = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  }
  {
//    const u32 encWorkSize = SSP_GetJpegEncoderBufferSize( 128,192,CTVT_COMM_JPG_OUT_TYPE,CTVT_COMM_JPG_OUT_OPT );
//    commWork->encWorkBuf = GFL_NET_Align32Alloc( heapId , encWorkSize );
  }
  commWork->encBuffer = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  commWork->sendBuffer = GFL_NET_Align32Alloc( heapId , CTVT_BUFFER_SCR_SIZE/2 );
  commWork->PhotobufUseBit = 0;
  commWork->reqCheckBit = 0;
  commWork->isSendWait = FALSE;
  return commWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_COMM_TermSystem( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  u8 i;
  GFL_HEAP_FreeMemory( commWork->encBuffer );
  GFL_HEAP_FreeMemory( commWork->sendBuffer );
  GFL_HEAP_FreeMemory( commWork->encWorkBuf );
  for( i=0;i<CTVT_MEMBER_NUM-1;i++ )
  {
    GFL_HEAP_FreeMemory( commWork->postPhotoBuf[i] );
  }
  GFL_HEAP_FreeMemory( commWork );
  
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void CTVT_COMM_Main( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork )
{
  switch( commWork->state )
  {
  case CCS_NONE:
    break;
  case CCS_INIT_COMM:
    CTVT_COMM_InitComm( work , commWork );
    commWork->state = CCS_INIT_COMM_WAIT;
    break;
  case CCS_INIT_COMM_WAIT:
    if( GFL_NET_IsInit() == TRUE )
    {
      GFL_NET_ChangeoverConnect( NULL );
      commWork->state = CCS_CONNECT;
    }
    break;
    
  case CCS_CONNECT:
    {
      CTVT_COMM_UpdateComm( work , commWork );
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
    0x333,                  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,         //元になるheapid
    HEAPID_NETWORK,         //通信用にcreateされるHEAPID
    HEAPID_WIFI,            //wifi用にcreateされるHEAPID
    HEAPID_NETWORK,         //
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    4,                      //_MAXNUM,  //最大接続人数
    110,                    //_MAXSIZE, //最大送信バイト数
    4,                      //_BCON_GET_NUM,  // 最大ビーコン収集数
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
  
  GFL_NET_LDATA_InitSystem( heapId );
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
  if( GameServiceID1 == GameServiceID2 )
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
  //ステートの更新
  {
    const u8 connectNum = GFL_NET_GetConnectNum();
    if( commWork->connectNum != connectNum )
    {
      CTVT_COMM_RefureshCommState( work , commWork );
      commWork->connectNum = connectNum;
      
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
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        void *selfBuf = CTVT_CAMERA_GetSelfBuffer( work , camWork );
        const u32 bufSize = CTVT_CAMERA_GetBufferSize( work , camWork );
        u32 dataSize;
        
        GFL_STD_MemCopy32( (void*)selfBuf , (void*)commWork->encBuffer , bufSize );
        {
          /*
          dataSize = SSP_StartJpegEncoder(  commWork->encBuffer , 
                                            commWork->sendBuffer , 
                                            bufSize ,
                                            commWork->encWorkBuf , 
                                            CTVT_CAMERA_GetPhotoSizeX(work,camWork) ,
                                            CTVT_CAMERA_GetPhotoSizeY(work,camWork) ,
                                            CTVT_COMM_JPG_QUALITY ,
                                            CTVT_COMM_JPG_OUT_TYPE ,
                                            CTVT_COMM_JPG_OUT_OPT );
          */
        }
        if( dataSize != 0 )
        {
          GFL_NET_LDATA_SetSendData( commWork->sendBuffer , dataSize , commWork->reqCheckBit , FALSE );
          CTVT_TPrintf("Send[%d]!!\n",commWork->reqCheckBit);
          commWork->photoReqBit = 0;
          commWork->isSendWait = TRUE;
        }
        else
        {
          CTVT_TPrintf("JpegEncode Error!!\n");
        }
      }
    }
  }
  
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
        isUpdateState = TRUE;
      }
      connectNum++;
    }
    else
    {
      if( commWork->member[i].isEnable == TRUE )
      {
        GFL_NET_LDATA_DeletePostBuffer( i );
        //バッファのクリア？
        //commWork->member[i].bufferNo
        commWork->PhotobufUseBit -= 1<<commWork->member[i].bufferNo;
        CTVT_COMM_ClearMemberState( work , commWork , &commWork->member[i] );
        commWork->reqCheckBit -= (1<<i);
        CTVT_TPrintf("Leave user[%d]\n",i);
        isUpdateState = TRUE;
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
}

#pragma mark [>member
//--------------------------------------------------------------
// メンバー情報クリア
//--------------------------------------------------------------
static void CTVT_COMM_ClearMemberState( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_MEMBER_STATE *state )
{
  state->isSelf = FALSE;
  state->isEnable = FALSE;
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
/*
      SSP_StartJpegDecoder( GFL_NET_LDATA_GetPostData(idx) ,
                            GFL_NET_LDATA_GetPostDataSize(idx) ,
                            buffer ,
                            &sizeX ,
                            &sizeY ,
                            0 );
*/
      CTVT_CAMERA_SetRefreshFlg( work , camWork , idx );
      state->state = CCSU_REQ_PHOTO;
      CTVT_TPrintf("Post[%d]!!\n",idx);
    }
    break;
  case CCSU_POST_PHOTO:
    break;
  }
}

#pragma mark [>send/post

typedef struct
{
  u32 value;
  u8 flg;
}CTVT_COMM_FLG_PACKET;
//--------------------------------------------------------------
// フラグ送信
//--------------------------------------------------------------
static const BOOL CTVT_COMM_SendFlg( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 flg , const u32 value )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  CTVT_COMM_FLG_PACKET pkt;
  pkt.flg = flg;
  pkt.value = value;
  //CTVT_TPrintf("Send Flg[%d:%d].\n",pkt.flg,pkt.value);
  
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

  //CTVT_TPrintf("Post Flg [%d][%d:%d].\n",netID,pkt->flg,pkt->value);
  switch( pkt->flg )
  {
  case CCFT_REQ_PHOTO:
    if( netID != selfId )
    {
      commWork->photoReqBit |= 1<<netID;
      CTVT_TPrintf("ReqBit [%d][%d].\n",commWork->photoReqBit,commWork->reqCheckBit);
    }
    break;
  }
}


