//==============================================================================
/**
 * @file    dendou_save.h
 * @brief   殿堂入りセーブデータ
 * @author  matsuda / mori
 * @date    2010.03.12(金)
 *
 * @note    今回殿堂入りデータは「殿堂入り１５件：外部」と、
 *          「初クリア時１件：内部」の２箇所に保存される。
 *          また『殿堂入り』はローテーション構造のDENDOU_SAVEDATAを使用するが、
 *          『初クリア』は１件のみで済むのでDENDOU_SAVEDATAの中のDENDOU_RECORDを
 *          表に出してアクセスできるようにしている。
 *        
 */
//==============================================================================
#pragma once

#include "poke_tool/pokeparty.h"
#include "buflen.h"



//--------------------------------------------------------------
/**
 *  関連定数
 */
//--------------------------------------------------------------
enum {
  DENDOU_RECORD_MAX = 15,   // セーブ領域に保存される殿堂入りデータの最大件数
  DENDOU_NUMBER_MAX = 9999, // データ追加される通算回数の最大（これ以上は殿堂入りしても追加されない）
};

//--------------------------------------------------------------
/**
 *  殿堂入りセーブデータの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _DENDOU_SAVEDATA DENDOU_SAVEDATA;  // 殿堂入りデータ（外部：15匹用
typedef struct DENDOU_RECORD    DENDOU_RECORD;    // 

//--------------------------------------------------------------
/**
 *  殿堂入りセーブデータからポケモンデータを取得するための構造体
 *（文字列受け取り用の STRBUF は作成しておくこと！）
 */
//--------------------------------------------------------------
typedef struct {
  STRBUF*   nickname;
  STRBUF*   oyaname;

  u32     personalRandom;
  u32     idNumber;
  u16     monsno;
  u8      level;
  u8      formNumber:6;
  u8      sex:2;
  
  u16     waza[4];

}DENDOU_POKEMON_DATA;






//------------------------------------------------------------------
/**
 * 殿堂入りデータ１件追加
 *
 * @param   data    殿堂入りセーブデータポインタ
 * @param   party   殿堂入りパーティーのデータ
 * @param   date    殿堂入り日付データ
 *
 */
//------------------------------------------------------------------
extern void DendouData_AddRecord( DENDOU_SAVEDATA* data, const POKEPARTY* party, const RTCDate* date, HEAPID heap_id );

// 殿堂入りデータ取得関連
extern u32  DendouData_GetRecordCount( const DENDOU_SAVEDATA* data );
extern u32  DendouData_GetRecordNumber( const DENDOU_SAVEDATA* data, int index );
extern u32  DendouData_GetPokemonCount( const DENDOU_SAVEDATA* data, int index );
extern void DendouData_GetPokemonData( const DENDOU_SAVEDATA* data, int index, int poke_pos, DENDOU_POKEMON_DATA* poke_data );
extern void DendouData_GetDate( const DENDOU_SAVEDATA* data, int index, RTCDate* date );

extern u32 DendouData_GetWorkSize( void );
extern void DendouData_Init( void *work );

//------------------------------------------------------------------
/**
 * ゲームクリア時セーブデータ追加(1回しか登録しません）
 *
 * @param   data    クリアセーブデータポインタ
 * @param   party   クリアパーティーのデータ
 * @param   date    クリア日付データ
 *
 */
//------------------------------------------------------------------
extern void DendouRecord_Add( DENDOU_RECORD* record, const POKEPARTY* party, const RTCDate* date, HEAPID heap_id );

// ゲームクリアデータ取得関連
extern u32  DendouRecord_GetPokemonCount( const DENDOU_RECORD* record );
extern void DendouRecord_GetPokemonData( const DENDOU_RECORD* record, int poke_pos, DENDOU_POKEMON_DATA* poke_data );
extern void DendouRecord_GetDate( const DENDOU_RECORD* record, RTCDate* date );


extern u32  GameClearData_GetWorkSize( void );
extern void GameClearData_Init( void *work );
