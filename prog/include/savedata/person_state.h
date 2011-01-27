//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		person_state.h
 *	@brief  重要人物状態
 *	@author	Toru=Nagihashi
 *	@date		2011.01.19
 *
 *	@note 主にローカルライブキャスターで呼び出したときにどんな状態なのかを
 *	      取得するたのワークです。スクリプト側から設定し、プログラム側で取得します。
 *	      状態は、ローカルライブキャスターのエクセルデータからコンバートされます。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//セーブデータ関係
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "system/gf_date.h"

//定義
#include "..\..\src\app\local_tvt\local_tvt_call.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブワーク
//=====================================
typedef struct _PERSON_STATE PERSON_STATE;

//現在はMISCへ統合するため公開しています。
//セーブが開いたら隠す予定
#define PERSON_STATE_MAX  (8)
struct _PERSON_STATE
{
  u8  state[PERSON_STATE_MAX];
  u8  is_ltvt[PERSON_STATE_MAX];
  u8  is_secret_displeased;   //不機嫌かどうか
  u8  is_secret_date_event;   //デートイベントフラグ→ONのときはサブイベント発生まち、電話できない
  u8  is_secret_once_day_call;//１日１回電話フラグ
  u8  dummy;
  GFDATE  secret_date;        //デートの日
};

#define PERSON_STATE_WORK_SIZE  (24)

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern u32 PERSON_STATE_GetWorkSize( void );
extern void PERSON_STATE_Init( void *p_wk_adrs );
extern PERSON_STATE* PERSON_STATE_GetWork( SAVE_CONTROL_WORK *p_sv );
extern const PERSON_STATE* PERSON_STATE_GetWorkConst( const SAVE_CONTROL_WORK *cp_sv );

//各キャラの状態
extern void PERSON_STATE_SetState( PERSON_STATE* p_wk, u16 person, u16 state );
extern u16  PERSON_STATE_GetState( const PERSON_STATE* cp_wk, u16 person );

//各キャラのローカルライブキャスター電話設定
extern void PERSON_STATE_SetLocalTvtCall( PERSON_STATE *p_wk, u16 person );
extern BOOL PERSON_STATE_GetLocalTvtCall( const PERSON_STATE *cp_wk, u16 person );

//ローカルライブキャスター隠しキャラとのデートイベントフラグ
extern void PERSON_STATE_SetSecretDateEvent( PERSON_STATE *p_wk, const RTCDate *cp_date );
extern void PERSON_STATE_ClearSecretDateEvent( PERSON_STATE *p_wk );
extern BOOL PERSON_STATE_CheckSecretDataEventToday( const PERSON_STATE *cp_wk, const RTCDate *cp_date );
extern BOOL PERSON_STATE_CheckSecretDataEventLater( const PERSON_STATE *cp_wk, const RTCDate *cp_date );
extern BOOL PERSON_STATE_GetSecretDateEvent( const PERSON_STATE *cp_wk );

//ローカルライブキャスター隠しキャラの不機嫌フラグ
extern void PERSON_STATE_SetSecretDispleased( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSecretDispleased( const PERSON_STATE *cp_wk );

//ローカルライブキャスター隠しキャラ１日１回電話したフラグ
extern void PERSON_STATE_SetSecretOnceDayCallFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSecretOnceDayCallFlag( const PERSON_STATE *cp_wk );
