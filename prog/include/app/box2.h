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

#include "savedata/battle_box_save.h"
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

#define	BOX_RET_SEL_NONE		( 0xff )		// 何も選択されなかった

// 外部設定データ
typedef struct {
	// [in]
  GAMEDATA * gamedata;			  // ゲームデータ ( バッグ/ステータスのYボタン登録に使用 )
	BOX_MANAGER * sv_box;				// ボックスセーブデータ
	POKEPARTY * pokeparty;			// 手持ちモケモン
	MYITEM_PTR	myitem;					// 所持アイテム（バッグで使用）
	MYSTATUS * mystatus;				// プレイヤーデータ（バッグで使用）
	BOOL	bbRockFlg;						// バトルボックスロックフラグ

  CONFIG * cfg;								// コンフィグデータ
  u32	zknMode;								// 図鑑ナンバー表示モード

	BOX_MODE	callMode;					// 呼び出しモード

	// [out]
	u16 retMode;								// 終了モード
	u8	retTray;								// 終了時に開いていたトレイ（寝かせる用）
	u8	retPoke;								// 終了時に選択された位置（寝かせる用）

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
