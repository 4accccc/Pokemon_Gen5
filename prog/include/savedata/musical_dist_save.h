//============================================================================================
/**
 * @file	musical_dist_save.h
 * @brief	ミュージカル配信用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================

#pragma once

#include "gamesystem/game_data.h"

// DLデータに付属しているCRCのサイズ
#define MUSICAL_DIST_CRC_SIZE (2)

// ヘッダー(DIST_DATA_HEADER)のサイズ
#define MUSICAL_DIST_HEADER_SIZE (16)

// ミュージカル配信のダウンロード実サイズ
// (DL時のサイズチェックに使用、CRCは含まない値)
// ※ セーブサイズ - ヘッダーサイズ + MUSICAL_DIST_CRC_SIZE = ミュージカルデータの最大サイズ
#define MUSICAL_DIST_SAVE_DL_SIZE ((95*1024)-MUSICAL_DIST_HEADER_SIZE)

//======================================================================
// @note ミュージカルデータのについて
//
// ◆ダウンロードしてくるデータ構成
// [データ本体(MUSICAL_DIST_SAVE_DL_SIZE)][CRC(MUSICAL_DIST_CRC_SIZE)]
// CRCは含まれているが、ヘッダーは含まれていない
//
// ◆セーブ領域について(saveData)
// [ヘッダー(saveDataHeader)][データ本体 (saveDataArc)]
// ※ saveData       = MUSICAL_DIST_SAVE_SIZE
// ※ saveDataHeader = MUSICAL_DIST_HEADER_SIZE
// ※ saveDataArc    = MUSICAL_DIST_SAVE_DL_SIZE
// ヘッダーにはダウンロードデータのサイズが入っており、CRCは含まれない
// ダウンロードしてきたデータをこの形式にしてセーブする
//
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct _MUSICAL_DIST_SAVE MUSICAL_DIST_SAVE;


//======================================================================
//	proto
//======================================================================
//ミュージカル配信データはnaixを持っていないし、コンバータで順番固定で出力しているので
//ココでenum定義
enum
{
  MUSICAL_ARCDATAID_PROGDATA = 0,
  MUSICAL_ARCDATAID_GMMDATA = 1,
  MUSICAL_ARCDATAID_SCRIPTDATA = 2,
  MUSICAL_ARCDATAID_SBNKDATA = 3,
  MUSICAL_ARCDATAID_SSEQDATA = 4,
  MUSICAL_ARCDATAID_SWAVDATA = 5,
};

//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int MUSICAL_DIST_SAVE_GetWorkSize(void);
extern void MUSICAL_DIST_SAVE_InitWork(MUSICAL_DIST_SAVE * musSave);

//----------------------------------------------------------
//	データのセーブ・ロード
//----------------------------------------------------------
extern MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_LoadData( GAMEDATA *gamedata , HEAPID heapId );
extern void MUSICAL_DIST_SAVE_UnloadData( MUSICAL_DIST_SAVE *distSave );

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
//データの設定(セーブ、Load・UnLoad込み)
extern MUSICAL_DIST_SAVE* MUSICAL_DIST_SAVE_SaveMusicalArchive_Init( GAMEDATA *gamedata , void *arcData , const u32 size , const HEAPID heapId );
extern const BOOL MUSICAL_DIST_SAVE_SaveMusicalArchive_Main( MUSICAL_DIST_SAVE *distSave );
//データの取得
extern void* MUSICAL_DIST_SAVE_GetMusicalArc( MUSICAL_DIST_SAVE *distSave );
extern const u32 MUSICAL_DIST_SAVE_GetMusicalArcSize( MUSICAL_DIST_SAVE *distSave );
