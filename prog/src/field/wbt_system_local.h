//============================================================================================
/**
 * @file  wbt_system_local.h
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.03.30
 */
//============================================================================================
#pragma once

#include "gflib.h"
#include "wbt_system.h"
#include "poke_tool/poke_tool.h"
#include "gamesystem/btl_setup.h"
#include "savedata/wbt_download.h"
#include "wbt_trlist_struct.h"    //WBT_TRDATA
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
  WBT_GAME_1ST_1 = 0,
  WBT_GAME_1ST_2,
  WBT_GAME_1ST_3,
  WBT_GAME_1ST_4,
  WBT_GAME_2ND_1,
  WBT_GAME_2ND_2,
  WBT_GAME_FINAL,

  WBT_GAME_TOTAL,
}WBT_GAME_ID;

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_COMMENT_A = 0,
  WBT_COMMENT_B,
  WBT_COMMENT_C,
  WBT_COMMENT_D,
  WBT_COMMENT_E,

  WBT_COMMENT_TOTAL,
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
///WBT用レギュレーションデータ
//------------------------------------------------------------------
typedef struct _WBT_REGLATION {
  ///標準のレギュレーションを内包する
  REGULATION base_reg;
  REGULATIONEX ex_reg;
}WBTREG;

//------------------------------------------------------------------
/// WBT用トレーナーデータ
//------------------------------------------------------------------
struct _WBTTRAINER{
  u8 wbt_tr_type:3; ///<WBT_TRTYPE
  u8 sex:1;         ///<性別
  u8 pri:3;         ///<優先順位（対戦結果やコメント判定に使用）
  u8 has_ace:1;     ///<切り札ポケモンを持っているか？

  u8 poketype;      ///<手持ちパーティのポケモンタイプ
  u8 tr_id;         ///<トレーナーID（wbt_tr_typeにより意味が変わる）
  u8 pow_rnd:6;     ///<パワー乱数（ロムトレーナーのみ）
  u8 dl_tr_id:2;    ///<ダウンロードデータのインデックス（DLトレーナーのみ）

  u16 mmdl_id;      ///<動作モデルコード
  u16 btl_tr_type:12;  ///<トレーナータイプ（tr_tool/trtype_def.h)
  u16 ai_bit:4;     ///<ai指定
  STRBUF * name;    ///<名前
  STRBUF * title;   ///<肩書
};

//------------------------------------------------------------------
/// WBT結果データ
//------------------------------------------------------------------
struct _WBTRESULT {
  u8 upside_win;
  u8 msg_id;
};

//------------------------------------------------------------------
///  トーナメント出場者の決定に必要な引数定義
//------------------------------------------------------------------
struct _WBT_MATCH_CONDITION{
  HEAPID heapID;

  WBTCUP wbtcup;
  
  PokeType poketype;              ///<タイプトーナメント用：タイプ指定

  u16 rom_trainer_num;
  const WBT_TRDATA * rom_trainer_data;

  u64 seed64;

  const WBTDL_MATCH * wbtdl_match;
};
//------------------------------------------------------------------
///
//------------------------------------------------------------------
typedef struct _WBT_TRCODE{
  union {
    u16 code;
    struct {
      u8 tr_id;
      u8 type;
    };
  };
};
//------------------------------------------------------------------
/**
 * @brief 事前定義済みトレーナーデータを扱うための構造体
 *
 * @note  実際のデータ部分は可変長配列となるため、sizeofの扱い等には注意
 */
//------------------------------------------------------------------
struct _WBTROM_TRDATA{
  u32 num;
  WBT_TRDATA data[1];
};

//------------------------------------------------------------------
/**
 * @brief WBT中に保持する進行データ
 */
//------------------------------------------------------------------
struct _WBT_SYSTEM {
  HEAPID heapID;

  ///チュートリアルモードかどうか
  BOOL isTutorial;

  ///ROM上のトレーナーデータ
  const WBTROM_TRDATA * rom_trdata;

  ///トーナメントの種類
  WBTCUP wbtcup;
  ///バトルルール（シングル、ダブル、トリプル、ローテーション）
  WBTRULE rule;
  ///話しかけた受付
  WBTRECEPT reception_id;

  ///WBTの進行段階
  WBTSTAGE stage;

  ///タイプトーナメントの場合のタイプ
  PokeType poketype;
  ///選択したダウンロードデータ(1org,0の場合存在しない）
  u8 select_dl_data;

  u8 audience_type;     ///<観客の種類

  u64 seed64;
  GFL_STD_RandContext rand_context;

  ///参戦トレーナー
  WBTTRAINER wbt_trainer[ WBT_TRAINER_NUM ];

  ///試合結果
  WBTRESULT wbt_result[ WBT_GAME_TOTAL ];

  ///WBT用レギュレーション
  WBTREG wbtreg;

  BOOL dldata_loaded;
  ///WBTダウンロードデータ
  WBTDL_DATA wbtdl_data;

  ///ポケモン選択関連

    u8 use_btlbox;  ///<バトルボックスを使用するかどうか？フラグ
    POKEPARTY *btl_box_party; //バトルボックス用POKEPARTY
    
    u8 pokelist_select_num[6]; ///<ポケモン選択で取得した手持ちNo
    u16 pokelist_result_select; ///<ポケモンリスト戻り値
    u16 pokelist_return_mode; ///<ポケモンリスト選択結果
  
    POKEPARTY * my_party;       ///<参戦する自分の手持ち
    POKEPARTY * npc_party;      ///<参戦する相手の手持ち
    POKEPARTY * sp_party;       ///<特殊トーナメント用手持ち

    u8 mix_pos[ WBTMIX_TARGET_NUM ];  ///<ミックストーナメントでの選択位置

    WBT_MIXWIN * MixInfoWinWork;
  //戦闘呼び出し関連
  BATTLE_SETUP_PARAM *btl_setup_param;
  STRBUF * win_msg;     ///<バトル内での勝利時メッセージ
  STRBUF * lose_msg;    ///<バトル内での敗北時メッセージ

  u8 survival_count[ WBT_BATTLE_NUM ];   ///<対戦後の手持ちの残り数

  ///スクリプトとプログラムのやりとりのためのアドレスを保持するポインタ
  u16 * scr_answer_work;

  WBTRENT_TYPE rental_type; ///<レンタル選別の種類
};


