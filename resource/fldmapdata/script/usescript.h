//======================================================================
/**
 * @file	usescript.h
 * @brief	スクリプトで使用するファイルのインクルードなど
 * @author	Satoshi Nohara
 * @date	2005.10.04
 */
//======================================================================
#ifndef __USESCRIPT_H__
#define __USESCRIPT_H__

#define	__ASM_NO_DEF_
#define ASM_CPP							//pm_version.h

//======================================================================
// include
//======================================================================
//共通スクリプトデータID
#include "common_scr_def.h"
#include "shop_scr_def.h"
#include "item_get_scr_def.h"
#include "btl_util_scr_def.h"

//動作モデル表示コード
#include "../../../prog/src/field/fldmmdl_code.h"
#include "../../../prog/arc/fieldmap/fldmmdl_objcode.h"

//自機関連
#include "../../../prog/src/field/field_player_code.h"
//方向
#include "../../../prog/include/field/field_dir.h"

//スクリプトコマンドを通してスクリプトとプログラムの間で
//だけ使用する値の定義
#include "../../../prog/src/field/script_def.h"

//イベントワーク、フラグ
#include "../flagwork/flag_define.h"  //イベントフラグ
#include "../flagwork/work_define.h"  //イベントワーク

//サウンドラベル
#include "../../../prog/include/sound/wb_sound_data.sadl"

#include "../../personal/monsno_def.h"		//ポケモンナンバー
#include "../../itemconv/itempocket_def.h"  //どうぐポケットナンバー指定
#include "../../../prog/include/item/itemsym.h"

#include "../zonetable/zone_id.h"     //ゾーンID参照
#include "../warpdata/warpdata.h"     //ワープID参照

//トレーナーID
#include "../../../prog/include/tr_tool/trno_def.h"

//ポケモンのタイプ指定ID
#include "../../../prog/include/poke_tool/poketype_def.h"
//レギュレーション指定
#include "../../../prog/include/poke_tool/regulation_def.h"

//フィールド通信の種類定義
#include "../../../prog/include/gamesystem/game_comm.h"

//移動ポケモン定義
#include "../../../prog/src/field/move_pokemon_def.h"

//性別、ロムバージョンなどの定義
#include "../../../prog/include/pm_version.h"

//季節指定IDの定義
#include "../../../prog/include/gamesystem/pm_season.h"

//ワザNo
#include "../../../prog/include/waza_tool/wazano_def.h"

//フィールドカットインナンバー定義
#include "../../fld3d_ci/fldci_id_def.h"

//遊覧船トレーナー数検索タイプ定義
#include "../../../prog/src/field/pl_boat_def.h"

//ボックス呼び出しモード
#include "../../../prog/include/app/box_mode.h"

//3Dデモ指定IDの定義
#include "../../../prog/include/demo/demo3d_demoid.h"

//カメラパラメータID
#include "../scr_cam_prm/scr_cam_prm_id.h"

//ゲーム内交換ID
#include "../../fld_trade/fld_trade_list.h"

//マップ置き換えイベント指定ID
#include "../../../prog/src/field/map_replace.h"




//スクリプト内共通定義ヘッダファイル
//最初の3体タイプわけなど
#include "user_define.h"

//--------------------------------------------------------------
//  wb_null
//--------------------------------------------------------------
#if 0
//include/field
#include "../../../include/field/weather_no.h"		//天候ナンバー

#include "../../../include/system/brightness.h"		//輝度
#include "../../../include/system/window.h"			//ウィンドウ
#include "../../../include/system/wipe.h"			//ワイプ
#include "../../../include/system/timezone.h"		//時間帯

#include "../../../include/battle/battle_common.h"	//戦闘関連
#include "../../../include/battle/trno_def.h"		//トレーナーナンバー定義

#include "../../../include/application/br_sys_def.h"//GDS定義
#include "../../../include/application/wifi_p2pmatch_def.h"	//P2P対戦マッチングボード
#include "../../../include/application/imageClip/imc_itemid_define.h"	//イメージクリップ定義
#include "../../../include/application/box_mode.h"	//ボックスモード定義
#include "../../../include/application/bag_def.h"	//ポケット定義
#include "../../../include/application/seal_id.h"	//シール定義
#include "../../../include/contest/contest_def.h"	//コンテスト
#include "../../../include/itemtool/itemsym.h"		//アイテムナンバー定義
#include "../../../include/communication/comm_def.h"//通信

#include "../../../include/battle/wazano_def.h"		//ワザNo
#include "../../../include/battle/battle_server.h"	//フォルムNo

//include/savedata
#include "../../../include/savedata/fnote_mem.h"	//冒険ノート
#include "../../../include/savedata/score_def.h"	//スコア
#include "../../../include/savedata/record.h"	//スコア

//src/field
#include "../../field/ev_win.h"						//イベントウィンドウ定義
#include "../../field/comm_union_def.h"				//ユニオン定義
#include "../../field/comm_union_beacon.h"			//ユニオンルーム用ビーコン定義
#include "../../field/comm_direct_counter_def.h"	//通信ダイレクトコーナー
#include "../../field/fieldobj_code.h"				//フィールドOBJで使用するシンボル、コード宣言
#include "../../field/player_code.h"				//REQBIT
#include "../../field/board.h"						//看板
#include "../../field/honey_tree_def.h"				//蜜木ステート定義
#include "../../field/sysflag_def.h"				//システムフラグ定義
#include "../../field/syswork_def.h"				//システムワーク定義
#include "../../field/gym_def.h"					//ジム関連定義
#include "../../field/safari_train_def.h"
#include "../../field/field_trade.h"				//ゲーム内交換
#include "../../field/scr_cycling.h"				//サイクリングロード座標

#include "../../../include/system/snd_def.h"		//サウンド定義の置き換え定義
#include "../maptable/zone_id.h"					//ゾーンID定義
#include "../shopdata/fs_item_def.h"				//固定アイテムショップ定義
#include "../shopdata/fs_goods_def.h"				//固定グッズショップ定義
#include "../shopdata/fs_seal_def.h"				//固定シールショップ定義

#include "../../field/move_pokemon_def.h"			//移動ポケモン定義
#include "../../field/elevator_anm_def.h"			//エレベータ関連定義
#include "../../field/ship_demo_def.h"				//船デモ関連定義

#include "../../frontier/factory_def.h"				//ファクトリー定義
#include "../../frontier/stage_def.h"				//ステージ定義
#include "../../frontier/castle_def.h"				//キャッスル定義
#include "../../frontier/roulette_def.h"			//ルーレット定義
#include "../../frontier/fss_scene.h"				//フロンティアシーン定義
#include "../../frontier/frontier_def.h"			//フロンティア定義
#endif //wb null

//======================================================================
//	共通メッセージ関連
//======================================================================
#include "../../message/dst/script/msg_common_scr.h"


//======================================================================
//	define
//======================================================================
//#ifdef PM_DEBUG
#define SCRIPT_PM_DEBUG					//*.ev内でPM_DEBUGが無効なので
#define SCRIPT_PL_NULL					//機能無効化
//#endif

//--------------------------------------------------------------
//(エメラルドではvmstat.hに定義してあった)
//--------------------------------------------------------------
#define	LT				0				/* <	*/
#define	EQ				1				/* ==	*/
#define	GT				2				/* >	*/
#define	LE				3				/* <=	*/
#define	GE				4				/* >=	*/
#define	NE				5				/* !=	*/
#define	EQUAL			EQ
#define	LITTLER			LT
#define	GREATER			GT
#define	LT_EQ			LE
#define	GT_EQ			GE

#define	FLGON			1
#define	FLGOFF			0

#define CMPID_LT    0
#define CMPID_EQ    1
#define CMPID_GT    2
#define CMPID_LT_EQ 3
#define CMPID_GT_EQ 4
#define CMPID_NE    5
#define CMPID_OR    6
#define CMPID_AND   7
#define CMPID_TRUE  8
#define CMPID_FALSE 9
#define CMPID_GET   0xff

//nitro/type.hで定義されているがインクルードできないので自力で定義
#define	TRUE			1
#define	FALSE			0

//スクリプト通信受付の判別定義
#define SCR_COMM_ID_NOTHING	(0)	//何もなし
#define SCR_COMM_ID_BATTLE	(1)	//バトル
#define SCR_COMM_ID_UNION	(2)	//ユニオン
#define SCR_COMM_ID_WIFI	(3)	//WiFi
#define SCR_COMM_ID_RECORD	(4)	//レコード(sp_***.evに埋め込んでしまったので残している)
#define SCR_COMM_ID_CONTEST	(5)			//コンテスト
#define SCR_COMM_ID_GTC		(6)			//GTC

//--------------------------------------------------------------
//*.evの中で使用する定義
//--------------------------------------------------------------
/*
	// 話かけた方向を保持するワーク
	u16 TalkSiteWork = 0;

	// 手持ちがいっぱいでＰＣに転送された時
	u16 BoxNoWork = 0;			//何番のBOXか
	u16 PasoNoWork = 0;			//そのBOXの何番目か

	//ユニオンルームでのイベント状態制御用ワーク
	u16 UnionRoomWork = 0;
*/

/*アンノーンいせき*/
#define ANOON_SEE_NUM_1	(10)
#define ANOON_SEE_NUM_2	(26)

//メニュー用戻り値
#define EV_WIN_B_CANCEL (0xfffe)

//フェード共通速度
#define DISP_FADE_SPEED (2)

#define BAG_POCKET_NORMAL   0
#define BAG_POCKET_DRUG     1
#define BAG_POCKET_WAZA     2
#define BAG_POCKET_NUTS     3
#define BAG_POCKET_EVENT    4

//ウィンドウ位置
#define WIN_UP (SCRCMD_MSGWIN_UP)
#define WIN_DOWN (SCRCMD_MSGWIN_DOWN)
#define WIN_NONE (SCRCMD_MSGWIN_NON)

//ワイプフェードの基本の値
#define SCR_WIPE_DIV				(WIPE_DEF_DIV)
#define SCR_WIPE_SYNC				(WIPE_DEF_SYNC)

//BGウィンドウタイプ
#define TYPE_INFO (SCRCMD_BGWIN_TYPE_INFO) //掲示板
#define TYPE_TOWN (SCRCMD_BGWIN_TYPE_TOWN) //街
#define TYPE_POST (SCRCMD_BGWIN_TYPE_POST) //施設
#define TYPE_ROAD (SCRCMD_BGWIN_TYPE_ROAD) //道

//特殊ウィンドウタイプ
#define SPWIN_LETTER (SCRCMD_SPWIN_TYPE_LETTER)
#define SPWIN_BOOK (SCRCMD_SPWIN_TYPE_BOOK)

//いあいぎりエフェクト表示開始から, OBJを消すまでの待ち時間
#define EV_HIDEN_IAIGIRI_OBJDEL_WAIT (3)

#endif //__USESCRIPT_H__
