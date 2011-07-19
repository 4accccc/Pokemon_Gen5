//============================================================================
/**
 *
 *	@file		win_record.h
 *	@brief  エントリーチェッカー(優勝記録)
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
FS_EXTERN_OVERLAY( entry_checker );

extern const GFL_PROC_DATA WinRecordProcData;

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
typedef enum {
	WIN_RECORD_STATUS_LV1 = 0,	///<シナリオクリア前
	WIN_RECORD_STATUS_LV2,		///<シナリオクリア後
	WIN_RECORD_STATUS_LV3,		///<シナリオクリア後&チャンピオンバトル出現後
}WIN_RECORD_STATUS;

//
// エントリーチェッカー(優勝記録)
typedef struct{
//	GAMESYS_WORK		*gamesys;		///<システム
	//
	WIN_RECORD_STATUS	status;			///<クリア状態
	u16 *				result_work;	///<結果 (1:フリー移動に戻る、0:パソコン画面に戻る)
	//
	const WBTSAVE		*wbtsave;		///<セーブデータ：優勝回数参照用(include/savedata/wbt_save.h)
	const MYSTATUS		*my_status;		///<セーブデータ：自分情報（include/savedata/mystatus.h)
}WIN_RECORD_PARAM;

