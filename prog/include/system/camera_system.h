//======================================================================
/**
 * @file	camera_system.h
 * @brief	TWLのカメラライブラリ
 * @author	ariizumi
 * @data	  09/05/18
 *
 * モジュール名：CAMERA
 */
//======================================================================

#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

#ifdef SDK_TWL
//CAMERASizeを再定義
enum
{
  CAMERA_SIZE_640x480 = CAMERA_SIZE_VGA,
  CAMERA_SIZE_320x240 = CAMERA_SIZE_QVGA,
  CAMERA_SIZE_160x120 = CAMERA_SIZE_QQVGA,
  CAMERA_SIZE_352x288 = CAMERA_SIZE_CIF,
  CAMERA_SIZE_176x144 = CAMERA_SIZE_QCIF,
  CAMERA_SIZE_256x192 = CAMERA_SIZE_DS_LCD,
  CAMERA_SIZE_512x384 = CAMERA_SIZE_DS_LCDx4,
};


#else
//CAMERASizeを再定義
typedef enum
{
  CAMERA_SIZE_640x480,
  CAMERA_SIZE_320x240,
  CAMERA_SIZE_160x120,
  CAMERA_SIZE_352x288,
  CAMERA_SIZE_176x144,
  CAMERA_SIZE_256x192,
  CAMERA_SIZE_512x384,
}CAMERASize;
typedef enum
{
    CAMERA_SELECT_NONE = 0,
    CAMERA_SELECT_IN   = (1<<0),
    CAMERA_SELECT_OUT  = (1<<1),
    CAMERA_SELECT_BOTH = (CAMERA_SELECT_IN|CAMERA_SELECT_OUT)
} CAMERASelect;

#endif

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
//ワーク
typedef struct _CAMERA_SYSTEM_WORK CAMERA_SYSTEM_WORK;

//キャプチャ完了時に呼ばれるコールバック
typedef void (*CAMERA_SYS_CaptureCallBack)( void* captureData , void* userWork );


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

CAMERA_SYSTEM_WORK* CAMERA_SYS_InitSystem( HEAPID heapId );
void CAMERA_SYS_ExitSystem( CAMERA_SYSTEM_WORK* work );

void CAMERA_SYS_CreateReadBuffer( CAMERA_SYSTEM_WORK* work , const int bufferNum , const HEAPID heapId );
const u32 CAMERA_SYS_GetBufferSize( CAMERA_SYSTEM_WORK* work , const int bufferNum );
void CAMERA_SYS_SetReadBuffer( CAMERA_SYSTEM_WORK* work , void *topAdr , const int bufferNum );

void CAMERA_SYS_DeleteReadBuffer( CAMERA_SYSTEM_WORK* work );
void CAMERA_SYS_StartCapture( CAMERA_SYSTEM_WORK* work );
void CAMERA_SYS_StopCapture( CAMERA_SYSTEM_WORK* work );
const BOOL CAMERA_SYS_IsCapture( CAMERA_SYSTEM_WORK* work );

//キャプチャ完了時に呼ばれます。
//カメラが呼ばれ数フレームは露出設定ができていなく、画像が汚いので
//呼ばれません。現在4フレーム(屋内で14、屋外で30が理想らしい)
void CAMERA_SYS_SetCaptureCallBack( CAMERA_SYSTEM_WORK* work , 
                                    CAMERA_SYS_CaptureCallBack callBack , 
                                    void* userWork );

//トリミング
//Y座標は偶数の必要があります。
//キャプチャ中は設定できません
void CAMERA_SYS_SetTrimming( CAMERA_SYSTEM_WORK* work , const u16 left , const u16 top , const u16 right , const u16 bottom );
const BOOL CAMERA_SYS_GetIsTrimming( CAMERA_SYSTEM_WORK* work );
void CAMERA_SYS_ResetTrimming( CAMERA_SYSTEM_WORK* work );
//実際にキャプチャされているサイズ
const u16 CAMERA_SYS_CaptureSizeToWidth( CAMERA_SYSTEM_WORK* work );
const u16 CAMERA_SYS_CaptureSizeToHeight( CAMERA_SYSTEM_WORK* work );

//カメラの解像度サイズ
void CAMERA_SYS_SetResolution( CAMERA_SYSTEM_WORK* work , const CAMERASize size );
const CAMERASize CAMERA_SYS_GetResolution( CAMERA_SYSTEM_WORK* work );
const u16 CAMERA_SYS_ResolutionSizeToWidth( CAMERA_SYSTEM_WORK* work );
const u16 CAMERA_SYS_ResolutionSizeToHeight( CAMERA_SYSTEM_WORK* work );

//カメラの内外の切り替え
//CAMERA_SELECT_IN
//CAMERA_SELECT_OUT
const CAMERASelect CAMERA_SYS_GetCameraPos( CAMERA_SYSTEM_WORK* work );
void CAMERA_SYS_SetCameraPos( CAMERA_SYSTEM_WORK* work , const CAMERASelect pos );
void CAMERA_SYS_SwapCameraPos( CAMERA_SYSTEM_WORK* work );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------



