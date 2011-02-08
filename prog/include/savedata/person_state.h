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
//隠しキャラのデートの状態
typedef enum
{
  PERSON_STATE_SECRET_DATE_NONE,      //なし
  PERSON_STATE_SECRET_DATE_FIRST,     //最初のデート
  PERSON_STATE_SECRET_DATE_REGULAR,   //定期デート
}PERSON_STATE_SECRET_DATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	セーブワーク
//=====================================
typedef struct _PERSON_STATE PERSON_STATE;

//現在はETCへ統合するため公開しています。
//セーブが開いたら隠す予定
#define PERSON_STATE_MAX  (8)
struct _PERSON_STATE
{
  u8  state[PERSON_STATE_MAX];
  u8  is_ltvt[PERSON_STATE_MAX];
  u8  is_secret_displeased;   //不機嫌かどうか
  u8  secret_date_event;   //デートイベント状態→ONのときはサブイベント発生まち、電話できない PERSON_STATE_SECRET_DATEの値
  u8  is_secret_once_day_call;//１日１回電話フラグ
  u8  is_season_date;         //１季節に１回のデートフラグ
  GFDATE  secret_date;        //デートの日
};

#define PERSON_STATE_WORK_SIZE  (24)

SDK_COMPILER_ASSERT( PERSON_STATE_WORK_SIZE == sizeof(PERSON_STATE) );

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
extern void PERSON_STATE_SetLocalTvtCall( PERSON_STATE *p_wk, u16 person, BOOL is_ltvt );
extern BOOL PERSON_STATE_GetLocalTvtCall( const PERSON_STATE *cp_wk, u16 person );


//ローカルライブキャスター隠しキャラ
extern void PERSON_STATE_SetSecretDateEvent( PERSON_STATE *p_wk, PERSON_STATE_SECRET_DATE date, const RTCDate *cp_date );
extern void PERSON_STATE_ClearSecretDateEvent( PERSON_STATE *p_wk );
extern BOOL PERSON_STATE_CheckSecretDataEventToday( const PERSON_STATE *cp_wk, const RTCDate *cp_date );
extern BOOL PERSON_STATE_CheckSecretDataEventLater( const PERSON_STATE *cp_wk, const RTCDate *cp_date );
extern PERSON_STATE_SECRET_DATE PERSON_STATE_GetSecretDateEvent( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetSecretDispleased( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSecretDispleased( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetSecretOnceDayCallFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSecretOnceDayCallFlag( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetSeasonDateFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSeasonDateFlag( const PERSON_STATE *cp_wk );

#ifdef PM_DEBUG
extern void DEBUG_PERSON_STATE_GetDate( const PERSON_STATE *cp_wk, RTCDate *p_date );
#endif //PM_DEBUG
