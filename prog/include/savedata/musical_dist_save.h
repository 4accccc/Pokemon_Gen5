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
extern void MUSICAL_DIST_SAVE_UnloadData( SAVE_CONTROL_WORK *sv );

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
//データの設定
extern void MUSICAL_DIST_SAVE_SetMusicalArchive( MUSICAL_DIST_SAVE *distSave , void *arcData , const u32 size );
//データの取得
extern void* MUSICAL_DIST_SAVE_GetProgramData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize );
extern void* MUSICAL_DIST_SAVE_GetMessageData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize );
extern void* MUSICAL_DIST_SAVE_GetScriptData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize );
extern void* MUSICAL_DIST_SAVE_GetStreamingData( MUSICAL_DIST_SAVE *distSave , const HEAPID heapId , u32 *dataSize );
