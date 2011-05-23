//============================================================================
/**
 *
 *	@file		pkwd_count.h
 *	@brief  ポケウッドカウント
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
#include "savedata/wbt_save.h"
#include "savedata/mystatus.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//オーバーレイ領域の指定
FS_EXTERN_OVERLAY( pkwd_announce );

extern const GFL_PROC_DATA PkwdCountProcData;

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
/// クリア状態
//-------------------------------------

// PKWD_COUNT
typedef struct{
	GAMESYS_WORK		*gamesys;		///<システム
	//
//	WIN_RECORD_STATUS	status;			///<クリア状態
//	u16 *				result_work;	///<結果 (1:フリー移動に戻る、0:パソコン画面に戻る)
	//
//	const WBTSAVE		*wbtsave;		///<セーブデータ：優勝回数参照用(include/savedata/wbt_save.h)
//	const MYSTATUS		*my_status;		///<セーブデータ：自分情報（include/savedata/mystatus.h)
}PKWD_COUNT_PARAM;

