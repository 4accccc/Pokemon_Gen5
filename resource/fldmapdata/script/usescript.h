//======================================================================
/**
 * @file  usescript.h
 * @brief スクリプトで使用するファイルのインクルードなど
 * @author  Satoshi Nohara
 * @date  2005.10.04
 */
//======================================================================
#ifndef __USESCRIPT_H__
#define __USESCRIPT_H__

#define __ASM_NO_DEF_
#define ASM_CPP             //pm_version.h

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

#include "../../personal/monsno_def.h"    //ポケモンナンバー
#include "../../itemconv/itempocket_def.h"  //どうぐポケットナンバー指定
#include "../../../prog/include/item/itemsym.h"   //アイテムナンバー定義

#include "../zonetable/zone_id.h"     //ゾーンID参照
#include "../warpdata/warpdata.h"     //ワープID参照

//トレーナーID
#include "../../../prog/include/tr_tool/trno_def.h"

//ポケモンのタイプ指定ID
#include "../../../prog/include/poke_tool/poketype_def.h"
//レギュレーション指定
#include "../../../prog/include/poke_tool/regulation_def.h"
//特性指定
#include "../../../prog/include/poke_tool/tokusyu_def.h"

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

//固定アイテムショップ指定用定義
#include "../../shopdata/shop_data_index.h"

//フィールドからコールされる簡易会話
#include "../../../prog/src/field/easytalk_mode_def.h"

//天候ナンバー
#include "../../../prog/include/field/weather_no.h"

//国連部屋情報タイプ定義
#include "../../../prog/src/field/un_roominfo_def.h"

// 趣味ID
#include "../../research_radar/data/hobby_id.h"

//トライアルハウス
#include "../../../prog/src/field/trial_house_scr_def.h"
#include "../../../prog/include/savedata/th_rank_def.h"

//スクリプト内共通定義ヘッダファイル
//最初の3体タイプわけなど
#include "user_define.h"

#include  "../../../prog/include/system/timezone.h" //時間帯定義

//顔アップ定義
#include "../../../prog/src/field/fld_faceup_def.h"

//自分の性格
#include "../../../prog/include/savedata/nature_def.h"

// すれ違い調査隊関連 ( 調査依頼ID・調査アンケートID )
#include "../../../prog/include/field/research_team_def.h"

//======================================================================
//  共通メッセージ関連
//======================================================================
#include "../../message/dst/script/msg_common_scr.h"

//======================================================================
//  define
//======================================================================
//#ifdef PM_DEBUG
#define SCRIPT_PM_DEBUG         //*.ev内でPM_DEBUGが無効なので
#define SCRIPT_PL_NULL          //機能無効化
//#endif

//--------------------------------------------------------------
//--------------------------------------------------------------
#define LT        0       /* <  */
#define EQ        1       /* == */
#define GT        2       /* >  */
#define LE        3       /* <= */
#define GE        4       /* >= */
#define NE        5       /* != */
#define EQUAL     EQ
#define LITTLER     LT
#define GREATER     GT
#define LT_EQ     LE
#define GT_EQ     GE

#define FLGON     1
#define FLGOFF      0

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
#define TRUE      1
#define FALSE     0


//--------------------------------------------------------------
//*.evの中で使用する定義
//--------------------------------------------------------------

//メニュー用戻り値
#define EV_WIN_B_CANCEL (0xfffe)
#define EV_WIN_X_BREAK  (0xfffd)

//フェード共通速度
#define DISP_FADE_SPEED (2)


//いあいぎりエフェクト表示開始から, OBJを消すまでの待ち時間
#define EV_HIDEN_IAIGIRI_OBJDEL_WAIT (3)

//======================================================================
//
//    ウィンドウ関連定義
//
//======================================================================
//ウィンドウ位置（旧定義、廃止予定）
#define WIN_UP      (SCRCMD_MSGWIN_UPLEFT)
#define WIN_DOWN    (SCRCMD_MSGWIN_DOWNLEFT)
#define WIN_NONE    (SCRCMD_MSGWIN_NON)

//システム＆プレーンウィンドウ用位置定義
#define POS_UP      SCRCMD_MSGWIN_UPLEFT
#define POS_DOWN    SCRCMD_MSGWIN_DOWNLEFT

//ふきだしウィンドウ用位置定義
#define POS_UL      SCRCMD_MSGWIN_UPLEFT      /* 左上から */
#define POS_DL      SCRCMD_MSGWIN_DOWNLEFT    /* 左下から */
#define POS_UR      SCRCMD_MSGWIN_UPRIGHT     /* 右上から */
#define POS_DR      SCRCMD_MSGWIN_DOWNRIGHT   /* 左上から */

#define POS_DEFAULT SCRCMD_MSGWIN_DEFAULT     /* 自動振り分け */

//BGウィンドウタイプ
#define TYPE_INFO (SCRCMD_BGWIN_TYPE_INFO) //掲示板
#define TYPE_TOWN (SCRCMD_BGWIN_TYPE_TOWN) //街
#define TYPE_POST (SCRCMD_BGWIN_TYPE_POST) //施設
#define TYPE_ROAD (SCRCMD_BGWIN_TYPE_ROAD) //道

//特殊ウィンドウタイプ
#define SPWIN_LETTER  (SCRCMD_SPWIN_TYPE_LETTER)
#define SPWIN_BOOK    (SCRCMD_SPWIN_TYPE_BOOK)

//吹き出しウィンドウ　種類
#define BALLOONWIN_TYPE_NORMAL (0)
#define BALLOONWIN_TYPE_GIZA   (1)

#endif //__USESCRIPT_H__
