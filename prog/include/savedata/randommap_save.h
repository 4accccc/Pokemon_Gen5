//============================================================================================
/**
 * @file	randommap_save.c
 * @brief	ランダム生成マップ用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 *
 * モジュール名：RANDOMMAP_SAVE
 */
//============================================================================================

#pragma once

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため

//======================================================================
//	typedef struct
//======================================================================

typedef struct _RANDOMMAP_SAVE RANDOMMAP_SAVE;

//======================================================================
//	proto
//======================================================================

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int RANDOMMAP_SAVE_GetWorkSize(void);
extern void RANDOMMAP_SAVE_InitWork(RANDOMMAP_SAVE *randomMapSave);

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
extern RANDOMMAP_SAVE* RANDOMMAP_SAVE_GetRandomMapSave( SAVE_CONTROL_WORK *sv );

extern void RANDOMMAP_SAVE_SetCityLevel( RANDOMMAP_SAVE *randomMapSave , const u16 level );
extern const u16 RANDOMMAP_SAVE_GetCityLevel( RANDOMMAP_SAVE *randomMapSave );

