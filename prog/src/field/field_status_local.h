//============================================================================================
/**
 * @file	field_status_local.h
 * @brief フィールドマップ管理情報
 * @author  tamada GAMEFREAK inc.
 * @date    2009.10.03
 *
 * @note
 * field_status.hとfield_status_local.hとは対になっている。
 * field_status.hは、FIELD_STATUSというオブジェクトをメンバに持ったり、
 * 取得したりするためだけのヘッダ。（現状gamedata.hのみを想定）
 * field_status_local.hは、FIELD_STATUSの操作関数が含まれるヘッダ。
 * src/field以下のソースファイルのみでインクルードされることを想定している。
 */
//============================================================================================
#pragma once

#include "field/field_status.h"

//======================================================================
//======================================================================
//------------------------------------------------------------------
/**
 * @briefマップモード
 */
//------------------------------------------------------------------
typedef enum{
  MAPMODE_NORMAL,     ///<通常状態
  MAPMODE_INTRUDE,    ///<侵入中

  MAPMODE_MAX,    ///<最大値　
}MAPMODE;

//------------------------------------------------------------------
/**
 * @briefPROCアクション
 */
//------------------------------------------------------------------
typedef enum{
  PROC_ACTION_FIELD,    ///<フィールド中
  PROC_ACTION_BATTLE,   ///<戦闘中
}PROC_ACTION;

//======================================================================
//======================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern MAPMODE FIELD_STATUS_GetMapMode(const FIELD_STATUS * fldstatus);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern void FIELD_STATUS_SetMapMode(FIELD_STATUS * fldstatus, MAPMODE map_mode);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_STATUS_SetFieldInitFlag( FIELD_STATUS * fldstatus, BOOL flag );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern BOOL FIELD_STATUS_GetFieldInitFlag( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
/**
 * @brief コンティニューかどうかのフラグ設定
 */
//------------------------------------------------------------------
void FIELD_STATUS_SetContinueFlag( FIELD_STATUS * fldstatus, BOOL flag );

//------------------------------------------------------------------
/**
 * @brief コンティニューかどうかのフラグ取得
 */
//------------------------------------------------------------------
BOOL FIELD_STATUS_GetContinueFlag( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
/**
 * @brief 季節表示フラグの設定
 */
//------------------------------------------------------------------
extern void FIELD_STATUS_SetSeasonDispFlag( FIELD_STATUS * fldstatus, BOOL flag );

//------------------------------------------------------------------
/**
 * @brief 季節表示フラグの取得
 */
//------------------------------------------------------------------
extern BOOL FIELD_STATUS_GetSeasonDispFlag( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
/**
 * @brief 最後に表示した季節の設定
 */
//------------------------------------------------------------------
void FIELD_STATUS_SetSeasonDispLast( FIELD_STATUS * fldstatus, u8 season );

//------------------------------------------------------------------
/**
 * @brief 最後に表示した季節の取得
 */
//------------------------------------------------------------------
u8 FIELD_STATUS_GetSeasonDispLast( const FIELD_STATUS * fldstatus );


//------------------------------------------------------------------
/// 予約スクリプト制御関連
//------------------------------------------------------------------
extern void FIELD_STATUS_SetReserveScript( FIELD_STATUS * fldstatus, u16 scr_id );
extern u16 FIELD_STATUS_GetReserveScript( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
/// PROCアクション
//------------------------------------------------------------------
extern PROC_ACTION FIELD_STATUS_SetProcAction( FIELD_STATUS * fldstatus, PROC_ACTION action);
extern PROC_ACTION FIELD_STATUS_GetProcAction( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
// フィールド技関連
//------------------------------------------------------------------
//------------------------------------------------------------------
// フィールド技　フラッシュのON・OFF
//------------------------------------------------------------------
extern void FIELD_STATUS_SetFieldSkillFlash( FIELD_STATUS * fldstatus, BOOL flag );
extern BOOL FIELD_STATUS_IsFieldSkillFlash( const FIELD_STATUS * fldstatus );

//------------------------------------------------------------------
// フィールド技　マップ効果状態
// field/fieldskill_mapeff.h
//  FIELDSKILL_MAPEFF_MSK
//------------------------------------------------------------------
extern void FIELD_STATUS_SetFieldSkillMapEffectMsk( FIELD_STATUS * fldstatus, u32 msk );
extern u32 FIELD_STATUS_GetFieldSkillMapEffectMsk( const FIELD_STATUS * fldstatus );


