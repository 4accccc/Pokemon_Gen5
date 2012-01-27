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
#include "field/fldmmdl_code.h"

//=============================================================================
/**
 *				人物ワーク
 *				  人物個別操作、または人物情報設定や読み取り
*/
//=============================================================================
//-------------------------------------
///	人物ワーク
//=====================================
typedef struct _RESORT_PEOPLE RESORT_PEOPLE;
//-------------------------------------
///	取得タイプ
//=====================================
typedef enum 
{
  RESORT_PEOPLE_DATA_PARAM_TYPE,    //人物の種類 旅行者とか
  RESORT_PEOPLE_DATA_PARAM_CREATE,  //生成状態
} RESORT_PEOPLE_DATA_PARAM;

//-------------------------------------
///	生成状態
//=====================================
typedef enum
{
  RESORT_PEOPLE_CREATE_NEW,     //新規作成
  RESORT_PEOPLE_CREATE_ADDALL,  //AddAll
} RESORT_PEOPLE_CREATE;

//-------------------------------------
///	PUBLIC
//=====================================
//行動

//設定
extern void RESORT_PEOPLE_SetVisible( RESORT_PEOPLE *p_wk, BOOL is_visible );

typedef enum
{
  RESORT_PEOPLE_VISIBLE_ON,
  RESORT_PEOPLE_VISIBLE_OFF,
  RESORT_PEOPLE_VISIBLE_MASTER,
}RESORT_PEOPLE_VISIBLE;
extern void RESORT_PEOPLE_SetTemporaryVisible( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_VISIBLE flag );

extern void RESORT_PEOPLE_SetData( RESORT_PEOPLE *p_wk, RESORT_DATA_PARAM param, u32 data );
extern u32 RESORT_PEOPLE_GetData( const RESORT_PEOPLE *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern u32 RESORT_PEOPLE_GetPeopleData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_DATA_PARAM param );
extern MMDL * RESORT_PEOPLE_GetMmdl( const RESORT_PEOPLE *cp_wk );
extern void * RESORT_PEOPLE_GetDataAdrs( RESORT_PEOPLE *p_wk );
extern RESORT_ABSTRUCT_DATA * RESORT_PEOPLE_GetAbstructData( RESORT_PEOPLE *p_wk );
extern void RESORT_PEOPLE_ChangeOBJCode( RESORT_PEOPLE *p_wk );
extern void RESORT_PEOPLE_SetTalkFlag( RESORT_PEOPLE *p_wk, BOOL is_talk );
extern BOOL RESORT_PEOPLE_GetTalkFlag( const RESORT_PEOPLE *cp_wk );

//住人しか使えない
extern RESORT_INSTITUTION_DATA * RESORT_PEOPLE_GetInstitution( RESORT_PEOPLE *p_wk );

extern void RESORT_PEOPLE_ResetPos( RESORT_PEOPLE *p_wk );
extern void RESORT_PEOPLE_GetInstitutionFrontPos( RESORT_PEOPLE *p_wk, u16 *p_x, u16 *p_z, u16 *p_dir );

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

#define RESORT_PEOPLE_MAX_CREATE_NUM  (68)

#define RESORT_PEOPLE_MMDLID_OFS_S (MMDL_ID_RRESORT_START)
#define RESORT_PEOPLE_MMDLID_OFS_E (RESORT_PEOPLE_MMDLID_OFS_S+RESORT_PEOPLE_MAX_CREATE_NUM)
//peopleIDは場所内インデックス
#define RESORT_PEOPLE_MMDLID_TO_PEOPLEID(x) (x-RESORT_PEOPLE_MMDLID_OFS_S)
#define RESORT_PEOPLE_PEOPLEID_TO_MMDLID(x) (x+RESORT_PEOPLE_MMDLID_OFS_S)

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
  RESORT_ABSTRUCT_MANAGER *p_abstruct;//データ抽象
  u32 max;                            //動作上限数
  RESORT_PLACEID placeID;                      //人物管理システムが動く場所
  const RESORT_BINARY_SHOP  *cp_shop;           //お店データ
  const RESORT_BINARY_INSTITUTION *cp_institution;  //施設データ
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
  RESORT_ABSTRUCT_DATA *p_data;
  RESORT_PLACEID placeID;
}RESORT_PEOPLE_SYS_ADD_PARAM;
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_AddNew( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param );
//人物を一括登録
extern void RESORT_PEOPLE_SYS_AddAll( RESORT_PEOPLE_SYS *p_wk );
//住人だけ登録
extern void RESORT_PEOPLE_SYS_AddPeopleBufferType( RESORT_PEOPLE_SYS *p_wk, const RESORT_DATA_BUFFER_TYPE c_buffer_tbl[], u32 tbl_len );
//人物ワークのメモリを開放する
extern void RESORT_PEOPLE_SYS_Remove( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people );
extern void RESORT_PEOPLE_SYS_RemoveAll( RESORT_PEOPLE_SYS *p_wk );

extern void RESORT_PEOPLE_SYS_EraseMMdl( RESORT_PEOPLE_SYS *p_wk );

extern void RESORT_PEOPLE_SYS_ReturnMasterVisibleAll( RESORT_PEOPLE_SYS *p_wk);

extern void RESORT_PEOPLE_SYS_ResetMmdl( RESORT_PEOPLE_SYS *p_sys, RESORT_PEOPLE *p_people );

//人物を完全に消す
extern void RESORT_PEOPLE_Delete( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people );

//人物を取得
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByMmdl( RESORT_PEOPLE_SYS *p_wk, const MMDL* cp_mmdl );
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByData( RESORT_PEOPLE_SYS *p_wk, const void *cp_data_adrs );
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByIndex( RESORT_PEOPLE_SYS *p_wk, u32 idx );
extern RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByAbstruct( RESORT_PEOPLE_SYS *p_wk, const RESORT_ABSTRUCT_DATA *cp_abstruct );

extern BOOL RESORT_PEOPLE_TOOL_IsObjID( int objid );
