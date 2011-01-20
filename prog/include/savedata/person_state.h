//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		person_state.h
 *	@brief  重要人物状態
 *	@author	Toru=Nagihashi
 *	@date		2011.01.19
 *
 *	@note 主にローカルライブキャスターで呼び出したときにどんな状態なのかを
 *	      取得するたのワークです。スクリプト側から設定し、プログラム側で取得します。
 *	      状態は、ローカルライブキャスターのエクセルデータからコンバートされます。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//セーブデータ関係
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"

//定義
#include "..\..\src\app\local_tvt\local_tvt_call.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブワーク
//=====================================
typedef struct _PERSON_STATE PERSON_STATE;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern u32 PERSON_STATE_GetWorkSize( void );
extern void PERSON_STATE_Init( void *p_wk_adrs );
extern PERSON_STATE* PERSON_STATE_GetWork( SAVE_CONTROL_WORK *p_sv );
extern const PERSON_STATE* PERSON_STATE_GetWorkConst( const SAVE_CONTROL_WORK *cp_sv );

extern void PERSON_STATE_SetState( PERSON_STATE* p_wk, u16 person, u16 state );
extern u16  PERSON_STATE_GetState( const PERSON_STATE* cp_wk, u16 person );
