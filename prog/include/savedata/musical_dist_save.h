//============================================================================================
/**
 * @file	musical_dist_save.h
 * @brief	ミュージカル配信用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================

#pragma once

#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
//======================================================================
//	typedef struct
//======================================================================

typedef struct _MUSICAL_DIST_SAVE MUSICAL_DIST_SAVE;


//======================================================================
//	proto
//======================================================================

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int MUSICAL_DIST_SAVE_GetWorkSize(void);
extern void MUSICAL_DIST_SAVE_InitWork(MUSICAL_DIST_SAVE * musSave);

//----------------------------------------------------------
//	データのセーブ・ロード
//----------------------------------------------------------
extern MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_LoadData( SAVE_CONTROL_WORK *sv , HEAPID heapId );
extern void MUSICAL_DIST_SAVE_UnloadData( MUSICAL_DIST_SAVE *distSave );

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
//データの設定(セーブ、Load・UnLoad込み)
extern MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_SaveMusicalArchive_Init( SAVE_CONTROL_WORK *sv , void *arcData , const u32 size , const HEAPID heapId );
extern const BOOL MUSICAL_DIST_SAVE_SaveMusicalArchive_Main( MUSICAL_DIST_SAVE *distSave );
//データの取得
extern void* MUSICAL_DIST_SAVE_GetMusicalArc( MUSICAL_DIST_SAVE *distSave );
extern const u32 MUSICAL_DIST_SAVE_GetMusicalArcSize( MUSICAL_DIST_SAVE *distSave );
