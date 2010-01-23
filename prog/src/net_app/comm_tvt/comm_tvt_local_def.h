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
#include "print/printsys.h"

#include "draw_system.h"
#include "net_app/comm_tvt_sys.h"
#include "app/app_taskmenu.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CTVT_USE_DEBUG (1)

#if CTVT_USE_DEBUG
#define CTVT_TPrintf(...) (void)((OS_TFPrintf(1,__VA_ARGS__)))
#define CTVT_Printf(...)  (void)((OS_FPrintf(1,__VA_ARGS__)))
#else
#define CTVT_TPrintf(...) ((void)0)
#define CTVT_Printf(...)  ((void)0)
#endif //DEB_ARI


#define CTVT_FRAME_MAIN_MSG     (GFL_BG_FRAME0_M)
#define CTVT_FRAME_MAIN_BG      (GFL_BG_FRAME1_M)
#define CTVT_FRAME_MAIN_DRAW    (GFL_BG_FRAME2_M)
#define CTVT_FRAME_MAIN_CAMERA  (GFL_BG_FRAME3_M)

#define CTVT_FRAME_SUB_MSG   (GFL_BG_FRAME0_S)
#define CTVT_FRAME_SUB_BAR   (GFL_BG_FRAME1_S)
#define CTVT_FRAME_SUB_MISC  (GFL_BG_FRAME2_S)
#define CTVT_FRAME_SUB_BG    (GFL_BG_FRAME3_S)

#define CTVT_PAL_BG_MAIN_BAR   (0) //1本
#define CTVT_PAL_BG_MAIN_BG    (1) //1本
#define CTVT_PAL_BG_MAIN_FONT   (10) //1本

#define CTVT_PAL_BG_SUB_BG    (0) //5本
#define CTVT_PAL_BG_BUTTON_NONE (1)
#define CTVT_PAL_BG_BUTTON_ACTIVE (3)
#define CTVT_PAL_BG_BUTTON_DISABLE (4)

#define CTVT_PAL_BG_SUB_BAR   (5) //1本
#define CTVT_PAL_BG_SUB_TASKMENU   (6) //2本
#define CTVT_PAL_BG_SUB_WINFRAME   (8) //1本
#define CTVT_PAL_BG_SUB_FONT   (10) //1本

#define CTVT_PAL_OBJ_MAIN_COMMON (0) //5本

#define CTVT_PAL_OBJ_SUB_COMMON  (0) //5本
#define CTVT_PAL_OBJ_SUB_BARICON  (5) //3本

#define CTVT_BUTTON_TALK (PAD_BUTTON_A|PAD_BUTTON_R)
#define CTVT_BUTTON_DRAW (PAD_BUTTON_START)

#define CTVT_BUTTON_PAUSE (PAD_BUTTON_SELECT)
#define CTVT_BUTTON_DRAW_EDIT (PAD_BUTTON_X)

#define CTVT_MEMBER_NUM (4)
#define CTVT_DRAW_BUFFER_NUM (100)

#define CTVT_FONT_COLOR_WHITE (PRINTSYS_MACRO_LSB(0xf,2,0))
#define CTVT_FONT_COLOR_BLACK (PRINTSYS_MACRO_LSB(1,2,0))
#define CTVT_FONT_COLOR_NAME   (PRINTSYS_MACRO_LSB(1,0xf,0))

#define CTVT_BMPWIN_CGX (32*16)

//スクリーンのバッファサイズ
#define CTVT_BUFFER_SCR_SIZE (256*192*2)  //0x18000

//音声再生速度の上限・下限
#define CTVT_TALK_SLIDER_MOVE_Y (40)  //上にも下にも40

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//上画面のモード(1枚2分割4分割
typedef enum
{
  CTDM_SINGLE,
  CTDM_DOUBLE,
  CTDM_FOUR,

  CTDM_MAX, //サイズとかの配列に使ってるので注意
}COMM_TVT_DISP_MODE;

//操作のモード
typedef enum
{
  CTM_NONE, 
  CTM_TALK, //会話
  CTM_CALL, //呼び出し
  CTM_DRAW, //落書き

  CTM_END,
  CTM_ERROR,

}COMM_TVT_MODE;

//セルリソース
typedef enum
{
  CTOR_COMMON_M_PLT,
  CTOR_COMMON_S_PLT,
  CTOR_BAR_BUTTON_S_PLT,
  CTOR_BAR_BUTTON_M_PLT,

  CTOR_COMMON_M_NCG,
  CTOR_COMMON_S_NCG,
  CTOR_BAR_BUTTON_S_NCG,
  CTOR_BAR_BUTTON_M_NCG,

  CTOR_COMMON_M_ANM,
  CTOR_COMMON_S_ANM,
  CTOR_BAR_BUTTON_ANM,  //上下共通

  CTOR_MAX,
}COMM_TVT_OBJ_RES;
#define CTOR_PLT_TOP (CTOR_COMMON_M_PLT)
#define CTOR_NCG_TOP (CTOR_COMMON_M_NCG)
#define CTOR_ANM_TOP (CTOR_COMMON_M_ANM)

//セルアニメインデックス
typedef enum
{
  CTOAS_PAUSE,
  CTOAS_PAUSE_ACTIVE,
  CTOAS_PLAY,
  CTOAS_PLAY_ACTIVE,
  CTOAS_SLIDER,
  CTOAS_SLIDER_ACTIVE,
  CTOAS_YOBIDASHI,
  CTOAS_YOBIDASHI_ACTIVE,
  CTOAS_YOBIDASHI_BAR,
  CTOAS_CHECK,
  CTOAS_CHECK_SELECT,
  CTOAS_CHECK_NONE,
  CTOAS_SCROLL_BAR,
  CTOAS_SCROLL_BAR_ACTIVE,
  CTOAS_PEN,
  CTOAS_SUPOITO,
  CTOAS_KESHIGOMU,
  CTOAS_CAMERA_ON,
  CTOAS_CAMERA_OFF,
}COMM_TVT_OBJ_ANM_SUB;

typedef enum
{
  CTOAM_TALK,
  CTOAM_PEN,
  CTOAM_PEN_ACTIVE,
  CTOAM_SUPOITO,
  CTOAM_SUPOITO_ACTIVE,
  CTOAM_KESHIGOMU,
  CTOAM_KESHIGOMU_ACTIVE,
  CTOAM_PEN_SELECT,
  CTOAM_PAUSE,
  CTOAM_PAUSE_ACTIVE,
  CTOAM_PLAY,
  CTOAM_PLAY_ACTIVE,
}COMM_TVT_OBJ_ANM_MAIN;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _COMM_TVT_WORK COMM_TVT_WORK;
typedef struct _CTVT_CAMERA_WORK CTVT_CAMERA_WORK;
typedef struct _CTVT_COMM_WORK CTVT_COMM_WORK;
typedef struct _CTVT_MIC_WORK CTVT_MIC_WORK;

typedef struct _CTVT_TALK_WORK CTVT_TALK_WORK;
typedef struct _CTVT_DRAW_WORK CTVT_DRAW_WORK;
typedef struct _CTVT_CALL_WORK CTVT_CALL_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//システム取得系
extern CTVT_CAMERA_WORK* COMM_TVT_GetCameraWork( COMM_TVT_WORK *work );
extern CTVT_COMM_WORK* COMM_TVT_GetCommWork( COMM_TVT_WORK *work );
extern CTVT_TALK_WORK* COMM_TVT_GetTalkWork( COMM_TVT_WORK *work );
extern CTVT_MIC_WORK* COMM_TVT_GetMicWork( COMM_TVT_WORK *work );
extern DRAW_SYS_WORK* COMM_TVT_GetDrawSys( COMM_TVT_WORK *work );

extern const COMM_TVT_INIT_WORK* COMM_TVT_GetInitWork( const COMM_TVT_WORK *work );

//数値取得系
extern const HEAPID COMM_TVT_GetHeapId( const COMM_TVT_WORK *work );
extern ARCHANDLE* COMM_TVT_GetArcHandle( const COMM_TVT_WORK *work );
extern const u32 COMM_TVT_GetObjResIdx( const COMM_TVT_WORK *work , const COMM_TVT_OBJ_RES resIdx );
extern GFL_CLUNIT* COMM_TVT_GetCellUnit( const COMM_TVT_WORK *work );
extern const BOOL COMM_TVT_GetUpperFade( const COMM_TVT_WORK *work );
extern void COMM_TVT_SetUpperFade( COMM_TVT_WORK *work , const BOOL flg );
extern APP_TASKMENU_RES* COMM_TVT_GetTaskMenuRes( COMM_TVT_WORK *work );

extern GFL_FONT* COMM_TVT_GetFontHandle( COMM_TVT_WORK *work );
extern GFL_MSGDATA* COMM_TVT_GetMegHandle( COMM_TVT_WORK *work );
extern PRINT_QUE* COMM_TVT_GetPrintQue( COMM_TVT_WORK *work );


extern const u8   COMM_TVT_GetConnectNum( const COMM_TVT_WORK *work );
extern void   COMM_TVT_SetConnectNum( COMM_TVT_WORK *work , const u8 num);
extern const COMM_TVT_DISP_MODE COMM_TVT_GetDispMode( const COMM_TVT_WORK *work );
//Doubleモードの切り替えはTrueがついているほうが実際のフラグ変更
//無い方がフラグ送信リクエスト
extern const BOOL COMM_TVT_IsDoubleMode( const COMM_TVT_WORK *work );
extern void COMM_TVT_SetDoubleMode( COMM_TVT_WORK *work , const BOOL flg );
extern void   COMM_TVT_FlipDoubleMode( COMM_TVT_WORK *work );
extern void   COMM_TVT_SetDoubleMode_Flag( COMM_TVT_WORK *work , const BOOL flg );
extern const u8   COMM_TVT_GetSelfIdx( const COMM_TVT_WORK *work );
extern void COMM_TVT_SetSelfIdx( COMM_TVT_WORK *work , const u8 idx );
extern const BOOL COMM_TVT_GetSusspend( COMM_TVT_WORK *work );
extern void COMM_TVT_SetSusspend( COMM_TVT_WORK *work , const BOOL flg );
extern const BOOL COMM_TVT_GetPause( COMM_TVT_WORK *work );
extern void COMM_TVT_SetPause( COMM_TVT_WORK *work , const BOOL flg );
extern void COMM_TVT_FlipPause( COMM_TVT_WORK *work );
extern const BOOL COMM_TVT_GetSusspendDraw( COMM_TVT_WORK *work );
extern void COMM_TVT_SetSusspendDraw( COMM_TVT_WORK *work , const BOOL flg );
extern const BOOL COMM_TVT_GetFinishReq( COMM_TVT_WORK *work );
extern void COMM_TVT_SetFinishReq( COMM_TVT_WORK *work , const BOOL flg );


//UTIL
extern const BOOL COMM_TVT_IsTwlMode( void );
extern const BOOL COMM_TVT_CanUseCamera( void );
extern APP_TASKMENU_WORK* COMM_TVT_OpenYesNoMenu( COMM_TVT_WORK *work );
extern void COMM_TVT_RedrawName( COMM_TVT_WORK *work );

