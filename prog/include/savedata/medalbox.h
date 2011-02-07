
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox.h
 *	@brief  メダルボックスデータ
 *	@author	Masayuki Onoue
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include  <gflib.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "../../../resource/medal/data/mdlid_def.h" // for MDLID_MAX

//=============================================================================
/**
 *					定数定義
*/
//=============================================================================

//-------------------------------------
///	メダル状態
//  下のものほど優先度が高く、タイトル達成→ヒント取得と遷移することはない
//=====================================
enum{
  MEDAL_STATE_NONE =0,      // 状態なし
  MEDAL_STATE_HINT_ACHIVE,  // ヒント達成
  MEDAL_STATE_HINT_GET,     // ヒント取得
  MEDAL_STATE_TITLE_ACHIVE, // タイトル達成
  MEDAL_STATE_TITLE_GET,    // タイトル取得
  MEDAL_STATE_MAX,
};

// ===========================
// ボックスランク
// ===========================
enum{
  MEDALBOX_RANK_IRON,
  MEDALBOX_RANK_COPPER,
  MEDALBOX_RANK_SILVER,
  MEDALBOX_RANK_GOLD,
  MEDALBOX_RANK_BW,
  MEDALBOX_RANK_MAX,
};

// ===========================
// ボックスランク毎のメダル最大数
// ===========================
enum{
  MEDALBOX_RANK_IRON_MAX    = 49,
  MEDALBOX_RANK_COPPER_MAX  = 99,
  MEDALBOX_RANK_SILVER_MAX  = 149,
  MEDALBOX_RANK_GOLD_MAX    = 199,
  MEDALBOX_RANK_BW_MAX      = 200,
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
typedef struct _MEDALBOX MEDALBOX;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// メダル状態取得
extern u8 MEDALBOX_GetMedalState( const MEDALBOX *p_mdlbox, MedalID medal_id );

// 達成状態
extern void MEDALBOX_SetTitleMedalAchive( MEDALBOX * p_mdlbox, MedalID medal_id );
extern void MEDALBOX_SetHintMedalAchive( MEDALBOX * p_mdlbox, MedalID medal_id );

// メダル獲得関数
extern void MEDALBOX_SetTitleMedalGetFlg( MEDALBOX *p_mdlbox, MedalID medal_id, u8 year, u8 month, u8 day );
extern void MEDALBOX_SetHintMedalGetFlg( MEDALBOX *p_mdlbox, MedalID medal_id , u8 year, u8 month, u8 day);

// 新着フラグ
extern void MEDALBOX_SetNewMedalFlgOff( MEDALBOX *p_mdlbox, MedalID medal_id );
extern BOOL MEDALBOX_GetNewMedalFlg( const MEDALBOX *p_mdlbox, MedalID medal_id );

// 年月日
extern void MEDALBOX_GetDate( const MEDALBOX *p_mdlbox, MedalID medal_id , u8 *p_year , u8 *p_month , u8 *p_day );


// イチオシメダル番号
extern MedalID MEDALBOX_GetPushMedalID( const MEDALBOX *p_mdlbox );
extern void MEDALBOX_SetPushMedalID( MEDALBOX *p_mdlbox , MedalID medal_id );

// ボックスランク
extern void MEDALBOX_RankUpMedalBox( MEDALBOX *p_mdlbox );
extern u8 MEDALBOX_GetMedalBoxRank( const MEDALBOX *p_mdlbox );

// 現在のメダル取得数
extern u16 MEDALBOX_GetMedalCount( const MEDALBOX *p_mdlbox , BOOL is_hint_count );

// 現在のランクでメダルボックスに入るメダル最大数を取得
extern u16 MEDALBOX_GetMedalBoxMax( const MEDALBOX *p_mdlbox );


// セーブデータのポインタを返す
extern MEDALBOX* SaveData_GetMedalBox( SAVE_CONTROL_WORK *p_sv );

#ifdef PM_DEBUG
extern void DEBUG_SetMedalState(MEDALBOX * p_mdlbox, MedalID medal_id , u8 state);
#endif  //PM_DEBUG

