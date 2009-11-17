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


//======================================================================
//	define
//======================================================================
#define BATTLE_BOX_PARTY_NUM (1)
#define BATTLE_BOX_PARTY_MEMBER (6)

//ボックス名。一応用意して通常のボックスにサイズは合わせてある
#define BATTLE_BOX_NAME_MAXLEN  (8)
#define BATTLE_BOX_NAME_BUFSIZE (20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。

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


//----------------------------------------------------------
//  PPP取得
//----------------------------------------------------------
POKEMON_PASO_PARAM* BATTLE_BOX_SAVE_GetPPP( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx , const u32 idx );
//----------------------------------------------------------
//  ボックス名取得
//----------------------------------------------------------
STRCODE* BATTLE_BOX_SAVE_GetBoxName( BATTLE_BOX_SAVE *btlBoxSave , const u32 boxIdx );
