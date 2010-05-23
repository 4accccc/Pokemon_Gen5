//============================================================================
/**
 *  @file   zukan_detail_touchbar.h
 *  @brief  図鑑詳細画面共通のタッチバー
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *
 *  モジュール名：ZUKAN_DETAIL_TOUCHBAR
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "zukan_detail_def.h"

// オーバーレイ
FS_EXTERN_OVERLAY(zukan_detail);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR,
  ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR_TO_APPEAR,
  ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR,
  ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR_TO_DISAPPEAR,
}
ZUKAN_DETAIL_TOUCHBAR_STATE;

typedef enum
{
  ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
  ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP,
  ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM,
  ZUKAN_DETAIL_TOUCHBAR_TYPE_MAX,
}
ZUKAN_DETAIL_TOUCHBAR_TYPE;

typedef enum
{
  ZUKAN_DETAIL_TOUCHBAR_DISP_INFO,
  ZUKAN_DETAIL_TOUCHBAR_DISP_MAP,
  ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE,
  ZUKAN_DETAIL_TOUCHBAR_DISP_FORM,

  ZUKAN_DETAIL_TOUCHBAR_DISP_CURR_ICON_NONE,  // 今アイコンなし
}
ZUKAN_DETAIL_TOUCHBAR_DISP;

typedef enum
{
  ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE,  // 外のページから(への)移動
  ZUKAN_DETAIL_TOUCHBAR_SPEED_INSIDE,   // 内部でのページ移動
  ZUKAN_DETAIL_TOUCHBAR_SPEED_MAX,
}
ZUKAN_DETAIL_TOUCHBAR_SPEED;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _ZUKAN_DETAIL_TOUCHBAR_WORK ZUKAN_DETAIL_TOUCHBAR_WORK;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
extern ZUKAN_DETAIL_TOUCHBAR_WORK* ZUKAN_DETAIL_TOUCHBAR_Init( HEAPID heap_id, BOOL form_version );
extern void ZUKAN_DETAIL_TOUCHBAR_Exit( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
extern void ZUKAN_DETAIL_TOUCHBAR_Main( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

extern void ZUKAN_DETAIL_TOUCHBAR_SetType(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   ZUKAN_DETAIL_TOUCHBAR_TYPE  type, 
                   ZUKAN_DETAIL_TOUCHBAR_DISP  disp,
                   BOOL                        is_cur_u_d );
extern ZUKAN_DETAIL_TOUCHBAR_STATE ZUKAN_DETAIL_TOUCHBAR_GetState( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
extern void ZUKAN_DETAIL_TOUCHBAR_Appear( ZUKAN_DETAIL_TOUCHBAR_WORK* work, ZUKAN_DETAIL_TOUCHBAR_SPEED speed );
extern void ZUKAN_DETAIL_TOUCHBAR_Disappear( ZUKAN_DETAIL_TOUCHBAR_WORK* work, ZUKAN_DETAIL_TOUCHBAR_SPEED speed );

extern ZKNDTL_COMMAND ZUKAN_DETAIL_TOUCHBAR_GetTrg( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
extern ZKNDTL_COMMAND ZUKAN_DETAIL_TOUCHBAR_GetTouch( ZUKAN_DETAIL_TOUCHBAR_WORK* work );
extern void ZUKAN_DETAIL_TOUCHBAR_Unlock( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

extern void ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_visible );

extern void ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   ZUKAN_DETAIL_TOUCHBAR_DISP  disp );

extern void ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_visible );

extern void ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work,
                   BOOL                        is_flip );
extern BOOL ZUKAN_DETAIL_TOUCHBAR_GetCheckFlipOfGeneral(
                   ZUKAN_DETAIL_TOUCHBAR_WORK* work );

// 見た目をアクティブ状態にしたまま、ユーザ指定の全体専用のアクティブフラグの切り替えを行う
extern void ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( ZUKAN_DETAIL_TOUCHBAR_WORK* work, BOOL is_active );  // TRUEのときアクティブ  // 切り替わったときのデフォルトはTRUE

// カスタムボタンのパレットを読み込んだ場所を得る
extern u32 ZUKAN_DETAIL_TOUCHBAR_GetCustomIconPlttRegIdx( ZUKAN_DETAIL_TOUCHBAR_WORK* work );

// BGプライオリティを設定する(OBJのBGプライオリティにもこの値が設定される)
extern void ZUKAN_DETAIL_TOUCHBAR_SetBgPriority( ZUKAN_DETAIL_TOUCHBAR_WORK* work, u8 pri );

