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
///WBT用レギュレーションデータ
//------------------------------------------------------------------
typedef struct _WBT_REGLATION {
  ///標準のレギュレーションを内包する
  REGULATION base_reg;
}WBTREG;

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _WBTTRAINER{
  u8 wbt_type:3; ///<WBT_TRTYPE
  u8 sex:1;
  u8 pri:4;
  u8 poketype;
  u8 tr_id;  ///<トレーナーID（trtypeにより意味が変わる）
};
//------------------------------------------------------------------
//------------------------------------------------------------------
struct _WBTRESULT {
  u8 upside_win;
  u8 msg_id;
};
//------------------------------------------------------------------
/**
 * @brief WBT中に保持する進行データ
 */
//------------------------------------------------------------------
struct _WBT_SYSTEM {
  HEAPID heapID;
  ///トーナメントの種類
  WBTTYPE type;
  ///バトルルール（シングル、ダブル、トリプル、ローテーション）
  WBTRULE rule;
  ///WBTの進行段階
  WBTSTAGE stage;

  ///選択したダウンロードデータ
  u8 select_dl_data;

  u64 seed64;
  GFL_STD_RandContext rand_context;

  ///参戦トレーナー
  WBTTRAINER wbt_trainer[ WBT_TRAINER_NUM ];

  ///試合結果
  WBTRESULT wbt_result[ WBT_GAME_TOTAL ];

  ///WBT用レギュレーション
  WBTREG wbtreg;

  ///ポケモン選択関連
    u8 use_btlbox;  ///<バトルボックスを使用するかどうか？フラグ
    POKEPARTY *btl_box_party; //バトルボックス用POKEPARTY
    
    u8 pokelist_select_num[6]; ///<ポケモン選択で取得した手持ちNo
    u16 pokelist_result_select; ///<ポケモンリスト戻り値
    u16 pokelist_return_mode; ///<ポケモンリスト選択結果
  
  BATTLE_SETUP_PARAM *btl_setup_param;
//  BTL_REC_SEL_PARAM btl_rec_sel_param;

  ///スクリプトとプログラムのやりとりのためのアドレスを保持するポインタ
  u16 * scr_answer_work;
};

#if 0
#include "field/location.h"

#include "net_app/comm_entry_menu.h"

#include "savedata/bsubway_savedata.h"
#include "savedata/bsubway_savedata_def.h"
#include "battle/bsubway_battle_data.h"
#include "bsubway_scr_def.h"

#include "event_field_proclink.h"
#include "demo/comm_btl_demo.h"
#include "net_app/irc_match.h"
#include "net_app/btl_rec_sel.h"

//--------------------------------------------------------------
/// バトルサブウェイ　スクリプトワーク
//--------------------------------------------------------------
struct _TAG_BSUBWAY_SCRWORK
{
  int magicNo;  ///<マジックナンバー
  int heapID;  ///<ヒープ
  
  u8 member_num;  ///<メンバー数
  u8 play_mode;  ///<プレイデータ
  u8 my_sex;      ///<自分の性別
  u8 pare_sex;    ///<自分とパートナーの性別
  
  u16         clear_f:1;  ///<クリアフラグ
  u16          boss_f:2;  ///<ボスと対戦したフラグ
  u16        retire_f:1;  ///<リタイアフラグ
  u16         prize_f:1;  ///<ご褒美フラグ
  u16         partner:3;  ///<パートナーNo
  u16      comm_sio_f:1; ///<通信中フラグ
  u16      comm_irc_f:1; ///<赤外線通信フラグ
  u16  btlrec_exist_f:2; ///<戦闘録画存在フラグ
  u16 btlrec_finish_f:1; ///<戦闘録画済みフラグ
  u16     padding_bit:3; ///<余り
  u16 now_renshou; ///<連勝数
  
  u16 btlrec_btlnum; ///<戦闘録画に設定する戦闘数
  u16 comm_send_command; //通信　送信コマンド
  
  BtlResult comm_btl_result; ///< 勝敗結果
  u16 pare_stage_no; ///<通信マルチパートナーの周回数
  
  u16 pare_poke[2];  ///<通信マルチパートナーが持つポケモンNo
  
  u8  member[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンの手持ちNo
  u16  mem_poke[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンのmonsno
  u16  mem_item[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンのアイテムno
  u16  trainer[BSUBWAY_STOCK_TRAINER_MAX];  ///<対戦トレーナーNo
  
  LOCATION now_location; //現在のロケーション
  
  GAMEDATA *gdata; ///<GAMEDATA
  BSUBWAY_PLAYDATA *playData;  ///<セーブデータへのポインタ
  BSUBWAY_SCOREDATA *scoreData;  ///<セーブデータへのポインタ
  BSUBWAY_WIFI_DATA *wifiData; ///<セーブデータへのポインタ
  
  u8 pokelist_select_num[6]; ///<ポケモン選択で取得した手持ちNo
  u16 pokelist_result_select; ///<ポケモンリスト戻り値
  u16 pokelist_return_mode;
  
  //対戦相手データ格納場所
  BSUBWAY_PARTNER_DATA  tr_data[2];
 	
  //パートナー抽選データ格納場所
	BSUBWAY_PARTNER_DATA five_data[BSWAY_PARTNER_NUM];
	//パートナー抽選ポケモンパラメータ格納場所
	BSUBWAY_PAREPOKE_PARAM five_poke[BSWAY_PARTNER_NUM];
	//5人衆抽選ポケモンアイテムfixフラグ
	u8 five_item[BSWAY_PARTNER_NUM]; 
  
  u16  send_buf[35];  //通信用データバッファ
  u16  recv_buf[35];  //通信用データバッファ
  
  COMM_ENTRY_BEACON commBeaconData; //ビーコンデータ
  COMM_ENTRY_MENU_PTR pCommEntryMenu; //通信マルチ参加メニュー
  u16 *pCommEntryResult; //通信マルチ参加メニュー結果格納先
  u8  comm_receive_count; //受信カウント
  u8  comm_mode; //通信モード
  u8  first_btl_flag;    //初戦をおこなったかフラグ
  u8  comm_timing_no; //通信タイミング番号
  u16 comm_check_work;
  u16 ret_work_id; ///<スクリプトコマンド戻り値
  
  MYSTATUS mystatus_fr; //通信パートナーのMYSTATUS
  COMM_BTL_DEMO_PARAM comm_btl_demo_param;
  IRC_MATCH_WORK irc_match;
  
  POKEPARTY *btl_box_party; //バトルボックス用POKEPARTY
  
  BATTLE_SETUP_PARAM *btl_setup_param;
  BTL_REC_SEL_PARAM btl_rec_sel_param;
  u16 btlrec_save_work[2];
};
#endif

