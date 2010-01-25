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

#include <net.h>

#include "gamesystem/gamedata_def.h"  //GAMEDATA
#include "field/intrude_common.h"
#include "field/field_wfbc_people_def.h"
#include "field/fldmmdl.h"
#include "field/intrude_common.h"

#include "field/field_status_local.h"

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
#define FIELD_WFBC_PEOPLE_MAX ( 10 )
#define FIELD_WFBC_INIT_PEOPLE_NUM ( 10 ) // 初期化での人の数を変更



//-------------------------------------
/// 機嫌
//=====================================
#define FIELD_WFBC_MOOD_MAX     ( 100 ) // 最大値
#define FIELD_WFBC_MOOD_ADD_INTOWN  ( 3 ) // 街に訪れると足される値
#define FIELD_WFBC_MOOD_ADD_TALK    ( 10 )  // 話をすると1日1回足される値
#define FIELD_WFBC_MOOD_SUB         ( -5 ) // 1日1回減る値

#define FIELD_WFBC_MOOD_TAKES       (20) // 連れていってほしいという値

#define FIELD_WFBC_MOOD_SUB_DAY_MAX ( FIELD_WFBC_MOOD_MAX / MATH_ABS(FIELD_WFBC_MOOD_SUB) + 1 ) // 引く日にちの最大値 + 1は割り切れないときの予備



//-------------------------------------
///	配置アイテム　不定値
//=====================================
#define FIELD_WFBC_ITEM_NONE  (0xffff)

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
  u8 data_in:4;   // 人の有無 TRUE FALSE
  u8 parent_in:4; // 親情報の有無 TRUE FALSE
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
  u32 random_no;// 街のランダム数
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

// WFBC パレス　開始
extern void GAMEDATA_SetUpPalaceWFBCCoreData( GAMEDATA * gamedata, const FIELD_WFBC_CORE* cp_core );
extern void GAMEDATA_ClearPalaceWFBCCoreData( GAMEDATA * gamedata );


// 1日の切り替え管理
extern void FIELD_WFBC_CORE_CalcOneDataStart( GAMEDATA * gamedata, s32 diff_day, HEAPID heapID );

//----------------------------------------------------------
//  ZONEDATAの操作    『常駐』
//----------------------------------------------------------
extern void FIELD_WFBC_CORE_SetUpZoneData( const FIELD_WFBC_CORE* cp_wk );


//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================
//ワークの全クリア
extern void FIELD_WFBC_CORE_Clear( FIELD_WFBC_CORE* p_wk ); // 『常駐』
extern void FIELD_WFBC_CORE_ClearNormal( FIELD_WFBC_CORE* p_wk );
extern void FIELD_WFBC_CORE_ClearBack( FIELD_WFBC_CORE* p_wk );
//街の情報をセットアップ
extern void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus, HEAPID tmp_heapID );
//整合性チェック
extern BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk );
// 人の数を返す
extern u32 FIELD_WFBC_CORE_GetPeopleNum( const FIELD_WFBC_CORE* cp_wk, MAPMODE mapmode );
// 機嫌値でソートする
// ソートに使用するテンポラリワークを生成するためのheapIDです。
extern void FIELD_WFBC_CORE_SortData( FIELD_WFBC_CORE* p_wk, HEAPID heapID );
// 街に入った！計算
extern void FIELD_WFBC_CORE_CalcMoodInTown( FIELD_WFBC_CORE* p_wk );
// 人を足しこむ
extern void FIELD_WFBC_CORE_AddPeople( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus, const FIELD_WFBC_CORE_PEOPLE* cp_people );
// 人を探す 常駐システム
extern FIELD_WFBC_CORE_PEOPLE* FIELD_WFBC_CORE_GetNpcIDPeople( FIELD_WFBC_CORE* p_wk, u32 npc_id, MAPMODE mapmode );
extern BOOL FIELD_WFBC_CORE_IsOnNpcIDPeople( const FIELD_WFBC_CORE* cp_wk, u32 npc_id, MAPMODE mapmode );
// データから、MMDLヘッダーを生成
// mapmode == field_status_local.h MAPMODE
// 戻り値は、GFL_HEAP_Freeをしてください。
extern MMDL_HEADER* FIELD_WFBC_CORE_MMDLHeaderCreateHeapLo( const FIELD_WFBC_CORE* cp_wk, MAPMODE mapmode, HEAPID heapID );


#ifdef PM_DEBUG
// まだ使用していないNPCをrandomで取得
extern u32 FIELD_WFBC_CORE_DEBUG_GetRandomNpcID( const FIELD_WFBC_CORE* cp_wk );
#endif

//-------------------------------------
///	FIELD_WFBC_CORE_PEOPLE用関数
//=====================================
//ワークのクリア
extern void FIELD_WFBC_CORE_PEOPLE_Clear( FIELD_WFBC_CORE_PEOPLE* p_wk ); // 『常駐』

//整合性チェック 　常駐システム
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsConfomity( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。　常駐システム
extern void FIELD_WFBC_CORE_PEOPLE_Management( FIELD_WFBC_CORE_PEOPLE* p_wk );
// データの有無   不正データの場合、FALSEを返します。 常駐システム
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsInData( const FIELD_WFBC_CORE_PEOPLE* cp_wk );

// 話しかけた計算！
extern void FIELD_WFBC_CORE_PEOPLE_CalcTalk( FIELD_WFBC_CORE_PEOPLE* p_wk );

// バトルした！設定
extern void FIELD_WFBC_CORE_PEOPLE_SetBattle( FIELD_WFBC_CORE_PEOPLE* p_wk );
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsBattle( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
 
// 情報にアクセス
extern void FIELD_WFBC_CORE_PEOPLE_SetParentData( FIELD_WFBC_CORE_PEOPLE* p_wk, const MYSTATUS* cp_mystatus );
extern void FIELD_WFBC_CORE_PEOPLE_GetParentName( const FIELD_WFBC_CORE_PEOPLE* cp_wk, STRCODE* p_buff );
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsParentIn( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
extern u32 FIELD_WFBC_CORE_PEOPLE_GetParentID( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
extern u32 FIELD_WFBC_CORE_PEOPLE_GetNpcID( const FIELD_WFBC_CORE_PEOPLE* cp_wk );
extern BOOL FIELD_WFBC_CORE_PEOPLE_IsMoodTakes( const FIELD_WFBC_CORE_PEOPLE* cp_wk );









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
typedef enum
{
  FIELD_WFBC_PEOPLE_ENC_POKE_GRASS_LOW,
  FIELD_WFBC_PEOPLE_ENC_POKE_GRASS_HIGH,
  FIELD_WFBC_PEOPLE_ENC_POKE_WATER,
  FIELD_WFBC_PEOPLE_ENC_POKE_MAX,
  
} FIELD_WFBC_PEOPLE_ENC_POKE_TYPE;

// メッセージの数
#define FIELD_WFBC_PEOPLE_WF_MESSAGE_MAX  (3)
#define FIELD_WFBC_PEOPLE_BC_MESSAGE_MAX  (4)

// エンカウント抽選確率　最大値
#define FIELD_WFBC_PEOPLE_ENC_POKE_PERCENT_MAX  ( 100 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	人物情報
// 44バイト
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
  u32     btl_trdata;   // 4byte
  // SCRIPT
  u16     script_wf; 
  u16     script_bc; 
  u16     script_plcw; 
  u16     script_plc; 
  // 道具
  u16     goods_wf;
  u16     goods_wf_percent;
  u16     goods_bc;
  u16     goods_bc_money;
  // 出現確率
  u16     hit_percent;
  // ブロック計算補正値
  u16     block_param;
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


//-----------------------------------------------------------------------------
/**
 *					WFBC人物位置情報へのアクセス
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	人物配置情報
//=====================================
typedef struct
{
  u8 gx;
  u8 gz;
} FIELD_WFBC_CORE_PEOPLE_POS;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern FIELD_WFBC_CORE_PEOPLE_POS* FIELD_WFBC_PEOPLE_POS_Create( FIELD_WFBC_PEOPLE_DATA_LOAD* p_loader, FIELD_WFBC_CORE_TYPE type, HEAPID heapID );
extern void FIELD_WFBC_PEOPLE_POS_Delete( FIELD_WFBC_CORE_PEOPLE_POS* p_wk );

extern const FIELD_WFBC_CORE_PEOPLE_POS* FIELD_WFBC_PEOPLE_POS_GetIndexData( const FIELD_WFBC_CORE_PEOPLE_POS* cp_wk, u32 index );

extern const FIELD_WFBC_CORE_PEOPLE_POS* FIELD_WFBC_PEOPLE_POS_GetIndexItemPos( const FIELD_WFBC_CORE_PEOPLE_POS* cp_wk, u32 index );


//-----------------------------------------------------------------------------
/**
 *					WFBCアイテム配置情報
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	WFBCアイテム配置情報
//=====================================
typedef struct {
  u16 scr_item[ FIELD_WFBC_PEOPLE_MAX ];  // idx位置のアイテムスクリプトID
} FIELD_WFBC_CORE_ITEM;

//-------------------------------------
///	GAMEDATAから取得
//=====================================
extern FIELD_WFBC_CORE_ITEM* GAMEDATA_GetWFBCItemData( GAMEDATA * gamedata );


//-------------------------------------
///	全体クリア
// 常駐
//=====================================
extern void WFBC_CORE_ITEM_ClaerAll( FIELD_WFBC_CORE_ITEM* p_wk );

//-------------------------------------
///	アイテム数
//=====================================
extern u32 WFBC_CORE_ITEM_GetNum( const FIELD_WFBC_CORE_ITEM* cp_wk );

//-------------------------------------
///	アイテム配置情報を設定
//=====================================
extern BOOL FIELD_WFBC_CORE_ITEM_SetItemData( FIELD_WFBC_CORE_ITEM* p_wk, u16 scr_item, u32 idx );
extern void FIELD_WFBC_CORE_ITEM_ClearItemData( FIELD_WFBC_CORE_ITEM* p_wk, u32 idx );

//-------------------------------------
///	アイテム配置情報を取得
//=====================================
extern u16 FIELD_WFBC_CORE_ITEM_GetItemData( const FIELD_WFBC_CORE_ITEM* cp_wk, u32 idx );
extern BOOL FIELD_WFBC_CORE_ITEM_IsInItemData( const FIELD_WFBC_CORE_ITEM* cp_wk, u32 idx );

//-------------------------------------
///	データから、MMDLヘッダーを生成
//=====================================
extern MMDL_HEADER* FIELD_WFBC_CORE_ITEM_MMDLHeaderCreateHeapLo( const FIELD_WFBC_CORE_ITEM* cp_wk, MAPMODE mapmode, FIELD_WFBC_CORE_TYPE type, HEAPID heapID );






//-----------------------------------------------------------------------------
/**
 *					WFBCイベント管理ワーク
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	BCイベント定数
//=====================================
#define FIELD_WFBC_EVENT_NPC_WIN_TARGET_INIT (10)
#define FIELD_WFBC_EVENT_NPC_WIN_TARGET_ADD (10)
#define FIELD_WFBC_EVENT_NPC_WIN_TARGET_MAX (1000)

//-------------------------------------
///	イベントワーク
//=====================================
typedef union {
  
  // BC用ワーク
  struct
  {
    u16 bc_npc_win_count;     // ボスイベント　BCの人に勝った数
    u16 bc_npc_win_target;    // ボスイベント　BCの人の勝利目標数
  };

  // WF用ワーク
  struct
  {
    u16 wf_poke_catch_item;   // 村長イベント　もらえるアイテム
    u16 wf_poke_catch_monsno; // 村長イベント　もってきてもらいたいポケモンナンバー
  };
} FIELD_WFBC_EVENT;

//-------------------------------------
///	GAMEDATAから取得
//=====================================
extern FIELD_WFBC_EVENT* GAMEDATA_GetWFBCEventData( GAMEDATA * gamedata );


//-------------------------------------
///	ワーク操作
//  常駐
//=====================================
extern void FIELD_WFBC_EVENT_Clear( FIELD_WFBC_EVENT* p_wk );

// BC
extern u16 FIELD_WFBC_EVENT_GetBCNpcWinCount( const FIELD_WFBC_EVENT* cp_wk );
extern void FIELD_WFBC_EVENT_AddBCNpcWinCount( FIELD_WFBC_EVENT* p_wk );
extern u16 FIELD_WFBC_EVENT_GetBCNpcWinTarget( const FIELD_WFBC_EVENT* cp_wk );
extern void FIELD_WFBC_EVENT_AddBCNpcWinTarget( FIELD_WFBC_EVENT* p_wk );

// WF
extern u16 FIELD_WFBC_EVENT_GetWFPokeCatchEventMonsNo( const FIELD_WFBC_EVENT* cp_wk );
extern void FIELD_WFBC_EVENT_SetWFPokeCatchEventMonsNo( FIELD_WFBC_EVENT* p_wk, u16 mons_no );
extern u16 FIELD_WFBC_EVENT_GetWFPokeCatchEventItem( const FIELD_WFBC_EVENT* cp_wk );
extern void FIELD_WFBC_EVENT_SetWFPokeCatchEventItem( FIELD_WFBC_EVENT* p_wk, u16 item );


//-----------------------------------------------------------------------------
/**
 *					通信情報
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	NPC_IDさんの状態取得リクエストタイプ
//=====================================
typedef enum 
{
  FIELD_WFBC_COMM_NPC_REQ_THERE,      // 所在確認
  FIELD_WFBC_COMM_NPC_REQ_WISH_TAKE,  // 連れて行きたい
  FIELD_WFBC_COMM_NPC_REQ_TAKE,       // 連れていった

  FIELD_WFBC_COMM_NPC_REQ_TYPE_NUM,   // システム内で使用
} FIELD_WFBC_COMM_NPC_REQ_TYPE;


//-------------------------------------
///	NPC_IDさんの状態 応答タイプ
//=====================================
typedef enum 
{
  FIELD_WFBC_COMM_NPC_ANS_ON,         // います。
  FIELD_WFBC_COMM_NPC_ANS_OFF,        // いません。
  FIELD_WFBC_COMM_NPC_ANS_TAKE_OK,    // つれていってください。
  FIELD_WFBC_COMM_NPC_ANS_TAKE_NG,    // つれていけません。

  FIELD_WFBC_COMM_NPC_ANS_TYPE_NUM,   // システム内で使用
} FIELD_WFBC_COMM_NPC_ANS_TYPE;


//-------------------------------------
///	子ー＞親  NPC_ID関連　リクエスト
//=====================================
typedef struct {
  int net_id;   // リクエストを出しているのは誰か？
  u16 npc_id;
  u16 req_type; // FIELD_WFBC_COMM_NPC_REQ_TYPE
} FIELD_WFBC_COMM_NPC_REQ;

//-------------------------------------
///	親ー＞子  NPC_ID関連　アンサー
//=====================================
typedef struct {
  int net_id;   // 誰に対する返答か？
  u16 npc_id;
  u16 ans_type; // FIELD_WFBC_COMM_NPC_ANS_TYPE
} FIELD_WFBC_COMM_NPC_ANS;

//-------------------------------------
///	通信用情報
//=====================================
typedef struct {
  u16 netID;
  
  u16 buff_msk;

  u16 intrudeNetID;
  
  // 通信バッファ
  FIELD_WFBC_COMM_NPC_REQ recv_req_que[ FIELD_COMM_MEMBER_MAX ];
  FIELD_WFBC_COMM_NPC_ANS recv_ans;
  
  FIELD_WFBC_COMM_NPC_REQ send_req;
  FIELD_WFBC_COMM_NPC_ANS send_ans_que[ FIELD_COMM_MEMBER_MAX ];
} WFBC_COMM_DATA;

//-------------------------------------
///	処理管理
//=====================================
extern void FIELD_WFBC_COMM_DATA_Init( WFBC_COMM_DATA* p_wk );
extern void FIELD_WFBC_COMM_DATA_Exit( WFBC_COMM_DATA* p_wk );
extern void FIELD_WFBC_COMM_DATA_Oya_Main( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, u8 member_bit );
extern void FIELD_WFBC_COMM_DATA_Ko_ChangeNpc( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, FIELD_WFBC_CORE* p_oyawfbc, const MYSTATUS* cp_mystatus, u16 npc_id );


extern void FIELD_WFBC_COMM_DATA_SetIntrudeNetID( WFBC_COMM_DATA* p_wk, NetID intrudeNetID );

//-------------------------------------
///	通信用処理
//    受信のながれ
//      受信した情報を以下の関数に渡すだけです。
//        FIELD_WFBC_COMM_DATA_SetRecvCommAnsData
//        FIELD_WFBC_COMM_DATA_SetRecvCommReqData
//    送信の流れ
//      以下の関数で、送信情報があるかチェックし、送信してください。
//        FIELD_WFBC_COMM_DATA_GetSendCommAnsData
//        FIELD_WFBC_COMM_DATA_GetSendCommAnsData
//      送信が完了したら、送信情報をクリアしてください。
//        FIELD_WFBC_COMM_DATA_ClearSendCommAnsData
//        FIELD_WFBC_COMM_DATA_ClearSendCommReqData
//
//    親にリクエストを出す。応答を待つ
//      まず、内部バッファをクリアします。
//        FIELD_WFBC_COMM_DATA_ClearReqAnsData
//      リクエスト情報を設定します。
//        FIELD_WFBC_COMM_DATA_SetReqData
//      応答を待ちます。
//        FIELD_WFBC_COMM_DATA_WaitAnsData
//      応答情報を取得します。
//        FIELD_WFBC_GetAnsData
//      
//=====================================
// 受信情報の設定
extern void FIELD_WFBC_COMM_DATA_SetRecvCommAnsData( WFBC_COMM_DATA* p_wk, const FIELD_WFBC_COMM_NPC_ANS* cp_ans );
extern void FIELD_WFBC_COMM_DATA_SetRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID, const FIELD_WFBC_COMM_NPC_REQ* cp_req );

// 送信情報の取得
extern BOOL FIELD_WFBC_COMM_DATA_GetSendCommAnsData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_ANS* p_ans ); // 0～3ループでまわして、毎フレームチェックしてください。
extern BOOL FIELD_WFBC_COMM_DATA_GetSendCommReqData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_REQ* p_req, NetID* p_send_netID );

// 送信完了したデータは破棄してください。
extern void FIELD_WFBC_COMM_DATA_ClearSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID );
extern void FIELD_WFBC_COMM_DATA_ClearSendCommReqData( WFBC_COMM_DATA* p_wk );

// リクエスト情報の設定
extern void FIELD_WFBC_COMM_DATA_SetReqData( WFBC_COMM_DATA* p_wk, u16 npc_id, FIELD_WFBC_COMM_NPC_REQ_TYPE req_type );
extern BOOL FIELD_WFBC_COMM_DATA_WaitAnsData( const WFBC_COMM_DATA* cp_wk, u16 npc_id );
extern FIELD_WFBC_COMM_NPC_ANS_TYPE FIELD_WFBC_GetAnsData( const WFBC_COMM_DATA* cp_wk );

// リクエストを出す前に呼んでください
extern void FIELD_WFBC_COMM_DATA_ClearReqAnsData( WFBC_COMM_DATA* p_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif


