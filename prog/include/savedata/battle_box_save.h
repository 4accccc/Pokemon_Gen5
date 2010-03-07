//============================================================================================
/**
 * @file	battle_box_save.h
 * @brief	バトルボックスセーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/09/25
 */
//============================================================================================

#pragma once

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"


//======================================================================
//	define
//======================================================================
#define BATTLE_BOX_PARTY_NUM (1)
#define BATTLE_BOX_PARTY_MEMBER (6)

//ボックス名。一応用意して通常のボックスにサイズは合わせてある
#define BATTLE_BOX_NAME_MAXLEN  (8)
#define BATTLE_BOX_NAME_BUFSIZE (20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。

//======================================================================
//	enum
//======================================================================
typedef enum
{ 

  BATTLE_BOX_LOCK_BIT_WIFI = 1<<0,  //WIFI大会用ロック
  BATTLE_BOX_LOCK_BIT_LIVE = 1<<1,  //LIVE大会用ロック

  BATTLE_BOX_LOCK_BIT_NONE = 0,      //ロックされていない
  BATTLE_BOX_LOCK_BIT_BOTH = BATTLE_BOX_LOCK_BIT_WIFI|BATTLE_BOX_LOCK_BIT_LIVE//両方LOCK
}BATTLE_BOX_LOCK_BIT;

//======================================================================
//	typedef struct
//======================================================================

typedef struct _BATTLE_BOX_SAVE BATTLE_BOX_SAVE;


//======================================================================
//	proto
//======================================================================

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int BATTLE_BOX_SAVE_GetWorkSize(void);
extern void BATTLE_BOX_SAVE_InitWork(BATTLE_BOX_SAVE * musSave);

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
extern BATTLE_BOX_SAVE* BATTLE_BOX_SAVE_GetBattleBoxSave( SAVE_CONTROL_WORK *sv );

extern void BATTLE_BOX_SAVE_SetPPP( BATTLE_BOX_SAVE * btlBoxSave, const u32 boxIdx, const u32 idx, const POKEMON_PASO_PARAM * ppp );

//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスのセーブデータからPOEPARTYを作成
 *
 * @param		sv				バトルボックスのセーブデータ
 * @param		heapID		ヒープＩＤ
 *
 * @return	作成したデータ
 */
//--------------------------------------------------------------------------------------------
extern POKEPARTY * BATTLE_BOX_SAVE_MakePokeParty( BATTLE_BOX_SAVE * sv, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		POEPARTYをバトルボックスのセーブデータに設定
 *
 * @param		sv				バトルボックスのセーブデータ
 * @param		party			POKEPARTY
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BATTLE_BOX_SAVE_SetPokeParty( BATTLE_BOX_SAVE * sv, POKEPARTY * party );

//----------------------------------------------------------
//  PPP取得
//----------------------------------------------------------
extern POKEMON_PASO_PARAM* BATTLE_BOX_SAVE_GetPPP( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx , const u32 idx );
//----------------------------------------------------------
//  ボックス名取得
//----------------------------------------------------------
extern STRCODE* BATTLE_BOX_SAVE_GetBoxName( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx );

//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスにポケモンが入っているかどうか
 * @param		sv				バトルボックスのセーブデータ
 * @return	いたらTRUE
 */
//--------------------------------------------------------------------------------------------
extern BOOL BATTLE_BOX_SAVE_IsIn( BATTLE_BOX_SAVE * sv );


//----------------------------------------------------------
//  
//----------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスをロックする
 * @param		btlBoxSave				バトルボックスのセーブデータ
 * @param  	flg       ロックするフラグ
 */
//--------------------------------------------------------------------------------------------
extern void BATTLE_BOX_SAVE_OnLockFlg( BATTLE_BOX_SAVE *btlBoxSave,BATTLE_BOX_LOCK_BIT flg );
extern void BATTLE_BOX_SAVE_OffLockFlg( BATTLE_BOX_SAVE *btlBoxSave,BATTLE_BOX_LOCK_BIT flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		バトルボックスがロックされているかどうか
 * @param		btlBoxSave				バトルボックスのセーブデータ
 * @return	FALSEならばロックされていない
 */
//--------------------------------------------------------------------------------------------
extern BOOL BATTLE_BOX_SAVE_GetLockFlg( BATTLE_BOX_SAVE *btlBoxSave );
extern BOOL BATTLE_BOX_SAVE_GetLockType( const BATTLE_BOX_SAVE *btlBoxSave, BATTLE_BOX_LOCK_BIT flg );
