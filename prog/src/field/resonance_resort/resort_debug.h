//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_debug.h
 *	@brief  ギミック内デバッグ処理
 *	@author	Toru=Nagihashi
 *	@date		2011.09.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#ifdef PM_DEBUG

extern void RESORT_DEBUG_FLAGWORK_Init( void );

//=============================================================================
/**
 *					デバッグフラグ
*/
//=============================================================================
typedef enum
{
  RESORT_DEBUG_FLAG_FORCE_EXCELLENT,    //強制◎
  RESORT_DEBUG_FLAG_EXP_9999,           //経験値9999
  RESORT_DEBUG_FLAG_FORCE_CELEB,        //強制セレブ
  RESORT_DEBUG_FLAG_FORCE_PRESENT,      //強制おみやげ
  RESORT_DEBUG_FLAG_FORCE_FEVER,        //強制フィーバー
  RESORT_DEBUG_FLAG_FORCE_INIT_COMBO,   //初回コンボ必ず成功
  RESORT_DEBUG_FLAG_FORCE_COMBO_MAX,    //コンボ必ず最大連鎖
  RESORT_DEBUG_FLAG_PRESENT_NONE,       //おみやげなし
  RESORT_DEBUG_FLAG_FORCE_SPOT_WORK_ON,  //強制行きたいお店使う？
  RESORT_DEBUG_FLAG_FORCE_RND_WORK_ON,   //乱数操作使う？
  RESORT_DEBUG_FLAG_FORCE_ITEM_WORK_ON, //アイテム固定使う？
  RESORT_DEBUG_FLAG_SEND_DUMMY, //ダミーデータ送信？

  RESORT_DEBUG_FLAG_MAX
}RESORT_DEBUG_FLAG;

extern void RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG flag, BOOL is_on );
extern BOOL RESORT_DEBUG_FLAG_Get( RESORT_DEBUG_FLAG flag );


//=============================================================================
/**
 *					デバッグワーク
*/
//=============================================================================
#define RESORT_DEBUG_WORK_DEFAULT (0)

typedef enum
{
  RESORT_DEBUG_WORK_FORCE_SPOT,     //強制行きたいお店
  RESORT_DEBUG_WORK_FORCE_RND,      //乱数操作
  RESORT_DEBUG_WORK_FORCE_ITEM,     //アイテム固定

  RESORT_DEBUG_WORK_COMBO_LOG_00,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_01,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_02,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_03,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_04,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_05,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_06,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_07,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_00,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_01,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_02,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_03,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_04,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_05,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_06,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_PER_07,     //こんぼろぐ
  RESORT_DEBUG_WORK_COMBO_LOG_CNT,     //コンボカウント
  RESORT_DEBUG_WORK_COMBO_LOG_TBL,     //コンボテーブル

  RESORT_DEBUG_WORK_MAX
}RESORT_DEBUG_WORK;

extern void RESORT_DEBUG_WORK_Set( RESORT_DEBUG_WORK wk, u32 value );
extern u32 RESORT_DEBUG_WORK_Get( RESORT_DEBUG_WORK wk );

#endif  //PM_DEBUG
