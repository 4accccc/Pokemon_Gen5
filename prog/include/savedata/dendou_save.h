//==============================================================================
/**
 * @file    dendou_save.h
 * @brief   殿堂入りセーブデータ
 * @author  matsuda
 * @date    2010.03.12(金)
 */
//==============================================================================
#pragma once

#include "poke_tool/pokeparty.h"
#include "buflen.h"



//--------------------------------------------------------------
/**
 *	関連定数
 */
//--------------------------------------------------------------
enum {
	DENDOU_RECORD_MAX = 15,		// セーブ領域に保存される殿堂入りデータの最大件数
	DENDOU_NUMBER_MAX = 9999,	// データ追加される通算回数の最大（これ以上は殿堂入りしても追加されない）
};

//--------------------------------------------------------------
/**
 *	殿堂入りセーブデータの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _DENDOU_SAVEDATA		DENDOU_SAVEDATA;


//--------------------------------------------------------------
/**
 *	殿堂入りセーブデータからポケモンデータを取得するための構造体
 *（文字列受け取り用の STRBUF は作成しておくこと！）
 */
//--------------------------------------------------------------
typedef struct {
	STRBUF*		nickname;
	STRBUF*		oyaname;

	u32			personalRandom;
	u32			idNumber;
	u16			monsno;
	u8			level;
	u8			formNumber:6;
	u8      sex:2;
	
	u16			waza[4];

}DENDOU_POKEMON_DATA;







extern u32 DendouData_GetWorkSize( void );
extern void DendouData_Init( void *work );


//------------------------------------------------------------------
/**
 * 殿堂入りデータ１件追加
 *
 * @param   data		殿堂入りセーブデータポインタ
 * @param   party		殿堂入りパーティーのデータ
 * @param   date		殿堂入り日付データ
 *
 */
//------------------------------------------------------------------
extern void DendouData_AddRecord( DENDOU_SAVEDATA* data, const POKEPARTY* party, const RTCDate* date, HEAPID heap_id );




extern u32 DendouData_GetRecordCount( const DENDOU_SAVEDATA* data );
extern u32 DendouData_GetRecordNumber( const DENDOU_SAVEDATA* data, int index );
extern u32 DendouData_GetPokemonCount( const DENDOU_SAVEDATA* data, int index );
extern void DendouData_GetPokemonData( const DENDOU_SAVEDATA* data, int index, int poke_pos, DENDOU_POKEMON_DATA* poke_data );
extern void DendouData_GetDate( const DENDOU_SAVEDATA* data, int index, RTCDate* date );

