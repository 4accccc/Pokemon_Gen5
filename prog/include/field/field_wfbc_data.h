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
#include "field/field_wfbc_people_def.h"

#include "savedata/mystatus.h"

#include "buflen.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	1日の制限マスク
// 1～8 bitまで
//=====================================
typedef enum {
  FIELD_WFBC_ONEDAY_MSK_BATTLE = 1<<0,    ///<バトル可能
  FIELD_WFBC_ONEDAY_MSK_INTOWN = 1<<1,    ///<街に入ったときの加算可能
  FIELD_WFBC_ONEDAY_MSK_TALK   = 1<<2,    ///<会話の後の加算可能
  
} FIELD_WFBC_ONEDAY_MSK;

// 1日のフラグ初期値
#define FIELD_WFBC_ONEDAY_MSK_INIT  ( FIELD_WFBC_ONEDAY_MSK_BATTLE | FIELD_WFBC_ONEDAY_MSK_INTOWN | FIELD_WFBC_ONEDAY_MSK_TALK )

// ON
#define FIELD_WFBC_ONEDAY_MSK_ON( val, msk )  ( (val) | (msk) )
// OFF
#define FIELD_WFBC_ONEDAY_MSK_OFF( val, msk )  ( (val) & ~(msk) )
// CHECK
#define FIELD_WFBC_ONEDAY_MSK_CHECK( val, msk )  ( (val) & (msk) )

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


//-------------------------------------
/// 機嫌
//=====================================
#define FIELD_WFBC_MOOD_DEFAULT ( 50 )  // @TODO 後々は、エクセルデータから
#define FIELD_WFBC_MOOD_MAX     ( 100 ) // 最大値
#define FIELD_WFBC_MOOD_ADD_INTOWN  ( 3 ) // 街に訪れると足される値
#define FIELD_WFBC_MOOD_ADD_TALK    ( 10 )  // 話をすると1日1回足される値
#define FIELD_WFBC_MOOD_SUB         ( -5 ) // 1日1回減る値

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
  u8 one_day_msk;// 1日の制限マスク FIELD_WFBC_ONEDAY_MSK
  

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
//ワークの全クリア
extern void FIELD_WFBC_CORE_Clear( FIELD_WFBC_CORE* p_wk );
extern void FIELD_WFBC_CORE_ClearNormal( FIELD_WFBC_CORE* p_wk );
extern void FIELD_WFBC_CORE_ClearBack( FIELD_WFBC_CORE* p_wk );
//街の情報をセットアップ
extern void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus, HEAPID heapID );
//整合性チェック
extern BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk );
// 人の数を返す
extern u32 FIELD_WFBC_CORE_GetPeopleNum( const FIELD_WFBC_CORE* cp_wk );
// 機嫌値でソートする
// ソートに使用するテンポラリワークを生成するためのheapIDです。
extern void FIELD_WFBC_CORE_SortData( FIELD_WFBC_CORE* p_wk, HEAPID heapID );
// 1日の切り替え管理
extern void FIELD_WFBC_CORE_CalcOneDataStart( FIELD_WFBC_CORE* p_wk );
// 街に入った！計算
extern void FIELD_WFBC_CORE_CalcMoodInTown( FIELD_WFBC_CORE* p_wk );
// 人を足しこむ
extern void FIELD_WFBC_CORE_AddPeople( FIELD_WFBC_CORE* p_wk, const FIELD_WFBC_CORE_PEOPLE* cp_people );
// 人を探す
extern FIELD_WFBC_CORE_PEOPLE* FIELD_WFBC_CORE_GetNpcIDPeople( FIELD_WFBC_CORE* p_wk, u32 npc_id );

//-------------------------------------
///	FIELD_WFBC_CORE_PEOPLE用関数
//=====================================
//ワークのクリア
extern void FIELD_WFBC_CORE_PEOPLE_Clear( FIELD_WFBC_CORE_PEOPLE* p_wk );
//整合性チェック 
//@TODO　最終的には、対応する人の情報ももらって、整合性をあわせる
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsConfomity( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_PEOPLE_Management( FIELD_WFBC_CORE_PEOPLE* p_wk );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsInData( const FIELD_WFBC_CORE_PEOPLE* cp_wk );

// 話しかけた計算！
extern void FIELD_WFBC_CORE_PEOPLE_CalcTalk( FIELD_WFBC_CORE_PEOPLE* p_wk );

// 情報にアクセス
extern void FIELD_WFBC_CORE_PEOPLE_SetParentData( FIELD_WFBC_CORE_PEOPLE* p_wk, const MYSTATUS* cp_mystatus );
extern void FIELD_WFBC_CORE_PEOPLE_GetParentName( const FIELD_WFBC_CORE_PEOPLE* cp_wk, STRCODE* p_buff );
extern u32 FIELD_WFBC_CORE_PEOPLE_GetParentID( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
extern u32 FIELD_WFBC_CORE_PEOPLE_GetNpcID( const FIELD_WFBC_CORE_PEOPLE* cp_wk );








//-----------------------------------------------------------------------------
/**
 *					WFBC人物情報へのアクセス
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// エンカウントポケモンの数
#define FIELD_WFBC_PEOPLE_ENC_POKE_MAX  (3)
// バトルポケモンの数
#define FIELD_WFBC_PEOPLE_BTL_POKE_MAX  (3)

// メッセージの数
#define FIELD_WFBC_PEOPLE_WF_MESSAGE_MAX  (3)
#define FIELD_WFBC_PEOPLE_BC_MESSAGE_MAX  (4)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	人物情報
// 68バイト
//=====================================
typedef struct 
{
  u16     objid;    // オブジェコード
  u16     trtype;   // トレーナータイプ
  u16     mood_wf;  // 機嫌値　WF
  u16     mood_bc;  // 機嫌値  BC
  // エンカウント情報
  u16     enc_monsno[FIELD_WFBC_PEOPLE_ENC_POKE_MAX+1]; //+1パディング 8byte
  u8      enc_lv[FIELD_WFBC_PEOPLE_ENC_POKE_MAX+1];       // 4byte
  u8      enc_percent[FIELD_WFBC_PEOPLE_ENC_POKE_MAX+1];  // 4byte
  // バトルポケモン情報
  u16     btl_monsno[FIELD_WFBC_PEOPLE_BTL_POKE_MAX+1];   // 8byte
  u8      btl_lv[FIELD_WFBC_PEOPLE_BTL_POKE_MAX+1];   // 4byte
  u8      btl_sex[FIELD_WFBC_PEOPLE_BTL_POKE_MAX+1];  // 4byte
  u8      btl_tokusei[FIELD_WFBC_PEOPLE_BTL_POKE_MAX+1]; // 4byte
  // MESSAGE
  u16     msg_wf[FIELD_WFBC_PEOPLE_WF_MESSAGE_MAX+1]; // 8byte
  u16     msg_bc[FIELD_WFBC_PEOPLE_BC_MESSAGE_MAX]; // 8byte
  // 道具
  u16     goods_wf;
  u16     goods_wf_percent;
  u16     goods_bc;
  u16     goods_bc_money;
} FIELD_WFBC_PEOPLE_DATA;

//-------------------------------------
///	人物情報ローダー
//=====================================
typedef struct _FIELD_WFBC_PEOPLE_DATA_LOAD FIELD_WFBC_PEOPLE_DATA_LOAD;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern FIELD_WFBC_PEOPLE_DATA_LOAD* FIELD_WFBC_PEOPLE_DATA_Create( u32 npc_id, HEAPID heapID );
extern void FIELD_WFBC_PEOPLE_DATA_Delete( FIELD_WFBC_PEOPLE_DATA_LOAD* p_wk );

// 再読み込み
extern void FIELD_WFBC_PEOPLE_DATA_Load( FIELD_WFBC_PEOPLE_DATA_LOAD* p_wk, u32 npc_id );

// 情報の取得
extern const FIELD_WFBC_PEOPLE_DATA* FIELD_WFBC_PEOPLE_DATA_GetData( const FIELD_WFBC_PEOPLE_DATA_LOAD* cp_wk );
extern u32 FIELD_WFBC_PEOPLE_DATA_GetLoadNpcID( const FIELD_WFBC_PEOPLE_DATA_LOAD* cp_wk );



#ifdef _cplusplus
}	// extern "C"{
#endif


