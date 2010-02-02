//======================================================================
/**
 * @file	ctvt_camera.c
 * @brief	通信TVTシステム：カメラ+上画面管理
 * @author	ariizumi
 * @data	09/12/16
 *
 * モジュール名：CTVT_CAMERA
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"

#include "comm_tvt.naix"
#include "ctvt_camera.h"
#include "ctvt_comm.h"
#include "ctvt_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CTVT_CAMERA_ANIME_SPD (8)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  BOOL isUpdate;
  BOOL isNew;
  BOOL isFirst; //効果音処理用
  u16 dispPosX;
  u16 dispPosY;
  u16 dispSizeX;
  u16 dispSizeY;
  
  u16 targetPosX;
  u16 targetPosY;
  u16 targetSizeX;
  u16 targetSizeY;
}CTVT_CAMERA_MEMBER_WORK;


struct _CTVT_CAMERA_WORK
{
  HEAPID camHeapId;
  
  void *scrBuf;
  void *picBuf;
  void *picBufDouble;
  
  u8   isUpdateBit;
  BOOL isWaitAllRefresh;
  BOOL isDispDouble;
  u8   waitAllConut;  //2回待たないといけない
  
  CTVT_CAMERA_MEMBER_WORK memWork[CTVT_MEMBER_NUM];
  
  //以下TWL時のみ作られるもの
  CAMERA_SYSTEM_WORK *camSys;       //  1サイズ*2 //0x30000
  void *cnvBuf;   //変換用バッファ  //  1/2サイズ //0x0c000
  void *sendBuf;  //送信用バッファ  //  1/2サイズ //0x0c000
  void *tempBuf;  //一時用バッファ  //  1/2サイズ //0x0c000
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_CAMERA_CapCallBack( void *captureArea , void *userWork );
static void CTVT_CAMERA_SetMemberState_Disp( CTVT_CAMERA_MEMBER_WORK *memWork , const u16 posX , const u16 posY , const u16 sizeX , const u16 sizeY );
static void CTVT_CAMERA_SetMemberState_Target( CTVT_CAMERA_MEMBER_WORK *memWork , const u16 posX , const u16 posY , const u16 sizeX , const u16 sizeY );
static const BOOL CTVT_CAMERA_UpdateMemberState( CTVT_CAMERA_WORK *camWork , CTVT_CAMERA_MEMBER_WORK *memWork );
static const BOOL CTVT_CAMERA_UpdateMemberState_Util( CTVT_CAMERA_WORK *camWork , u16 *now , u16 *target );
static void CTVT_CAMERA_SetMemberState_TargetPos( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 tempIdx );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_CAMERA_WORK* CTVT_CAMERA_Init( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_CAMERA_WORK* camWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_CAMERA_WORK) );
  ARCHANDLE *arcHandle = COMM_TVT_GetArcHandle( work );
  if( COMM_TVT_CanUseCamera() == TRUE )
  {
    camWork->camHeapId = HEAPID_CTVT_CAMERA;
    GFL_HEAP_CreateHeap( GFL_HEAPID_TWL, HEAPID_CTVT_CAMERA, 0x80000 );
    
    camWork->camSys = CAMERA_SYS_InitSystem( camWork->camHeapId );
    CAMERA_SYS_SetResolution( camWork->camSys , CAMERA_SIZE_256x192 );
    CAMERA_SYS_SetCameraPos( camWork->camSys , CAMERA_SELECT_IN );
    CAMERA_SYS_CreateReadBuffer( camWork->camSys , 2 , GFL_HEAP_LOWID(camWork->camHeapId) );
    CAMERA_SYS_SetCaptureCallBack( camWork->camSys , CTVT_CAMERA_CapCallBack , work );
  
    CAMERA_SYS_StartCapture( camWork->camSys );

    camWork->cnvBuf  = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(camWork->camHeapId) , CTVT_BUFFER_SCR_SIZE/2 );
    camWork->sendBuf = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(camWork->camHeapId) , CTVT_BUFFER_SCR_SIZE/2 );
    camWork->tempBuf = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(camWork->camHeapId) , CTVT_BUFFER_SCR_SIZE/2 );
    camWork->picBuf = GFL_ARCHDL_UTIL_Load( arcHandle , NARC_comm_tvt_tv_t_ds_mode_ntft , FALSE , camWork->camHeapId );
    camWork->picBufDouble = GFL_ARCHDL_UTIL_Load( arcHandle , NARC_comm_tvt_tv_t_ds_mode_double_ntft , FALSE , camWork->camHeapId );
  }
  else
  {
    camWork->picBuf = GFL_ARCHDL_UTIL_Load( arcHandle , NARC_comm_tvt_tv_t_ds_mode_ntft , FALSE , heapId );
    camWork->picBufDouble = GFL_ARCHDL_UTIL_Load( arcHandle , NARC_comm_tvt_tv_t_ds_mode_double_ntft , FALSE , heapId );
  }
  
  camWork->isUpdateBit = 0;
  camWork->waitAllConut = 0;
  camWork->isWaitAllRefresh = FALSE;
  camWork->isDispDouble = FALSE;
  camWork->scrBuf = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapId) , CTVT_BUFFER_SCR_SIZE );
  
  for( i=0;i<CTVT_MEMBER_NUM;i++ )
  {
    CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[i] , 0 , 0 , 0 , 0 );
    CTVT_CAMERA_SetMemberState_Target( &camWork->memWork[i] , 0 , 0 , 0 , 0 );
    camWork->memWork[i].isUpdate = FALSE;
    camWork->memWork[i].isNew = TRUE;
    camWork->memWork[i].isFirst = TRUE;
  }
  return camWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_CAMERA_Term( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  GFL_HEAP_FreeMemory( camWork->scrBuf );
  GFL_HEAP_FreeMemory( camWork->picBufDouble );
  GFL_HEAP_FreeMemory( camWork->picBuf );
  if( COMM_TVT_CanUseCamera() == TRUE )
  {
    GFL_HEAP_FreeMemory( camWork->tempBuf );
    GFL_HEAP_FreeMemory( camWork->cnvBuf );
    GFL_HEAP_FreeMemory( camWork->sendBuf );
    CAMERA_SYS_ExitSystem( camWork->camSys );
    GFL_HEAP_DeleteHeap( HEAPID_CTVT_CAMERA );
  }

  GFL_HEAP_FreeMemory( camWork );
  
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void CTVT_CAMERA_Main( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  u8 i;
  if( camWork->isWaitAllRefresh == FALSE )
  {
    for( i=0;i<CTVT_MEMBER_NUM;i++ )
    {
      const BOOL ret = CTVT_CAMERA_UpdateMemberState( camWork , &camWork->memWork[i] );
      if( ret == TRUE )
      {
        camWork->isUpdateBit |= 1<<i;
        break;
      }
    }
  }
  
  if( COMM_TVT_CanUseCamera() == TRUE )
  {
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
#if (defined(SDK_TWL))
    static effNum = 0;
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      effNum++;
      if( effNum >= CAMERA_EFFECT_MAX )
      {
        effNum = 0;
      }
      CAMERA_I2CEffect( CAMERA_SELECT_BOTH , effNum );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      CAMERA_SYS_SwapCameraPos( camWork->camSys );
    }
#endif
#endif
  }
}

//--------------------------------------------------------------
//	VBlank更新
//--------------------------------------------------------------
void CTVT_CAMERA_VBlank( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

  if( mode == CTDM_SINGLE )
  {
    if( camWork->isUpdateBit != 0 )
    {
      DC_FlushRange( camWork->scrBuf , CTVT_BUFFER_SCR_SIZE );
      GX_LoadBG3Bmp( camWork->scrBuf , 0 , CTVT_BUFFER_SCR_SIZE );
      camWork->isUpdateBit = 0;
    }
  }
  else
  {
    u8 i;
    
    if( camWork->isWaitAllRefresh == TRUE )
    {
      u8 num = 0;
      //全員到着までの待機
      for( i=0;i<CTVT_MEMBER_NUM;i++ )
      {
        if( camWork->isUpdateBit & (1<<i) )
        {
          num++;
        }
      }
      if( num != COMM_TVT_GetConnectNum( work ) )
      {
        return;
      }
      if( camWork->waitAllConut < 2 )
      {
        camWork->isUpdateBit = 0;
        camWork->waitAllConut++;

        for( i=0;i<CTVT_MEMBER_NUM;i++ )
        {
          if( CTVT_COMM_IsEnableMemberData( work , commWork , i ) == TRUE && 
              CTVT_COMM_CanUseCameraMember( work , commWork , i ) == FALSE )
          {
            camWork->isUpdateBit |= 1<<i;
          }
        }
        return;
      }
      else
      {
        u8 i;
        //絵がそろったら許可＆倍角の反映
        GFL_STD_MemFill32( G2_GetBG3ScrPtr() ,0x00000000 , CTVT_BUFFER_SCR_SIZE );
        camWork->isWaitAllRefresh = FALSE;
        if( camWork->isDispDouble != COMM_TVT_IsDoubleMode(work) )
        {
          //BGの拡大
          MtxFx22 mtx;
          camWork->isDispDouble = COMM_TVT_IsDoubleMode(work);
          if( camWork->isDispDouble == TRUE )
          {
            mtx._00 = FX_Inv(FX32_ONE*2);
            mtx._11 = FX_Inv(FX32_ONE*2);
          }
          else
          {
            mtx._00 = FX_Inv(FX32_ONE);
            mtx._11 = FX_Inv(FX32_ONE);
          }
          mtx._01 = 0;
          mtx._10 = 0;
          G2_SetBG3Affine(&mtx,0,0,0,0);
          for( i=0;i<CTVT_MEMBER_NUM;i++ )
          {
            if( camWork->isDispDouble == TRUE )
            {
              camWork->memWork[i].targetPosX  /= 2;
              camWork->memWork[i].targetPosY  /= 2;
              camWork->memWork[i].targetSizeX /= 2;
              camWork->memWork[i].targetSizeY /= 2;
              camWork->memWork[i].dispPosX  /= 2;
              camWork->memWork[i].dispPosY  /= 2;
              camWork->memWork[i].dispSizeX /= 2;
              camWork->memWork[i].dispSizeY /= 2;
            }
            else
            {
              camWork->memWork[i].targetPosX  *= 2;
              camWork->memWork[i].targetPosY  *= 2;
              camWork->memWork[i].targetSizeX *= 2;
              camWork->memWork[i].targetSizeY *= 2;
              camWork->memWork[i].dispPosX  *= 2;
              camWork->memWork[i].dispPosY  *= 2;
              camWork->memWork[i].dispSizeX *= 2;
              camWork->memWork[i].dispSizeY *= 2;
            }
          }
        }
      }
    }

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
    {
      static u8 befBit = 0;
      if( befBit != camWork->isUpdateBit )
      {
        OS_TFPrintf(2,"[%d]-?[%d]\n",befBit,camWork->isUpdateBit);
      }
    }
    
    for( i=0;i<CTVT_MEMBER_NUM;i++ )
    {
      if( camWork->isUpdateBit & (1<<i) )
      {
        u16 scrPosX;
        u16 scrPosY;
        u16 scrSizeX;
        u16 scrSizeY;
        u32 bufferBase;
        int iy;
        u8 idx = i;
        if( mode == CTDM_DOUBLE && idx != 0 )
        {
          idx = 1;
        }
        
        if( isDouble == FALSE )
        {
          scrPosX = ( idx == 0 || idx == 2 ? 0 : 128 );
          scrPosY = ( idx == 0 || idx == 1 ? 0 :  96 );
          scrSizeX = ( mode == CTDM_DOUBLE ?128 :128 );
          scrSizeY = ( mode == CTDM_DOUBLE ?192 : 96 );
        }
        else
        {
          scrPosX = ( idx == 0 || idx == 2 ? 0 : 64 );
          scrPosY = ( idx == 0 || idx == 1 ? 0 : 48 );
          scrSizeX = ( mode == CTDM_DOUBLE ?64 :64 );
          scrSizeY = ( mode == CTDM_DOUBLE ?96 :48 );
        }
        
        if( CTVT_COMM_CanUseCameraMember( work , commWork , i ) == TRUE )
        {
          bufferBase = (u32)camWork->scrBuf + scrSizeX*scrSizeY*2*idx;
          DC_FlushRange( (void*)bufferBase , scrSizeX*scrSizeY*2 );
        }
        else
        {
          if( isDouble == FALSE )
          {
            if( mode == CTDM_DOUBLE )
            {
              bufferBase = (u32)camWork->picBuf;
              DC_FlushRange( (void*)bufferBase , 128*192*2 );
            }
            else
            {
              bufferBase = (u32)camWork->picBuf + 128*48*2;
              DC_FlushRange( (void*)bufferBase , 128*96*2 );
            }
          }
          else
          {
            if( mode == CTDM_DOUBLE )
            {
              bufferBase = (u32)camWork->picBufDouble;
              DC_FlushRange( (void*)bufferBase , 64*96*2 );
            }
            else
            {
              bufferBase = (u32)camWork->picBufDouble + 64*24*2;
              DC_FlushRange( (void*)bufferBase , 64*48*2 );
            }
          }
        }
        
        for( iy=0;iy<camWork->memWork[i].dispSizeY;iy++ )
        {
          const u32 bufX = scrSizeX-camWork->memWork[i].dispSizeX;
          const u32 bufY = scrSizeY-camWork->memWork[i].dispSizeY+iy;
          const u32 bufTopAdr = bufferBase + bufY*scrSizeX*2 + bufX*2;
          
          const u32 scrTopAdr = (camWork->memWork[i].dispPosY+iy)*256*2 + camWork->memWork[i].dispPosX*2;
          GX_LoadBG3Bmp( (void*)bufTopAdr ,
                          scrTopAdr ,
                          camWork->memWork[i].dispSizeX*2 );
        }
        
        camWork->isUpdateBit -= 1<<i;
      }
    }
  }
}

//--------------------------------------------------------------
//	カメラ用コールバック
//--------------------------------------------------------------
static void CTVT_CAMERA_CapCallBack( void *captureArea , void *userWork )
{
  COMM_TVT_WORK *work = userWork;
  CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(work);
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );
  const u8 idx = COMM_TVT_GetSelfIdx( work );

  //サスペンドは完全停止、ポーズは更新扱い

  if( COMM_TVT_GetSusspend(work) == TRUE )
  {
    return;
  }

  if( camWork->isUpdateBit & (1<<idx) )
  {
    //CTVT_TPrintf("BufferIsBusy![%d][%d]\n",camWork->isUpdateBit,1<<idx);
    CTVT_TPrintf("!");
    return;
  }
  
  if( COMM_TVT_GetPause(work) == FALSE )
  {
    //tempバッファにコピー
    {
      int iy;
      const u16 capPosX = 64;
      const u16 capPosY = 0;
      const u16 capSizeX = 128;
      const u16 capSizeY = 192;
      
      for( iy=0;iy<capSizeY;iy++ )
      {
        const u32 capTopAdr = (u32)captureArea     + (capPosY+iy)*256*2 + capPosX*2;
        const u32 bufTopAdr = (u32)camWork->tempBuf + capSizeX*iy*2 ;
        GFL_STD_MemCopy32( (void*)capTopAdr , (void*)bufTopAdr , capSizeX*2 );
      }
    }
  }

  if( mode == CTDM_SINGLE )
  {
    //もうココには来ない
    //GFL_STD_MemCopy32( captureArea , camWork->scrBuf , CTVT_BUFFER_SCR_SIZE );
    GF_ASSERT_MSG(0,"mode CTDM_SINGLE is no support!\n");
  }
  else
  {
    int iy;
    //どこを切り取るか？
    u16 capPosX;
    u16 capPosY;
    u16 capSizeX;
    u16 capSizeY;
    u8 tempIdx = idx;
    if( mode == CTDM_DOUBLE && idx > 1 )
    {
      tempIdx = 1;
    }
    //この段階でサイズは1/2サイズ
    if( isDouble == FALSE )
    {
      capPosX = ( mode == CTDM_DOUBLE ?  0 :  0 );
      capPosY = ( mode == CTDM_DOUBLE ?  0 : 48 );
      capSizeX= ( mode == CTDM_DOUBLE ?128 :128 );
      capSizeY= ( mode == CTDM_DOUBLE ?192 : 96 );
    }
    else
    {
      capPosX = ( mode == CTDM_DOUBLE ? 32 : 32 );
      capPosY = ( mode == CTDM_DOUBLE ? 48 : 80 );
      capSizeX= ( mode == CTDM_DOUBLE ? 64 : 64 );
      capSizeY= ( mode == CTDM_DOUBLE ? 96 : 48 );
    }
    
    for( iy=0;iy<capSizeY;iy++ )
    {
      const u32 capTopAdr = (u32)camWork->tempBuf+ (capPosY+iy)*128*2 + capPosX*2;
      const u32 bufTopAdr = (u32)camWork->scrBuf + capSizeX*capSizeY*2*tempIdx + capSizeX*iy*2 ;
      GFL_STD_MemCopy32( (void*)capTopAdr , (void*)bufTopAdr , capSizeX*2 );
      
    }
  }
  camWork->isUpdateBit |= 1<<idx;
}

//--------------------------------------------------------------
//	更新フラグを立てる
//--------------------------------------------------------------
void CTVT_CAMERA_SetRefreshFlg( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 idx )
{
  camWork->isUpdateBit |= 1<<idx;
}

//--------------------------------------------------------------
//	全員更新待ちフラグを立てる
//--------------------------------------------------------------
void CTVT_CAMERA_SetWaitAllRefreshFlg( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  u8 i;
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  camWork->isWaitAllRefresh = TRUE;

  camWork->isUpdateBit = 0;
  camWork->waitAllConut = 0;
  if( COMM_TVT_CanUseCamera() == FALSE )
  {
    camWork->isUpdateBit |= 1<<COMM_TVT_GetSelfIdx( work );
  }
  
  for( i=0;i<CTVT_MEMBER_NUM;i++ )
  {
    if( CTVT_COMM_IsEnableMember( work , commWork , i ) == TRUE )
    {
      CTVT_CAMERA_SetMemberState_TargetPos( work , camWork , i );
    }
  }
}

//--------------------------------------------------------------
//	サイズの取得
//--------------------------------------------------------------
const u16 CTVT_CAMERA_GetPhotoSizeX( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );
  static const u32 sizeXArr[CTDM_MAX] = {256,128,128};
  if( isDouble == FALSE )
  {
    return sizeXArr[mode];
  }
  else
  {
    return sizeXArr[mode]/2;
  }
}
const u16 CTVT_CAMERA_GetPhotoSizeY( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );
  static const u32 sizeYArr[CTDM_MAX] = {192,192,96};
  if( isDouble == FALSE )
  {
    return sizeYArr[mode];
  }
  else
  {
    return sizeYArr[mode]/2;
  }
}
//--------------------------------------------------------------
//	バッファの取得
//--------------------------------------------------------------
void* CTVT_CAMERA_GetSelfBuffer( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const u8 idx = COMM_TVT_GetSelfIdx( work );
  return CTVT_CAMERA_GetBuffer( work , camWork , idx );
}
void* CTVT_CAMERA_GetBuffer( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 idx )
{
  const u32 size = CTVT_CAMERA_GetBufferSize( work , camWork );
  if( COMM_TVT_GetDispMode( work ) == CTDM_DOUBLE &&
      idx != 0 )
  {
    return (void*)((u32)camWork->scrBuf + size*1 );
  }
  return (void*)((u32)camWork->scrBuf + size*idx );
}
const u32 CTVT_CAMERA_GetBufferSize( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );
  static const u32 bufSizeArr[CTDM_MAX] = {(CTVT_BUFFER_SCR_SIZE),(CTVT_BUFFER_SCR_SIZE/2),(CTVT_BUFFER_SCR_SIZE/4)};
  
  if( isDouble == FALSE )
  {
    return bufSizeArr[mode];
  }
  else
  {
    return bufSizeArr[mode]/4;
  }
}

static void CTVT_CAMERA_SetMemberState_Disp( CTVT_CAMERA_MEMBER_WORK *memWork , const u16 posX , const u16 posY , const u16 sizeX , const u16 sizeY )
{
  memWork->isUpdate = TRUE;
  memWork->dispPosX  = posX;
  memWork->dispPosY  = posY;
  memWork->dispSizeX = sizeX;
  memWork->dispSizeY = sizeY;
}

static void CTVT_CAMERA_SetMemberState_Target( CTVT_CAMERA_MEMBER_WORK *memWork , const u16 posX , const u16 posY , const u16 sizeX , const u16 sizeY )
{
  memWork->isUpdate = TRUE;
  memWork->targetPosX  = posX;
  memWork->targetPosY  = posY;
  memWork->targetSizeX = sizeX;
  memWork->targetSizeY = sizeY;
}

static const BOOL CTVT_CAMERA_UpdateMemberState( CTVT_CAMERA_WORK *camWork ,CTVT_CAMERA_MEMBER_WORK *memWork )
{
  if( memWork->isUpdate == TRUE )
  {
    BOOL ret = FALSE;
    ret |= CTVT_CAMERA_UpdateMemberState_Util( camWork , &memWork->dispPosX  , &memWork->targetPosX  );
    ret |= CTVT_CAMERA_UpdateMemberState_Util( camWork , &memWork->dispPosY  , &memWork->targetPosY  );
    ret |= CTVT_CAMERA_UpdateMemberState_Util( camWork , &memWork->dispSizeX , &memWork->targetSizeX );
    ret |= CTVT_CAMERA_UpdateMemberState_Util( camWork , &memWork->dispSizeY , &memWork->targetSizeY );
    if( ret == FALSE )
    {
      memWork->isUpdate = FALSE;
      memWork->isNew = FALSE;
    }
    if( memWork->isFirst == TRUE )
    {
      memWork->isFirst = FALSE;
      PMSND_PlaySystemSE( CTVT_SND_SLIDE_IN );
    }

    return TRUE;
  }
  return FALSE;
}
 

static const BOOL CTVT_CAMERA_UpdateMemberState_Util( CTVT_CAMERA_WORK *camWork , u16 *now , u16 *target )
{
  const u8 spd = (camWork->isDispDouble == TRUE ? CTVT_CAMERA_ANIME_SPD/2 : CTVT_CAMERA_ANIME_SPD );
  if( *now < *target )
  {
    if( *now + spd < *target )
    {
      *now += spd;
    }
    else
    {
      *now = *target;
    }
    return TRUE;
  }
  if( *now > *target )
  {
    if( *now - spd > *target )
    {
      *now -= spd;
    }
    else
    {
      *now = *target;
    }
    return TRUE;
  }
  return FALSE;
}

static void CTVT_CAMERA_SetMemberState_TargetPos( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 tempIdx )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  u8 idx = tempIdx;
  
  if( mode == CTDM_DOUBLE && idx != 0 )
  {
    idx = 1;
  }
  if( camWork->isDispDouble == FALSE )
  {
    camWork->memWork[tempIdx].targetPosX = ( idx == 0 || idx == 2 ? 0 : 128 );
    camWork->memWork[tempIdx].targetPosY = ( idx == 0 || idx == 1 ? 0 :  96 );
    camWork->memWork[tempIdx].targetSizeX = ( mode == CTDM_DOUBLE ?128 :128 );
    camWork->memWork[tempIdx].targetSizeY = ( mode == CTDM_DOUBLE ?192 : 96 );
  }
  else
  {
    camWork->memWork[tempIdx].targetPosX = ( idx == 0 || idx == 2 ? 0 : 64 );
    camWork->memWork[tempIdx].targetPosY = ( idx == 0 || idx == 1 ? 0 : 48 );
    camWork->memWork[tempIdx].targetSizeX = ( mode == CTDM_DOUBLE ?64 :64 );
    camWork->memWork[tempIdx].targetSizeY = ( mode == CTDM_DOUBLE ?96 :48 );
  }
  
  if( camWork->memWork[tempIdx].isNew == FALSE )
  {
    camWork->memWork[tempIdx].dispPosX  = camWork->memWork[tempIdx].targetPosX;
    camWork->memWork[tempIdx].dispPosY  = camWork->memWork[tempIdx].targetPosY;
    camWork->memWork[tempIdx].dispSizeX = camWork->memWork[tempIdx].targetSizeX;
    camWork->memWork[tempIdx].dispSizeY = camWork->memWork[tempIdx].targetSizeY;
  }
  else
  {
    camWork->memWork[tempIdx].isFirst = TRUE;
    switch( idx )
    {
      case 0:
        if( camWork->isDispDouble == FALSE )
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           0,
                                           0,
                                           128 ,
                                           0 );
        }
        else
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           0,
                                           0,
                                           64 ,
                                           0 );
        }
        break;
      case 1:
        if( camWork->isDispDouble == FALSE )
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           128,
                                           0,
                                           0 ,
                                           ( mode == CTDM_DOUBLE ?192 : 96 ) );
        }
        else
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           64,
                                           0,
                                           0 ,
                                           ( mode == CTDM_DOUBLE ?96 : 48 ) );
        }
        break;
      case 2:
        if( camWork->isDispDouble == FALSE )
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           0 ,
                                           96,
                                           128,
                                           0 );
        }
        else
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           0 ,
                                           48,
                                           64,
                                           0 );
        }
        break;
      case 3:
        if( camWork->isDispDouble == FALSE )
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           128,
                                           96,
                                           0 ,
                                           96 );
        }
        else
        {
          CTVT_CAMERA_SetMemberState_Disp( &camWork->memWork[tempIdx] , 
                                           64,
                                           48,
                                           0 ,
                                           48);
        }
        break;
    }
  }
  camWork->memWork[tempIdx].isUpdate = TRUE;
}

void CTVT_CAMERA_SetNewMember( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 idx )
{
  camWork->memWork[idx].isNew = TRUE;
  camWork->memWork[idx].isFirst = TRUE;
}

const BOOL CTVT_CAMERA_IsUpdateCameraAnime( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 idx )
{
  return camWork->memWork[idx].isUpdate;
}
void CTVT_CAMERA_SetUpdateCameraAnime( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork , const u8 idx , const u8 flg )
{
  camWork->memWork[idx].isUpdate = flg;
  CTVT_CAMERA_SetMemberState_TargetPos( work , camWork , idx );
}
