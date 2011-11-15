//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fes_gimmick_event.h
 *  @brief  フェスミッション イベントミッションギミック
 *	@author	Masayuki Onoue
 *	@date		2011.11.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "field/fes_gimmick_trail.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//フェスミッションイベントギミック 隠し穴大捜索用ポケモンデータ
typedef struct _FES_TRAIL_DATA_SET{
  u16 monsno;    // モンスター番号
  u8  formno;     // フォルム番号
}FES_TRAIL_DATA_SET;

//フェスミッションイベントギミック 隠し穴大捜索データ
typedef struct _FES_ANIMAL_TRAIL_DATA{
  u8 trail_id;          // 置換え対象獣道ID
  u8 is_female;         // メスで置き換えるか
  u8 is_enable;         // 置換えが有効かどうか
  u8 is_through_door;   // 置換えが有効かどうか
  u16 monsno;           // モンスター番号
  u8  formno;           // フォルム番号
}FES_ANIMAL_TRAIL_DATA;

//フェスミッションイベントギミック 隠し穴大捜索用ワーク領域
typedef struct _FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK{
  FES_ANIMAL_TRAIL_DATA replace_data[FES_ATRAIL_REPLACE_NUM];
}FES_GIMMICK_EVENT_ANIMAL_TRAIL_EX_WK;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================

//=============================================================================
//  隠し穴大捜索EX用
//=============================================================================
extern const FES_ANIMAL_TRAIL_DATA* FES_GIMMICK_EVENT_GetExAnimalTrailData( FES_GIMMICK* sys_wk ,u8 trail_id );
extern void FES_GIMMICK_EVENT_RefreshExtraAnimalTrailData( FES_GIMMICK* wk ,u8 end_trail_id );
extern BOOL FES_GIMMICK_EVENT_Delete( FES_GIMMICK* sys_wk );
extern BOOL FES_GIMMICK_EVENT_IsSetExAnimalTrailData( FES_GIMMICK* sys_wk ,u8 trail_id );
extern void FES_GIMMICK_EVENT_SetExAnimalTrailDoorFlag( FES_GIMMICK* sys_wk ,u8 trail_id );
extern u8 FES_GIMMICK_EVENT_GetExAnimalTrailDataID( FES_GIMMICK* sys_wk ,u8 trail_id );

//=============================================================================
//  隠し穴大捜索EX用(NOTWIFI)
//=============================================================================
extern void FES_GIMMICK_EVENT_ExAnimalTrailMissionWkCreate( FES_GIMMICK* sys_wk );
extern BOOL FES_GIMMICK_EVENT_IsExAnimalTrailMission( FES_GIMMICK* sys_wk );

