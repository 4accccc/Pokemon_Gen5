//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc_data.h
 *	@brief  White Forest  Black City 基本データ
 *	@author	tomoya takahashi
 *	@date		2009.11.10
 *
 *	モジュール名：FIELD_WFBC_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gamesystem/gamedata_def.h"  //GAMEDATA
#include "field/intrude_common.h"

#include "savedata/mystatus.h"

#include "buflen.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	GAMEDATA　街情報インデックス
//=====================================
typedef enum {
  GAMEDATA_WFBC_ID_COMM = 0,    // パレス進入先のWFBCワーク
  GAMEDATA_WFBC_ID_MINE,        // 自分のWFBCワーク
  GAMEDATA_WFBC_ID_MAX,
} GAMEDATA_WFBC_ID;


//-------------------------------------
///	街の種類
//=====================================
typedef enum
{
  FIELD_WFBC_CORE_TYPE_BLACK_CITY,    // ブラックシティ
  FIELD_WFBC_CORE_TYPE_WHITE_FOREST,  // ホワイトフォレスト

  FIELD_WFBC_CORE_TYPE_MAX,  // 街の数
} FIELD_WFBC_CORE_TYPE;


//-------------------------------------
///	記憶する人物の数　最大数
//=====================================
#define FIELD_WFBC_PEOPLE_MAX ( 20 )
#define FIELD_WFBC_INIT_PEOPLE_NUM ( 10 ) // 初期化での人の数を変更

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	WFBCマップCOREデータ
//=====================================
// 人のCOREデータ
typedef struct 
{
  u8 data_in;   // 人の有無 TRUE FALSE
  u8 npc_id;    // 人物を特定するためのNPCID
  u8 mood;      // 機嫌
  u8 battle;    // バトルフラグ
  

  // 個々に、通信などで引き継ぐ情報を入れていく
  // １つ前の親情報
	STRCODE parent[PERSON_NAME_SIZE + EOM_SIZE];		// 16
  u32     parent_id;  // 4


} FIELD_WFBC_CORE_PEOPLE;

// 全体のCOREデータ 
typedef struct 
{
  u16 data_in;  // TRUE FALSE
  u16 type;     // FIELD_WFBC_CORE_TYPE
  FIELD_WFBC_CORE_PEOPLE people[FIELD_WFBC_PEOPLE_MAX];
  FIELD_WFBC_CORE_PEOPLE back_people[FIELD_WFBC_PEOPLE_MAX];
} FIELD_WFBC_CORE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------
//  GAMEDATAからの取得関数郡
//----------------------------------------------------------
extern FIELD_WFBC_CORE* GAMEDATA_GetMyWFBCCoreData( GAMEDATA * gamedata );
extern FIELD_WFBC_CORE* GAMEDATA_GetWFBCCoreData( GAMEDATA * gamedata, GAMEDATA_WFBC_ID id );



//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================
//ワークのクリア
extern void FIELD_WFBC_CORE_Crear( FIELD_WFBC_CORE* p_wk );
//街の情報をセットアップ
//@TODO　最終的には、対応する人の情報ももらって、情報を作成する。
extern void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk );
//整合性チェック
//@TODO　最終的には、対応する人の情報ももらって、整合性をあわせる
extern BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk );
// 人の数を返す
extern u32 FIELD_WFBC_CORE_GetPeopleNum( const FIELD_WFBC_CORE* cp_wk );


//-------------------------------------
///	FIELD_WFBC_CORE_PEOPLE用関数
//=====================================
//ワークのクリア
extern void FIELD_WFBC_CORE_PEOPLE_Crear( FIELD_WFBC_CORE_PEOPLE* p_wk );
//整合性チェック 
//@TODO　最終的には、対応する人の情報ももらって、整合性をあわせる
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsConfomity( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_PEOPLE_Management( FIELD_WFBC_CORE_PEOPLE* p_wk );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsInData( const FIELD_WFBC_CORE_PEOPLE* cp_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif


