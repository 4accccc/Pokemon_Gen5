//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		tradepoke_after_save.h
 *	@brief  ポケモン交換　その後　セーブデータ
 *	@author	tomoya takahashi
 *	@date		2010.03.26
 *
 *	モジュール名：TRPOKE_AFTER_SAVE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK

#include "gamesystem/game_data.h"

#include "poke_tool/poke_tool.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
typedef enum{
  TRPOKE_AFTER_SAVE_C02,    // シッポウ
  TRPOKE_AFTER_SAVE_C05,    // ホドモエ

  TRPOKE_AFTER_SAVE_TYPE_MAX,
} TRPOKE_AFTER_SAVE_TYPE;

//-------------------------------------
///	レベル成長
//=====================================
#define TRPOKE_AFTER_LEVEL_DEF  (65)
#define TRPOKE_AFTER_LEVEL_ADD  (5)
#define TRPOKE_AFTER_LEVEL_MAX  (100)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	交換ポケモン　その後　セーブワーク
//=====================================
typedef struct _TRPOKE_AFTER_SAVE TRPOKE_AFTER_SAVE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int TRPOKE_AFTER_SV_GetWorkSize(void);
extern TRPOKE_AFTER_SAVE* TRPOKE_AFTER_SV_AllocWork(HEAPID heapID);
extern void TRPOKE_AFTER_SV_Init(TRPOKE_AFTER_SAVE* sv);

//----------------------------------------------------------
//	GAMEDATAのアクセス
//----------------------------------------------------------
extern TRPOKE_AFTER_SAVE* GAMEDATA_GetTrPokeAfterSaveData( GAMEDATA* gdata );

//----------------------------------------------------------
//	データアクセス
//----------------------------------------------------------
// 取得
extern BOOL TRPOKE_AFTER_SV_IsData( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type );
extern u8 TRPOKE_AFTER_SV_GetLevel( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  );
extern u32 TRPOKE_AFTER_SV_GetRnd( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  ); // 固体乱数
extern const STRCODE* TRPOKE_AFTER_SV_GetNickName( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type );
extern u32 TRPOKE_AFTER_SV_GetID( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  ); // ID
extern BOOL TRPOKE_AFTER_SV_IsSpeabino3( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  ); // 第３とくせい
extern u8 TRPOKE_AFTER_SV_GetSpeabino3( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type  ); // 第３とくせい
extern u32 TRPOKE_AFTER_SV_GetTradeMonsNo( const TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type, u32 rom_version );// トレードポケモンナンバー

//　設定
extern void TRPOKE_AFTER_SV_SetData( TRPOKE_AFTER_SAVE* sv, TRPOKE_AFTER_SAVE_TYPE type, const POKEMON_PARAM * pp );




#ifdef _cplusplus
}	// extern "C"{
#endif



