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

#include "system/vm.h"

#include "gamesystem/gamedata_def.h"  //GAMEDATA
#include "field/intrude_common.h"
#include "field/field_wfbc_people_def.h"
#include "field/fldmmdl.h"
#include "field/intrude_common.h"

#include "field/field_status_local.h"
#include "field/scrcmd_shop.h"


#include "savedata/mystatus.h"

#include "buflen.h"



//-------------------------------------
///	デバック用宣言
//=====================================
#ifdef PM_DEBUG

#define FIELD_WFBC_MAKE_MAPDATA_DEBUG

#endif



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
//src/field/scrcmd_keysystem.hで同じ並びでDeFINEしているので順番かえちゃダメ
typedef enum
{
  FIELD_WFBC_CORE_TYPE_BLACK_CITY,    // ブラックシティ
  FIELD_WFBC_CORE_TYPE_WHITE_FOREST,  // ホワイトフォレスト

  FIELD_WFBC_CORE_TYPE_MAX,  // 街の数
} FIELD_WFBC_CORE_TYPE;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

// 全体のCOREデータ 
typedef struct 
{
  u16 data_in;  // TRUE FALSE
  u16 type;     // FIELD_WFBC_CORE_TYPE
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
extern FIELD_WFBC_CORE_TYPE GAMEDATA_GetMyWFBCType( const GAMEDATA * gamedata );

// WFBC パレス　開始
extern void GAMEDATA_SetUpPalaceWFBCCoreData( GAMEDATA * gamedata, const FIELD_WFBC_CORE* cp_core );
extern void GAMEDATA_ClearPalaceWFBCCoreData( GAMEDATA * gamedata );


//----------------------------------------------------------
//  ZONEDATAの操作    『常駐』
//----------------------------------------------------------
extern void FIELD_WFBC_CORE_SetUpZoneData( const FIELD_WFBC_CORE* cp_wk );

//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================
//ワークの全クリア
extern void FIELD_WFBC_CORE_Clear( FIELD_WFBC_CORE* p_wk ); // 『常駐』
//街の情報をセットアップ
extern void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus, SAVE_CONTROL_WORK *p_sv, HEAPID tmp_heapID );
//整合性チェック
extern BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk );
//データの調整  不正データの場合、正常な情報に書き換えます。
extern void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus );
// データの有無   不正データの場合、FALSEを返します。
extern BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk );

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

// リクエストを出す前に呼んでください
extern void FIELD_WFBC_COMM_DATA_ClearReqAnsData( WFBC_COMM_DATA* p_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif


