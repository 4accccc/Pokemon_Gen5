//======================================================================
/**
 * @file	comm_tvt_local_def.h
 * @brief	通信TVT定義
 * @author	ariizumi
 * @data	09/12/16
 *
 * モジュール名：CTVT_
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CTVT_USE_DEBUG (0)

#if CTVT_USE_DEBUG
#define CTVT_TPrintf(...) (void)((OS_TFPrintf(1,__VA_ARGS__)))
#define CTVT_Printf(...)  (void)((OS_FPrintf(1,__VA_ARGS__)))
#else
#define CTVT_TPrintf(...) ((void)0)
#define CTVT_Printf(...)  ((void)0)
#endif //DEB_ARI


#define CTVT_FRAME_MAIN_DRAW    (GFL_BG_FRAME2_M)
#define CTVT_FRAME_MAIN_CAMERA  (GFL_BG_FRAME3_M)

#define CTVT_FRAME_SUB_BG  (GFL_BG_FRAME3_M)

#define CTVT_MEMBER_NUM (4)

//スクリーンのバッファサイズ
#define CTVT_BUFFER_SCR_SIZE (256*192*2)  //0x18000

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CTDM_SINGLE,
  CTDM_DOUBLE,
  CTDM_FOUR,

  CTDM_MAX, //サイズとかの配列に使ってるので注意
}COMM_TVT_DISP_MODE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _COMM_TVT_WORK COMM_TVT_WORK;
typedef struct _CTVT_CAMERA_WORK CTVT_CAMERA_WORK;
typedef struct _CTVT_COMM_WORK CTVT_COMM_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//システム取得系
extern CTVT_CAMERA_WORK* COMM_TVT_GetCameraWork( COMM_TVT_WORK *work );

//数値取得系
extern const HEAPID COMM_TVT_GetHeapId( const COMM_TVT_WORK *work );
extern const u8   COMM_TVT_GetConnectNum( const COMM_TVT_WORK *work );
extern void   COMM_TVT_SetConnectNum( COMM_TVT_WORK *work , const u8 num);
extern const COMM_TVT_DISP_MODE COMM_TVT_GetDispMode( const COMM_TVT_WORK *work );
extern const BOOL COMM_TVT_IsDoubleMode( const COMM_TVT_WORK *work );
extern void COMM_TVT_SetDoubleMode( COMM_TVT_WORK *work , const BOOL flg );
extern const u8   COMM_TVT_GetSelfIdx( const COMM_TVT_WORK *work );
extern void COMM_TVT_SetSelfIdx( COMM_TVT_WORK *work , const u8 idx );

//UTIL
extern const BOOL COMM_TVT_IsTwlMode( void );

