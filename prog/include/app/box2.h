//============================================================================================
/**
 * @file		box2.h
 * @brief		ボックス画面
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2
 */
//============================================================================================
#pragma	once

#include "savedata/box_savedata.h"
#include "savedata/myitem_savedata.h"
#include "savedata/config.h"
#include "gamesystem/game_data.h"
#include "poke_tool/pokeparty.h"
#include "box_mode.h"
/*
#include "savedata/savedata_def.h"
//#include "system/procsys.h"
#include "system/keytouch_status.h"
*/

//============================================================================================
//	定数定義
//============================================================================================

typedef int BOX_MODE;

// 外部設定データ
typedef struct {
	// [in]
  GAMEDATA * gamedata;			  // ゲームデータ ( バッグ/ステータスのYボタン登録に使用 )
	BOX_MANAGER * sv_box;				// ボックスセーブデータ
	POKEPARTY * pokeparty;			// 手持ちモケモン
	MYITEM_PTR	myitem;					// 所持アイテム（バッグで使用）
	MYSTATUS * mystatus;				// プレイヤーデータ（バッグで使用）

  CONFIG * cfg;								// コンフィグデータ
  u32	zknMode;								// 図鑑ナンバー表示モード

	BOX_MODE	callMode;					// 呼び出しモード

/*
	// [in] 
	SAVEDATA * savedata;
	KEYTOUCH_STATUS * kt_status;	// キータッチステータス

	// [out]
//	BOOL	modifiedFlag;	///< ボックス内容が変更されたらTRUEが入る
*/

}BOX2_GFL_PROC_PARAM;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//============================================================================================
//	グローバル
//============================================================================================

// ボックスPROCデータ
extern const GFL_PROC_DATA BOX2_ProcData;
FS_EXTERN_OVERLAY(box);
