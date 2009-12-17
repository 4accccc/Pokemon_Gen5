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
  
  CAMERA_SYSTEM_WORK *camSys;
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
    GFL_HEAP_CreateHeap( GFL_HEAPID_TWL, HEAPID_CTVT_CAMERA, 0x40000 );
    
    camWork->camSys = CAMERA_SYS_InitSystem( camWork->camHeapId );
    CAMERA_SYS_SetResolution( camWork->camSys , CAMERA_SIZE_256x192 );
    CAMERA_SYS_SetCameraPos( camWork->camSys , CAMERA_SELECT_IN );
    CAMERA_SYS_CreateReadBuffer( camWork->camSys , 2 , GFL_HEAP_LOWID(camWork->camHeapId) );
    CAMERA_SYS_SetCaptureCallBack( camWork->camSys , CTVT_CAMERA_CapCallBack , camWork );
  
    CAMERA_SYS_StartCapture( camWork->camSys );

  }
  
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
  if( COMM_TVT_IsTwlMode() == TRUE )
  {
  }
}

//--------------------------------------------------------------
//	カメラ用コールバック
//--------------------------------------------------------------
static void CTVT_CAMERA_CapCallBack( void *captureArea , void *userWork )
{
  CTVT_CAMERA_WORK* camWork = userWork;
  DC_FlushRange( captureArea , 256*192*2 );
  GX_LoadBG3Bmp( captureArea , 0 , 256*192*2 );
}


