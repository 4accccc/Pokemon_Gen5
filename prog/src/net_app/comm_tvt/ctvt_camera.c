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

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _CTVT_CAMERA_WORK
{
  HEAPID camHeapId;
  
  void *scrBuf;
  void *picBuf;
  
  u8   isUpdateBit;
  BOOL isWaitAllRefresh;
  u8   waitAllConut;  //2回待たないといけない
  
  //以下TWL時のみ作られるもの
  CAMERA_SYSTEM_WORK *camSys;       //  1サイズ*2 //0x30000
  void *cnvBuf;   //変換用バッファ  //  1/2サイズ //0x0c000
  void *sendBuf;  //送信用バッファ  //  1/2サイズ //0x0c000
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_CAMERA_CapCallBack( void *captureArea , void *userWork );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_CAMERA_WORK* CTVT_CAMERA_Init( COMM_TVT_WORK *work , const HEAPID heapId )
{
  CTVT_CAMERA_WORK* camWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_CAMERA_WORK) );
  if( COMM_TVT_IsTwlMode() == TRUE )
  {
    camWork->camHeapId = HEAPID_CTVT_CAMERA;
    GFL_HEAP_CreateHeap( GFL_HEAPID_TWL, HEAPID_CTVT_CAMERA, 0x50000 );
    
    camWork->camSys = CAMERA_SYS_InitSystem( camWork->camHeapId );
    CAMERA_SYS_SetResolution( camWork->camSys , CAMERA_SIZE_256x192 );
    CAMERA_SYS_SetCameraPos( camWork->camSys , CAMERA_SELECT_IN );
    CAMERA_SYS_CreateReadBuffer( camWork->camSys , 2 , GFL_HEAP_LOWID(camWork->camHeapId) );
    CAMERA_SYS_SetCaptureCallBack( camWork->camSys , CTVT_CAMERA_CapCallBack , work );
  
    CAMERA_SYS_StartCapture( camWork->camSys );

    camWork->cnvBuf  = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(camWork->camHeapId) , CTVT_BUFFER_SCR_SIZE/2 );
    camWork->sendBuf = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(camWork->camHeapId) , CTVT_BUFFER_SCR_SIZE/2 );
  }
  else
  {
    ARCHANDLE *arcHandle = COMM_TVT_GetArcHandle( work );
    camWork->picBuf = GFL_ARCHDL_UTIL_Load( arcHandle , NARC_comm_tvt_tv_t_ds_mode_ntft , FALSE , heapId );
  }
  
  camWork->isUpdateBit = 0;
  camWork->waitAllConut = 0;
  camWork->isWaitAllRefresh = FALSE;
  camWork->scrBuf = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapId) , CTVT_BUFFER_SCR_SIZE );
  return camWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_CAMERA_Term( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  GFL_HEAP_FreeMemory( camWork->scrBuf );
  if( COMM_TVT_IsTwlMode() == TRUE )
  {
    GFL_HEAP_FreeMemory( camWork->cnvBuf );
    GFL_HEAP_FreeMemory( camWork->sendBuf );
    CAMERA_SYS_ExitSystem( camWork->camSys );
    GFL_HEAP_DeleteHeap( HEAPID_CTVT_CAMERA );
  }
  else
  {
    GFL_HEAP_FreeMemory( camWork->picBuf );
  }

  GFL_HEAP_FreeMemory( camWork );
  
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void CTVT_CAMERA_Main( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
  {
    GFL_STD_MemFill32( G2_GetBG3ScrPtr() ,0x80008000 , CTVT_BUFFER_SCR_SIZE );
    if( COMM_TVT_IsTwlMode() == FALSE )
    {
      CTVT_CAMERA_CapCallBack( camWork->picBuf , work );
    }
  }
  
  if( COMM_TVT_IsTwlMode() == TRUE )
  {
  }
}

//--------------------------------------------------------------
//	VBlank更新
//--------------------------------------------------------------
void CTVT_CAMERA_VBlank( COMM_TVT_WORK *work , CTVT_CAMERA_WORK *camWork )
{
  const COMM_TVT_DISP_MODE mode = COMM_TVT_GetDispMode( work );
  const BOOL isDouble = COMM_TVT_IsDoubleMode( work );

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
      if( camWork->waitAllConut == 0 )
      {
        camWork->waitAllConut++;
        return;
      }
      else
      {
        GFL_STD_MemFill32( G2_GetBG3ScrPtr() ,0x80008000 , CTVT_BUFFER_SCR_SIZE );
        camWork->isWaitAllRefresh = FALSE;
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
        bufferBase = (u32)camWork->scrBuf + scrSizeX*scrSizeY*2*idx;
        DC_FlushRange( (void*)bufferBase , scrSizeX*scrSizeY*2 );
        
        for( iy=0;iy<scrSizeY;iy++ )
        {
          const u32 bufTopAdr = bufferBase + iy*scrSizeX*2;
          const u32 scrTopAdr = (scrPosY+iy)*256*2 + scrPosX*2;
          GX_LoadBG3Bmp( (void*)bufTopAdr , scrTopAdr , scrSizeX*2 );
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
  
  if( mode == CTDM_SINGLE )
  {
    GFL_STD_MemCopy32( captureArea , camWork->scrBuf , CTVT_BUFFER_SCR_SIZE );
  }
  else
  {
    int iy;
    //どこを切り取るか？
    u16 capPosX;
    u16 capPosY;
    u16 capSizeX;
    u16 capSizeY;
    //どこに貼り付けるか？
    u32 bufTopAdr;
    if( isDouble == FALSE )
    {
      capPosX = ( mode == CTDM_DOUBLE ? 64 : 64 );
      capPosY = ( mode == CTDM_DOUBLE ?  0 : 48 );
      capSizeX= ( mode == CTDM_DOUBLE ?128 :128 );
      capSizeY= ( mode == CTDM_DOUBLE ?192 : 96 );
    }
    else
    {
      capPosX = ( mode == CTDM_DOUBLE ? 96 : 96 );
      capPosY = ( mode == CTDM_DOUBLE ? 48 : 80 );
      capSizeX= ( mode == CTDM_DOUBLE ? 64 : 64 );
      capSizeY= ( mode == CTDM_DOUBLE ? 96 : 48 );
    }
    bufTopAdr = capSizeX*capSizeY*idx;
    
    
    for( iy=0;iy<capSizeY;iy++ )
    {
      const u32 capTopAdr = (u32)captureArea     + (capPosY+iy)*256*2 + capPosX*2;
      const u32 bufTopAdr = (u32)camWork->scrBuf + capSizeX*capSizeY*2*idx + capSizeX*iy*2 ;
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
  camWork->isWaitAllRefresh = TRUE;

  camWork->isUpdateBit = 0;
  camWork->waitAllConut = 0;
  if( COMM_TVT_IsTwlMode() == FALSE )
  {
    CTVT_CAMERA_CapCallBack( camWork->picBuf , work );
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
    return bufSizeArr[mode]/2;
  }
}

