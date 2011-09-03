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
  PERSON_STATE_SECRET_DATE_GIVE,      //受け渡し
  PERSON_STATE_SECRET_DATE_MAX
}PERSON_STATE_SECRET_DATE;

typedef enum
{
  PERSON_STATE_SECRET_CALLPLACE_C04,
  PERSON_STATE_SECRET_CALLPLACE_C05,
  PERSON_STATE_SECRET_CALLPLACE_C06,
  PERSON_STATE_SECRET_CALLPLACE_T03,
  PERSON_STATE_SECRET_CALLPLACE_T04,
  PERSON_STATE_SECRET_CALLPLACE_T07,
  PERSON_STATE_SECRET_CALLPLACE_R05,
  PERSON_STATE_SECRET_CALLPLACE_R06,
  PERSON_STATE_SECRET_CALLPLACE_R07,
  PERSON_STATE_SECRET_CALLPLACE_R09,
  PERSON_STATE_SECRET_CALLPLACE_R11,
  PERSON_STATE_SECRET_CALLPLACE_R12,
  PERSON_STATE_SECRET_CALLPLACE_R13,
  PERSON_STATE_SECRET_CALLPLACE_R14,
  PERSON_STATE_SECRET_CALLPLACE_R16,
  PERSON_STATE_SECRET_CALLPLACE_MAX,
}PERSON_STATE_SECRET_CALLPLACE;

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
  u8  secret_date_event;   //デートイベント状態→ONのときはサブイベント発生まち、電話できない PERSON_STATE_SECRET_DATEの値
  u8  is_secret_once_date:1;  //１日１回デート
  u8  is_trade:1;               //交換フラグ
  u8  is_date_end:1;            //デート終了フラグ
  u8  dummy:5;
  u16 trade_monsno;           //交換ポケモン種別
  u8  call_place[ PERSON_STATE_SECRET_CALLPLACE_MAX/8 ]; 
  u16 dummy2;
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
extern void PERSON_STATE_SetSecretDateEvent( PERSON_STATE *p_wk, PERSON_STATE_SECRET_DATE date );
extern void PERSON_STATE_ClearSecretDateEvent( PERSON_STATE *p_wk );
extern PERSON_STATE_SECRET_DATE PERSON_STATE_GetSecretDateEvent( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetSecretOnceDateFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetSecretOnceDateFlag( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetTradeFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetTradeFlag( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetTradeMonsno( PERSON_STATE *p_wk, u16 monsno );
extern u16 PERSON_STATE_GetTradeMonsno( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetDateEndFlag( PERSON_STATE *p_wk, BOOL enable );
extern BOOL PERSON_STATE_GetDateEndFlag( const PERSON_STATE *cp_wk );

extern void PERSON_STATE_SetCallPlaceFlag( PERSON_STATE *p_wk, PERSON_STATE_SECRET_CALLPLACE place, BOOL enable );
extern BOOL PERSON_STATE_GetCallPlaceFlag( const PERSON_STATE *cp_wk, PERSON_STATE_SECRET_CALLPLACE place );
