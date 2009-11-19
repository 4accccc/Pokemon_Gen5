//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldskill_mapeff.h
 *	@brief  フィールド技　ゾーンエフェクト
 *	@author	tomoya takahashi
 *	@date		2009.11.19
 *
 *	モジュール名：FIELDSKILL_MAPEFF
 *
 *
 *	フラッシュ　や　きりばらい　など画面全体を覆う　フィールド技効果を管理
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field_flash.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フィールドスキル　効果設定マスク
//=====================================
typedef enum 
{
  FIELDSKILL_MAPEFF_MSK_FLASH_NEAR   = 1 << 0,    // フラッシュ効果　NEAR状態
  FIELDSKILL_MAPEFF_MSK_FLASH_FAR    = 1 << 1,    // フラッシュ効果　FAR状態
  
} FIELDSKILL_MAPEFF_MSK;


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フィールドスキル　ゾーンごとの効果
//=====================================
typedef struct _FIELDSKILL_MAPEFF FIELDSKILL_MAPEFF;


//-------------------------------------
///	フィールドスキル　ゾーンごとの効果データ
//=====================================
typedef struct 
{
  u16 zone_id;
  u16 msk;    // FIELDSKILL_MAPEFF_MSK
} FIELDSKILL_MAPEFF_DATA;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成・破棄
extern FIELDSKILL_MAPEFF* FIELDSKILL_MAPEFF_Create( FIELDSKILL_MAPEFF_MSK msk, HEAPID heapID );
extern void FIELDSKILL_MAPEFF_Delete( FIELDSKILL_MAPEFF* p_wk );

// メイン
extern void FIELDSKILL_MAPEFF_Main( FIELDSKILL_MAPEFF* p_wk );
extern void FIELDSKILL_MAPEFF_Draw( FIELDSKILL_MAPEFF* p_wk );


// それぞれのワーク取得
extern BOOL FIELDSKILL_MAPEFF_IsFlash( const FIELDSKILL_MAPEFF* cp_wk );
extern FIELD_FLASH* FIELDSKILL_MAPEFF_GetFlash( const FIELDSKILL_MAPEFF* cp_wk );

#ifdef _cplusplus
}	// extern "C"{
#endif



