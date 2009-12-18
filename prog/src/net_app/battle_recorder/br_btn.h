//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn.h
 *	@brief	バトルレコーダー	ボタン管理
 *	@author	Toru=Nagihashi
 *	@data		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "system/main.h"  //HEAPID
#include "br_menu_proc.h"	//MENUID
#include "br_res.h"
#include "br_inner.h"
#include "system/bmp_oam.h"
#include "br_btn_data.h"
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					ボタン管理システム（MENU_PROCでしか使わないはず）
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン状態
//=====================================
typedef enum
{
	BR_BTN_SYS_STATE_WAIT,	//タッチ待ち
	BR_BTN_SYS_STATE_HANGER_MOVE,	//移動中
	BR_BTN_SYS_STATE_BTN_CHANGE,	//ボタン切り替え中
	BR_BTN_SYS_STATE_APPEAR_MOVE,
	BR_BTN_SYS_STATE_INPUT,
} BR_BTN_SYS_STATE;

//-------------------------------------
///	ボタン入力
//=====================================
typedef enum
{
	BR_BTN_SYS_INPUT_NONE,						//押していない（or押した後の動作中）
	BR_BTN_SYS_INPUT_CHANGESEQ,				//別シーケンスに飛ぶ
	BR_BTN_SYS_INPUT_EXIT,			//終了
} BR_BTN_SYS_INPUT;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
typedef struct _BR_BTN_SYS_WORK BR_BTN_SYS_WORK;

//=============================================================================
/**
 *					PUBILIC関数
*/
//=============================================================================
extern BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MENUID menuID, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, const BR_BTN_DATA_SETUP *cp_setup, HEAPID heapID );
extern void BR_BTN_SYS_Exit( BR_BTN_SYS_WORK *p_wk );
extern void BR_BTN_SYS_Main( BR_BTN_SYS_WORK *p_wk );
extern BR_BTN_SYS_INPUT BR_BTN_SYS_GetInput( const BR_BTN_SYS_WORK *cp_wk, u32 *p_seq, u32 *p_param );
extern BR_BTN_SYS_STATE BR_BTN_SYS_GetState( const BR_BTN_SYS_WORK *cp_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					ボタン単体作成関数(各プロックで使用する)
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
typedef struct _BR_BTN_WORK BR_BTN_WORK;

extern BR_BTN_WORK * BR_BTN_Init( const GFL_CLWK_DATA *cp_cldata, u16 msgID, u16 w, CLSYS_DRAW_TYPE display, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, GFL_FONT *p_font, GFL_MSGDATA *p_msg, const BR_RES_OBJ_DATA *cp_res, HEAPID heapID );
extern BR_BTN_WORK * BR_BTN_InitEx( const GFL_CLWK_DATA *cp_cldata, const STRBUF *cp_strbuf, u16 w, CLSYS_DRAW_TYPE display, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, GFL_FONT *p_font, const BR_RES_OBJ_DATA *cp_res, HEAPID heapID );
extern void BR_BTN_Exit( BR_BTN_WORK *p_wk );
extern BOOL BR_BTN_GetTrg( const BR_BTN_WORK *cp_wk, u32 x, u32 y );
extern void BR_BTN_SetPos( BR_BTN_WORK *p_wk, s16 x, s16 y );
extern void BR_BTN_GetPos( const BR_BTN_WORK *cp_wk, s16 *p_x, s16 *p_y );
extern void BR_BTN_SetSoftPriority( BR_BTN_WORK *p_wk, u8 soft_pri );
extern u8 BR_BTN_GetSoftPriority( const BR_BTN_WORK *cp_wk );

