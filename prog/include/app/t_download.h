//============================================================================
/**
 *
 *	@file		t_download.h
 *	@brief  トーナメントダウンロード画面
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
#include "savedata/wbt_download.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//オーバーレイ領域の指定
FS_EXTERN_OVERLAY( entry_checker );

extern const GFL_PROC_DATA TDownloadProcData;

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
/// 状態
//-------------------------------------
enum{
	T_DOWNLOAD_CURPOS_OFFSET = 1,	// オフセット
};
	
typedef enum {
	T_DOWNLOAD_STATUS_SAVE = 0,		///<セーブ
	T_DOWNLOAD_STATUS_SELECT,		///<選択
	T_DOWNLOAD_STATUS_JOIN,			///<参加
	T_DOWNLOAD_STATUS_INFO,			///<詳細
}T_DOWNLOAD_STATUS;
//
// トーナメントダウンロード
typedef struct{
	GAMESYS_WORK		*gamesys;   ///<ゲームシステム

  WBTDL_TOTAL_DATA  *data;          ///<wifiからダウンロードしたデータ
	//
	T_DOWNLOAD_STATUS	status;     ///<呼び出しモード
	u16 *				result_work;	///<結果 (1:フリー移動に戻る、0:パソコン画面に戻る)
	u16 *				curpos;			///<カーソル位置(0:なし、値：T_DOWNLOAD_CURPOS_OFFSETはじまり)
}T_DOWNLOAD_PARAM;


