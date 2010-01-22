//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_flagcontrol.h
 *	@brief  フィールド　フラグ管理
 *	@author	tomoya takahashi
 *	@date		2009.11.19
 *
 *	モジュール名：FIELD_FLAGCONT
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field/fieldmap.h"
#include "gamesystem/game_data.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	各種フラグの初期化タイミング
//=====================================
extern void FIELD_FLAGCONT_INIT_WalkStepOver(GAMEDATA * gdata, FIELDMAP_WORK* fieldWork);   // fieldmap.c
extern void FIELD_FLAGCONT_INIT_MapJump(GAMEDATA * gdata, u16 zone_id);        // event_mapchange.c
extern void FIELD_FLAGCONT_INIT_FlySky(GAMEDATA * gdata, u16 zone_id);         // 未実装
extern void FIELD_FLAGCONT_INIT_Teleport(GAMEDATA * gdata, u16 zone_id);       // 未実装
extern void FIELD_FLAGCONT_INIT_Escape(GAMEDATA * gdata, u16 zone_id);         // 未実装
extern void FIELD_FLAGCONT_INIT_GameOver(GAMEDATA * gdata, u16 zone_id);       // event_mapchange.c
extern void FIELD_FLAGCONT_INIT_Continue(GAMEDATA * gdata, u16 zone_id);       // event_mapchange.c
extern void FIELD_FLAGCONT_INIT_FieldIn(GAMEDATA * gdata, u16 zone_id);


#ifdef _cplusplus
}	// extern "C"{
#endif



