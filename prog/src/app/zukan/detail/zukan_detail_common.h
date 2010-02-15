//============================================================================
/**
 *  @file   zukan_detail_common.h
 *  @brief  図鑑詳細画面共通のデータをまとめたもの
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *
 *  モジュール名：ZKNDTL_COMMON
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "zukan_detail_graphic.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "zukan_detail_touchbar.h"
#include "zukan_detail_headbar.h"

// オーバーレイ
FS_EXTERN_OVERLAY(zukan_detail);


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _ZKNDTL_COMMON_WORK ZKNDTL_COMMON_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
extern ZKNDTL_COMMON_WORK* ZKNDTL_COMMON_Init(
                HEAPID                       heap_id,
                GAMEDATA*                    gamedata,
                ZUKAN_DETAIL_GRAPHIC_WORK*   graphic,
                GFL_FONT*                    font,
                PRINT_QUE*                   print_que,
                ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar,
                ZUKAN_DETAIL_HEADBAR_WORK*   headbar,
                u16*                         list,
                u16                          num,
                u16*                         no );
extern void ZKNDTL_COMMON_Exit( ZKNDTL_COMMON_WORK* cmn );

extern GAMEDATA*                    ZKNDTL_COMMON_GetGamedata( ZKNDTL_COMMON_WORK* cmn );
extern ZUKAN_DETAIL_GRAPHIC_WORK*   ZKNDTL_COMMON_GetGraphic( ZKNDTL_COMMON_WORK* cmn );
extern GFL_FONT*                    ZKNDTL_COMMON_GetFont( ZKNDTL_COMMON_WORK* cmn );
extern PRINT_QUE*                   ZKNDTL_COMMON_GetPrintQue( ZKNDTL_COMMON_WORK* cmn );
extern ZUKAN_DETAIL_TOUCHBAR_WORK*  ZKNDTL_COMMON_GetTouchbar( ZKNDTL_COMMON_WORK* cmn );
extern ZUKAN_DETAIL_HEADBAR_WORK*   ZKNDTL_COMMON_GetHeadbar( ZKNDTL_COMMON_WORK* cmn );

extern u16   ZKNDTL_COMMON_GetCurrPoke( ZKNDTL_COMMON_WORK* cmn );
extern void  ZKNDTL_COMMON_GoToNextPoke( ZKNDTL_COMMON_WORK* cmn );
extern void  ZKNDTL_COMMON_GoToPrevPoke( ZKNDTL_COMMON_WORK* cmn );


// 一部分フェード
//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  ZKNDTL_COMMON_FADE_DISP_M,
  ZKNDTL_COMMON_FADE_DISP_S,
  ZKNDTL_COMMON_FADE_DISP_MAX,
}
ZKNDTL_COMMON_FADE_DISP;

#define ZKNDTL_COMMON_FADE_BRIGHT_DELTA (1)
#define ZKNDTL_COMMON_FADE_BRIGHT_WAIT  (1)

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
typedef struct _ZKNDTL_COMMON_FADE_WORK ZKNDTL_COMMON_FADE_WORK;

//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
extern ZKNDTL_COMMON_FADE_WORK* ZKNDTL_COMMON_FadeInit( HEAPID heap_id );
extern void ZKNDTL_COMMON_FadeExit( ZKNDTL_COMMON_FADE_WORK* fade_wk );

extern void ZKNDTL_COMMON_FadeMain( ZKNDTL_COMMON_FADE_WORK* fade_wk_m, ZKNDTL_COMMON_FADE_WORK* fade_wk_s );

extern BOOL ZKNDTL_COMMON_FadeIsExecute( ZKNDTL_COMMON_FADE_WORK* fade_wk );
extern BOOL ZKNDTL_COMMON_FadeIsColorless( ZKNDTL_COMMON_FADE_WORK* fade_wk );
extern BOOL ZKNDTL_COMMON_FadeIsBlack( ZKNDTL_COMMON_FADE_WORK* fade_wk );

extern void ZKNDTL_COMMON_FadeSetBlackToColorless( ZKNDTL_COMMON_FADE_WORK* fade_wk );
extern void ZKNDTL_COMMON_FadeSetColorlessToBlack( ZKNDTL_COMMON_FADE_WORK* fade_wk );

extern void ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk );
extern void ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP fade_disp, ZKNDTL_COMMON_FADE_WORK* fade_wk );

extern void ZKNDTL_COMMON_FadeSetPlane( ZKNDTL_COMMON_FADE_WORK* fade_wk, int plane1, int plane2 );
extern void ZKNDTL_COMMON_FadeSetPlaneDefault( ZKNDTL_COMMON_FADE_WORK* fade_wk );


// 最背面
//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  ZKNDTL_COMMON_REAR_TYPE_INFO,
  ZKNDTL_COMMON_REAR_TYPE_MAP,
  ZKNDTL_COMMON_REAR_TYPE_VOICE,
  ZKNDTL_COMMON_REAR_TYPE_FORM,
  ZKNDTL_COMMON_REAR_TYPE_MAX,
}
ZKNDTL_COMMON_REAR_TYPE;

enum
{
  ZKNDTL_COMMON_REAR_BG_PAL_NUM     = 2,
};

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
typedef struct _ZKNDTL_COMMON_REAR_WORK ZKNDTL_COMMON_REAR_WORK;

//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
extern ZKNDTL_COMMON_REAR_WORK* ZKNDTL_COMMON_RearInit(
                                     HEAPID                    heap_id,
                                     ZKNDTL_COMMON_REAR_TYPE   type,
                                     u8                        bg_frame,
                                     u8                        pal0,
                                     u8                        pal1
                                );
extern void ZKNDTL_COMMON_RearExit( ZKNDTL_COMMON_REAR_WORK* rear_wk );
extern void ZKNDTL_COMMON_RearMain( ZKNDTL_COMMON_REAR_WORK* rear_wk );


// BGパネル
//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  ZKNDTL_COMMON_PANEL_CREATE_TYPE_TRANS_PAL_CHARA,  // パレットとキャラを転送する
  ZKNDTL_COMMON_PANEL_CREATE_TYPE_EXIST_PAL_CHARA,  // 既存のパレットとキャラを使用する
}
ZKNDTL_COMMON_PANEL_CREATE_TYPE;

//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
                                             // type          TRANS  EXIST
extern
GFL_ARCUTIL_TRANSINFO ZKNDTL_COMMON_PanelCreate(           //  o      x
             ZKNDTL_COMMON_PANEL_CREATE_TYPE    type,      //  o      o
             HEAPID                             heap_id,   //  o      o
             u8                                 bg_frame,  //  o      o
             u8                                 pal_num,   //  o      x
             u8                                 pal_pos,   //  o      o
             u8                                 pal_ofs,   //  o      x
             u32                                arc_id,    //  o      o
             ARCDATID                           data_ncl,  //  o      x
             ARCDATID                           data_ncg,  //  o      x 
             ARCDATID                           data_nsc,  //  o      o
             GFL_ARCUTIL_TRANSINFO              tinfo      //  x      o
       );

extern
void ZKNDTL_COMMON_PanelDelete(
                       u8                      bg_frame,
                       GFL_ARCUTIL_TRANSINFO   tinfo
        );

