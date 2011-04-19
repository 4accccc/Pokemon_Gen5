//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_people.c
 *	@brief  レゾナンスリゾート人物管理
 *	@author	Toru=Nagihashi
 *	@date		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "resort_people_def.h"
#include "resort_binary.h"
#include "field/resonance_resort_data.h"
#include "../fldmmdl.h"
#include "../field_player.h"
#include "resort_data_manager.h"

//=============================================================================
/**
 *				人物ワーク
 *				  人物個別操作、または人物情報設定や読み取り
*/
//=============================================================================
//-------------------------------------
///	行動
//=====================================
typedef enum
{
  RESORT_PEOPLE_ACTION_WARPDOWN,  //下にワープ
  RESORT_PEOPLE_ACTION_WARPUP,    //上にワープ
  RESORT_PEOPLE_ACTION_JUMP_START,  //ジャンプ開始
  RESORT_PEOPLE_ACTION_JUMP_END,    //ジャンプ終了
  RESORT_PEOPLE_ACTION_MAX,
}RESORT_PEOPLE_ACTION;

//-------------------------------------
///	人物ワーク
//=====================================
typedef struct _RESORT_PEOPLE RESORT_PEOPLE;
//-------------------------------------
///	
//=====================================
typedef enum 
{
  RESORT_PEOPLE_DATA_PARAM_TYPE, //人物の種類 旅行者とか
} RESORT_PEOPLE_DATA_PARAM;

//-------------------------------------
///	PUBLIC
//=====================================
//行動
extern void RESORT_PEOPLE_ActionReq( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_ACTION action );
extern BOOL RESORT_PEOPLE_IsActionEnd( const RESORT_PEOPLE *cp_wk );

//設定
extern void RESORT_PEOPLE_SetVisible( RESORT_PEOPLE *p_wk, BOOL is_visible );
extern void RESORT_PEOPLE_SetData( RESORT_PEOPLE *p_wk, RESORT_DATA_PARAM param, u32 data );
extern u32 RESORT_PEOPLE_GetData( const RESORT_PEOPLE *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern u32 RESORT_PEOPLE_GetPeopleData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_DATA_PARAM param );
extern u32 RESORT_PEOPLE_GetMsgID( const RESORT_PEOPLE *cp_wk, u32 index );
extern MMDL * RESORT_PEOPLE_GetMmdl( const RESORT_PEOPLE *cp_wk );
extern void * RESORT_PEOPLE_GetDataAdrs( RESORT_PEOPLE *p_wk );
//住人しか使えない
extern RESORT_INSTITUTION_DATA * RESORT_PEOPLE_GetInstitution( RESORT_PEOPLE *p_wk );

//=============================================================================
/**
 *					人物管理ワーク
 *					  人物ワークの作成、破棄、一括処理
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//人物管理ワークでMMDLに登録するIDの開始値
//ただし、0ofsは無効値としているので、
//実質129～
#define RESORT_PEOPLE_MMDLID_OFS (128)
//peopleIDは場所内インデックス
#define RESORT_PEOPLE_MMDLID_TO_PEOPLEID(x) (x-RESORT_PEOPLE_MMDLID_OFS)
#define RESORT_PEOPLE_PEOPLEID_TO_MMDLID(x) (x+RESORT_PEOPLE_MMDLID_OFS)

//-------------------------------------
///	人物管理ワーク
//=====================================
typedef struct _RESORT_PEOPLE_SYS RESORT_PEOPLE_SYS;

//-------------------------------------
///	人物管理ワーク生成パラメータ
//=====================================
typedef struct
{
  MMDLSYS *p_mmdlsys;                 //動作モデルシステム
  RESORT_DATA_MANAGER *p_data_manager;//データマネージャー
  u32 max;                            //動作上限数
  RESORT_PLACEID placeID;                      //人物管理システムが動く場所
} RESORT_PEOPLE_SYS_PARAM;

//-------------------------------------
///	PUBLIC
//=====================================
//システム作成、破棄、メイン
extern RESORT_PEOPLE_SYS * RESORT_PEOPLE_SYS_Init( const RESORT_PEOPLE_SYS_PARAM *cp_param, HEAPID heapID );
extern void RESORT_PEOPLE_SYS_Exit( RESORT_PEOPLE_SYS *p_wk );
extern void RESORT_PEOPLE_SYS_Main( RESORT_PEOPLE_SYS *p_wk,FIELD_PLAYER *p_player );

extern u32 RESORT_PEOPLE_SYS_GetPeopleMax( const RESORT_PEOPLE_SYS *cp_wk );

//人物を一人登録
typedef struct
{
  RESORT_PEOPLE_TYPE people_type;
  void *p_data_adrs;
  RESORT_PLACEID placeID;
}RESORT_PEOPLE_SYS_ADD_PARAM;
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_AddNew( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param );
//人物を一括登録
extern void RESORT_PEOPLE_SYS_AddAll( RESORT_PEOPLE_SYS *p_wk );
//人物ワークのメモリを開放する
extern void RESORT_PEOPLE_SYS_Remove( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people );
extern void RESORT_PEOPLE_SYS_RemoveAll( RESORT_PEOPLE_SYS *p_wk );

extern void RESORT_PEOPLE_SYS_EraseMMdl( RESORT_PEOPLE_SYS *p_wk );

//人物を完全に消す
extern void RESORT_PEOPLE_Delete( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people );

//人物を取得
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByMmdl( RESORT_PEOPLE_SYS *p_wk, const MMDL* cp_mmdl );
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByData( RESORT_PEOPLE_SYS *p_wk, const void *cp_data_adrs );
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByIndex( RESORT_PEOPLE_SYS *p_wk, u32 idx );
