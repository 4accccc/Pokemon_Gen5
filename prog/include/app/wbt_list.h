//============================================================================
/**
 *
 *	@file		wbt_list.h
 *	@brief  WBTトーナメント表
 *	@author	tamada
 *	@date		2011.03.02
 *
 * 2011.03.02 ui_templateから派生、作成開始
 *	@note   1.ファイルをコピペします。
 *	@note   2.UITemplate / UI_TEMPLATE / ui_template をアプリ名にリネームしてください。
 *	@note   3.HEAPID_UI_DEBUG をアプリ用のヒープに書き換えてください。
 *	@note   4.不要な要素を外します
 *
 */
//============================================================================
#pragma once
//	lib
#include <gflib.h>

#include "field/wbt.h"
//system
#include "gamesystem/gamesystem.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//オーバーレイ領域の指定
FS_EXTERN_OVERLAY(wbtlist);

extern const GFL_PROC_DATA WBTListProcData;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
/// トーナメントの段階指定
//-------------------------------------
typedef enum {
  WBTLIST_STAGE_ENTRY = 0,  ///<エントリー直後
  WBTLIST_STAGE_1ST,        ///<1回戦終了後
  WBTLIST_STAGE_2ND,        ///<2回戦終了後
}WBTLIST_STAGE;

//-------------------------------------
/// トレーナーの情報
//-------------------------------------
typedef struct {
  STRBUF * name;    ///<名前文字列
  u16 mmdl_id;      ///<対応する動作モデルのID
  u8  pattern_no;   ///<上記中の正面絵パターンナンバー
  u8  sex;          ///<性別(PM_MALE/PM_FEMALE)
  u8  priority;     ///<優先度(1-5)
  u8  poketype;     ///<タイプ（include/poke_tool/poketype_def.hを参照）
  u8  is_player;    ///<プレイヤーかどうかのフラグ
}WBTLIST_TRAINER;
//-------------------------------------
//-------------------------------------
typedef struct {
  BOOL upside_win;
  u8 msg_type;
}WBTLIST_RESULT;

//-------------------------------------
///	PROCに渡す引数
//=====================================
typedef struct
{	
	GAMESYS_WORK				*gamesys;	//[in]ゲームシステム
  WBTLIST_STAGE stage;
  WBTLIST_TRAINER trainer[ WBT_TRAINER_NUM ];
  WBTLIST_RESULT  first_stage[4];
  WBTLIST_RESULT  second_stage[2];
  STRBUF * tournament_name;
  STRBUF * tournament_rule;
#ifdef  PM_DEBUG
  int debug_value;
#endif
  BOOL call_fade_flag;    //このPROC内部では使用しない。呼び出し側で設定・参照している
}WBTLIST_PARAM;

