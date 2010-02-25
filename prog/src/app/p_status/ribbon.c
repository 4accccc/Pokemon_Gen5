//============================================================================================
/**
 * @file  ribbon.c
 * @brief リボンデータ関連
 * @author  Hiroyuki Nakamura
 * @date  2006.04.18
 */
//============================================================================================
#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "p_status_gra.naix"
#include "msg/msg_ribbon.h"

#include "ribbon.h"


//============================================================================================
//  定数定義
//============================================================================================
typedef struct {
  u32 pp_id;    // PokeParaGetの取得ID
  u16 graphic;  // グラフィックID
  u16 pal_num;  // パレット番号
  u16 name;   // リボン名
  u16 info;   // リボン説明
  u8  category; //リボン種類
}RIBBON_TABLE;


//============================================================================================
//  グローバル変数
//============================================================================================
// リボンデータ
static const RIBBON_TABLE RibbonTable[] =
{
  { ID_PARA_champ_ribbon, NARC_p_status_gra_ribbon00_NCGR,  0,  mes_ribbon_name_000,  mes_ribbon_text_000,  RIBBON_CATEGORY_CHAMP,  },  //チャンプリボン

  { ID_PARA_stylemedal_normal,  NARC_p_status_gra_ribbon01_NCGR,  0,  mes_ribbon_name_001,  mes_ribbon_text_001,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(ノーマル)AGBコンテスト
  { ID_PARA_stylemedal_super, NARC_p_status_gra_ribbon02_NCGR,  0,  mes_ribbon_name_002,  mes_ribbon_text_002,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(スーパー)AGBコンテスト
  { ID_PARA_stylemedal_hyper, NARC_p_status_gra_ribbon03_NCGR,  0,  mes_ribbon_name_003,  mes_ribbon_text_003,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(ハイパー)AGBコンテスト
  { ID_PARA_stylemedal_master,  NARC_p_status_gra_ribbon04_NCGR,  0,  mes_ribbon_name_004,  mes_ribbon_text_004,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(マスター)AGBコンテスト

  { ID_PARA_beautifulmedal_normal,  NARC_p_status_gra_ribbon01_NCGR,  1,  mes_ribbon_name_005,  mes_ribbon_text_005,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(ノーマル)AGBコンテスト
  { ID_PARA_beautifulmedal_super, NARC_p_status_gra_ribbon02_NCGR,  1,  mes_ribbon_name_006,  mes_ribbon_text_006,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(スーパー)AGBコンテスト
  { ID_PARA_beautifulmedal_hyper, NARC_p_status_gra_ribbon03_NCGR,  1,  mes_ribbon_name_007,  mes_ribbon_text_007,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(ハイパー)AGBコンテスト
  { ID_PARA_beautifulmedal_master,  NARC_p_status_gra_ribbon04_NCGR,  1,  mes_ribbon_name_008,  mes_ribbon_text_008,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(マスター)AGBコンテスト

  { ID_PARA_cutemedal_normal, NARC_p_status_gra_ribbon01_NCGR,  2,  mes_ribbon_name_009,  mes_ribbon_text_009,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(ノーマル)AGBコンテスト
  { ID_PARA_cutemedal_super,  NARC_p_status_gra_ribbon02_NCGR,  2,  mes_ribbon_name_010,  mes_ribbon_text_010,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(スーパー)AGBコンテスト
  { ID_PARA_cutemedal_hyper,  NARC_p_status_gra_ribbon03_NCGR,  2,  mes_ribbon_name_011,  mes_ribbon_text_011,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(ハイパー)AGBコンテスト
  { ID_PARA_cutemedal_master, NARC_p_status_gra_ribbon04_NCGR,  2,  mes_ribbon_name_012,  mes_ribbon_text_012,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(マスター)AGBコンテスト

  { ID_PARA_clevermedal_normal, NARC_p_status_gra_ribbon01_NCGR,  3,  mes_ribbon_name_013,  mes_ribbon_text_013,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(ノーマル)AGBコンテスト
  { ID_PARA_clevermedal_super,  NARC_p_status_gra_ribbon02_NCGR,  3,  mes_ribbon_name_014,  mes_ribbon_text_014,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(スーパー)AGBコンテスト
  { ID_PARA_clevermedal_hyper,  NARC_p_status_gra_ribbon03_NCGR,  3,  mes_ribbon_name_015,  mes_ribbon_text_015,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(ハイパー)AGBコンテスト
  { ID_PARA_clevermedal_master, NARC_p_status_gra_ribbon04_NCGR,  3,  mes_ribbon_name_016,  mes_ribbon_text_016,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(マスター)AGBコンテスト

  { ID_PARA_strongmedal_normal, NARC_p_status_gra_ribbon01_NCGR,  4,  mes_ribbon_name_017,  mes_ribbon_text_017,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(ノーマル)AGBコンテスト
  { ID_PARA_strongmedal_super,  NARC_p_status_gra_ribbon02_NCGR,  4,  mes_ribbon_name_018,  mes_ribbon_text_018,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(スーパー)AGBコンテスト
  { ID_PARA_strongmedal_hyper,  NARC_p_status_gra_ribbon03_NCGR,  4,  mes_ribbon_name_019,  mes_ribbon_text_019,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(ハイパー)AGBコンテスト
  { ID_PARA_strongmedal_master, NARC_p_status_gra_ribbon04_NCGR,  4,  mes_ribbon_name_020,  mes_ribbon_text_020,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(マスター)AGBコンテスト

  { ID_PARA_winning_ribbon, NARC_p_status_gra_ribbon06_NCGR,  0,  mes_ribbon_name_021,  mes_ribbon_text_021,  RIBBON_CATEGORY_TOWER,  },  //ウィニングリボン
  { ID_PARA_victory_ribbon, NARC_p_status_gra_ribbon05_NCGR,  0,  mes_ribbon_name_022,  mes_ribbon_text_022,  RIBBON_CATEGORY_TOWER,  },  //ビクトリーリボン
  { ID_PARA_bromide_ribbon, NARC_p_status_gra_ribbon07_NCGR,  1,  mes_ribbon_name_023,  mes_ribbon_text_023,  RIBBON_CATEGORY_MEMORY,  },  //ブロマイドリボン

  { ID_PARA_ganba_ribbon, NARC_p_status_gra_ribbon08_NCGR,  2,  mes_ribbon_name_024,  mes_ribbon_text_024,  RIBBON_CATEGORY_MEMORY,  },  //がんばリボン
  { ID_PARA_marine_ribbon,  NARC_p_status_gra_ribbon09_NCGR,  1,  mes_ribbon_name_025,  mes_ribbon_name_025,  RIBBON_CATEGORY_PRESENT,  },  //マリンリボン
  { ID_PARA_land_ribbon,  NARC_p_status_gra_ribbon09_NCGR,  3,  mes_ribbon_name_026,  mes_ribbon_name_026,  RIBBON_CATEGORY_PRESENT,  },  //ランドリボン
  { ID_PARA_sky_ribbon, NARC_p_status_gra_ribbon09_NCGR,  4,  mes_ribbon_name_027,  mes_ribbon_name_027,  RIBBON_CATEGORY_PRESENT,  },  //スカイリボン

  { ID_PARA_country_ribbon, NARC_p_status_gra_ribbon10_NCGR,  3,  mes_ribbon_name_028,  mes_ribbon_haihu_text_032,  RIBBON_CATEGORY_MEMORY,  },  //MSG_SP_RIBBON+3
  { ID_PARA_national_ribbon,  NARC_p_status_gra_ribbon10_NCGR,  4,  mes_ribbon_name_029,  mes_ribbon_haihu_text_044,  RIBBON_CATEGORY_MEMORY,  },  //MSG_SP_RIBBON+4
  { ID_PARA_earth_ribbon, NARC_p_status_gra_ribbon11_NCGR,  0,  mes_ribbon_name_030,  mes_ribbon_haihu_text_045,  RIBBON_CATEGORY_MEMORY,  },  //MSG_SP_RIBBON+5
  { ID_PARA_world_ribbon, NARC_p_status_gra_ribbon11_NCGR,  1,  mes_ribbon_name_031,  mes_ribbon_haihu_text_032,  RIBBON_CATEGORY_MEMORY,  },  //MSG_SP_RIBBON+6

  { ID_PARA_sinou_champ_ribbon, NARC_p_status_gra_ribbon32_NCGR,  0,  mes_ribbon_name_032,  mes_ribbon_text_032,  RIBBON_CATEGORY_CHAMP,  },  //シンオウチャンプリボン

  { ID_PARA_trial_stylemedal_normal,  NARC_p_status_gra_ribbon33_NCGR,  0,  mes_ribbon_name_033,  mes_ribbon_text_033,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(ノーマル)トライアル
  { ID_PARA_trial_stylemedal_super, NARC_p_status_gra_ribbon34_NCGR,  0,  mes_ribbon_name_034,  mes_ribbon_text_034,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(スーパー)トライアル
  { ID_PARA_trial_stylemedal_hyper, NARC_p_status_gra_ribbon35_NCGR,  0,  mes_ribbon_name_035,  mes_ribbon_text_035,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(ハイパー)トライアル
  { ID_PARA_trial_stylemedal_master,  NARC_p_status_gra_ribbon36_NCGR,  0,  mes_ribbon_name_036,  mes_ribbon_text_036,  RIBBON_CATEGORY_CONTEST,  },  //かっこよさ勲章(マスター)トライアル

  { ID_PARA_trial_beautifulmedal_normal,  NARC_p_status_gra_ribbon33_NCGR,  1,  mes_ribbon_name_037,  mes_ribbon_text_037,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(ノーマル)トライアル
  { ID_PARA_trial_beautifulmedal_super, NARC_p_status_gra_ribbon34_NCGR,  1,  mes_ribbon_name_038,  mes_ribbon_text_038,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(スーパー)トライアル
  { ID_PARA_trial_beautifulmedal_hyper, NARC_p_status_gra_ribbon35_NCGR,  1,  mes_ribbon_name_039,  mes_ribbon_text_039,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(ハイパー)トライアル
  { ID_PARA_trial_beautifulmedal_master,  NARC_p_status_gra_ribbon36_NCGR,  1,  mes_ribbon_name_040,  mes_ribbon_text_040,  RIBBON_CATEGORY_CONTEST,  },  //うつくしさ勲章(マスター)トライアル

  { ID_PARA_trial_cutemedal_normal, NARC_p_status_gra_ribbon33_NCGR,  2,  mes_ribbon_name_041,  mes_ribbon_text_041,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(ノーマル)トライアル
  { ID_PARA_trial_cutemedal_super,  NARC_p_status_gra_ribbon34_NCGR,  2,  mes_ribbon_name_042,  mes_ribbon_text_042,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(スーパー)トライアル
  { ID_PARA_trial_cutemedal_hyper,  NARC_p_status_gra_ribbon35_NCGR,  2,  mes_ribbon_name_043,  mes_ribbon_text_043,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(ハイパー)トライアル
  { ID_PARA_trial_cutemedal_master, NARC_p_status_gra_ribbon36_NCGR,  2,  mes_ribbon_name_044,  mes_ribbon_text_044,  RIBBON_CATEGORY_CONTEST,  },  //かわいさ勲章(マスター)トライアル

  { ID_PARA_trial_clevermedal_normal, NARC_p_status_gra_ribbon33_NCGR,  3,  mes_ribbon_name_045,  mes_ribbon_text_045,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(ノーマル)トライアル
  { ID_PARA_trial_clevermedal_super,  NARC_p_status_gra_ribbon34_NCGR,  3,  mes_ribbon_name_046,  mes_ribbon_text_046,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(スーパー)トライアル
  { ID_PARA_trial_clevermedal_hyper,  NARC_p_status_gra_ribbon35_NCGR,  3,  mes_ribbon_name_047,  mes_ribbon_text_047,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(ハイパー)トライアル
  { ID_PARA_trial_clevermedal_master, NARC_p_status_gra_ribbon36_NCGR,  3,  mes_ribbon_name_048,  mes_ribbon_text_048,  RIBBON_CATEGORY_CONTEST,  },  //かしこさ勲章(マスター)トライアル

  { ID_PARA_trial_strongmedal_normal, NARC_p_status_gra_ribbon33_NCGR,  4,  mes_ribbon_name_049,  mes_ribbon_text_049,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(ノーマル)トライアル
  { ID_PARA_trial_strongmedal_super,  NARC_p_status_gra_ribbon34_NCGR,  4,  mes_ribbon_name_050,  mes_ribbon_text_050,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(スーパー)トライアル
  { ID_PARA_trial_strongmedal_hyper,  NARC_p_status_gra_ribbon35_NCGR,  4,  mes_ribbon_name_051,  mes_ribbon_text_051,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(ハイパー)トライアル
  { ID_PARA_trial_strongmedal_master, NARC_p_status_gra_ribbon36_NCGR,  4,  mes_ribbon_name_052,  mes_ribbon_text_052,  RIBBON_CATEGORY_CONTEST,  },  //たくましさ勲章(マスター)トライアル

  { ID_PARA_sinou_battle_tower_ttwin_first, NARC_p_status_gra_ribbon53_NCGR,  0,  mes_ribbon_name_053,  mes_ribbon_text_053,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワータワータイクーン勝利1回目
  { ID_PARA_sinou_battle_tower_ttwin_second,  NARC_p_status_gra_ribbon54_NCGR,  0,  mes_ribbon_name_054,  mes_ribbon_text_054,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワータワータイクーン勝利2回目
  { ID_PARA_sinou_battle_tower_2vs2_win50,  NARC_p_status_gra_ribbon55_NCGR,  0,  mes_ribbon_name_055,  mes_ribbon_text_055,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワータワーダブル50連勝
  { ID_PARA_sinou_battle_tower_aimulti_win50, NARC_p_status_gra_ribbon56_NCGR,  0,  mes_ribbon_name_056,  mes_ribbon_text_056,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワータワーAIマルチ50連勝
  { ID_PARA_sinou_battle_tower_siomulti_win50,  NARC_p_status_gra_ribbon57_NCGR,  0,  mes_ribbon_name_057,  mes_ribbon_text_057,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワータワー通信マルチ50連勝
  { ID_PARA_sinou_battle_tower_wifi_rank5,  NARC_p_status_gra_ribbon58_NCGR,  0,  mes_ribbon_name_058,  mes_ribbon_text_058,  RIBBON_CATEGORY_TOWER,  },  //シンオウバトルタワーWifiランク５入り

  { ID_PARA_sinou_syakki_ribbon,  NARC_p_status_gra_ribbon59_NCGR,  2,  mes_ribbon_name_059,  mes_ribbon_text_059,  RIBBON_CATEGORY_MEMORY,  },  //シンオウしゃっきリボン
  { ID_PARA_sinou_dokki_ribbon, NARC_p_status_gra_ribbon60_NCGR,  0,  mes_ribbon_name_060,  mes_ribbon_text_060,  RIBBON_CATEGORY_MEMORY,  },  //シンオウどっきリボン
  { ID_PARA_sinou_syonbo_ribbon,  NARC_p_status_gra_ribbon61_NCGR,  1,  mes_ribbon_name_061,  mes_ribbon_text_061,  RIBBON_CATEGORY_MEMORY,  },  //シンオウしょんぼリボン
  { ID_PARA_sinou_ukka_ribbon,  NARC_p_status_gra_ribbon62_NCGR,  2,  mes_ribbon_name_062,  mes_ribbon_text_062,  RIBBON_CATEGORY_MEMORY,  },  //シンオウうっかリボン
  { ID_PARA_sinou_sukki_ribbon, NARC_p_status_gra_ribbon63_NCGR,  3,  mes_ribbon_name_063,  mes_ribbon_text_063,  RIBBON_CATEGORY_MEMORY,  },  //シンオウすっきリボン
  { ID_PARA_sinou_gussu_ribbon, NARC_p_status_gra_ribbon64_NCGR,  0,  mes_ribbon_name_064,  mes_ribbon_text_064,  RIBBON_CATEGORY_MEMORY,  },  //シンオウぐっすリボン
  { ID_PARA_sinou_nikko_ribbon, NARC_p_status_gra_ribbon65_NCGR,  2,  mes_ribbon_name_065,  mes_ribbon_text_065,  RIBBON_CATEGORY_MEMORY,  },  //シンオウにっこリボン
  { ID_PARA_sinou_gorgeous_ribbon,  NARC_p_status_gra_ribbon66_NCGR,  1,  mes_ribbon_name_066,  mes_ribbon_text_066,  RIBBON_CATEGORY_MEMORY,  },  //シンオウゴージャスリボン
  { ID_PARA_sinou_royal_ribbon, NARC_p_status_gra_ribbon67_NCGR,  3,  mes_ribbon_name_067,  mes_ribbon_text_067,  RIBBON_CATEGORY_MEMORY,  },  //シンオウロイヤルリボン
  { ID_PARA_sinou_gorgeousroyal_ribbon, NARC_p_status_gra_ribbon68_NCGR,  0,  mes_ribbon_name_068,  mes_ribbon_text_068,  RIBBON_CATEGORY_MEMORY,  },  //シンオウゴージャスロイヤルリボン
  { ID_PARA_sinou_ashiato_ribbon, NARC_p_status_gra_ribbon69_NCGR,  0,  mes_ribbon_name_069,  mes_ribbon_text_069,  RIBBON_CATEGORY_MEMORY,  },  //シンオウあしあとリボン
  { ID_PARA_sinou_record_ribbon,  NARC_p_status_gra_ribbon70_NCGR,  1,  mes_ribbon_name_070,  mes_ribbon_text_070,  RIBBON_CATEGORY_MEMORY,  },  //シンオウレコードリボン
  { ID_PARA_sinou_history_ribbon, NARC_p_status_gra_ribbon71_NCGR,  3,  mes_ribbon_name_071,  mes_ribbon_text_071,  RIBBON_CATEGORY_PRESENT,  },  //シンオウヒストリーリボン
  { ID_PARA_sinou_legend_ribbon,  NARC_p_status_gra_ribbon72_NCGR,  0,  mes_ribbon_name_072,  mes_ribbon_text_072,  RIBBON_CATEGORY_MEMORY,  },  //シンオウレジェンドリボン
  { ID_PARA_sinou_red_ribbon, NARC_p_status_gra_ribbon73_NCGR,  0,  mes_ribbon_name_073,  mes_ribbon_name_073,  RIBBON_CATEGORY_PRESENT,  },  //シンオウレッドリボン
  { ID_PARA_sinou_green_ribbon, NARC_p_status_gra_ribbon74_NCGR,  3,  mes_ribbon_name_074,  mes_ribbon_name_074,  RIBBON_CATEGORY_PRESENT,  },  //シンオウグリーンリボン
  { ID_PARA_sinou_blue_ribbon,  NARC_p_status_gra_ribbon75_NCGR,  1,  mes_ribbon_name_075,  mes_ribbon_name_075,  RIBBON_CATEGORY_PRESENT,  },  //シンオウブルーリボン
  { ID_PARA_sinou_festival_ribbon,  NARC_p_status_gra_ribbon76_NCGR,  1,  mes_ribbon_name_076,  mes_ribbon_name_076,  RIBBON_CATEGORY_PRESENT,  },  //シンオウフェスティバルリボン
  { ID_PARA_sinou_carnival_ribbon,  NARC_p_status_gra_ribbon77_NCGR,  0,  mes_ribbon_name_077,  mes_ribbon_name_077,  RIBBON_CATEGORY_PRESENT,  },  //シンオウカーニバルリボン
  { ID_PARA_sinou_classic_ribbon, NARC_p_status_gra_ribbon78_NCGR,  1,  mes_ribbon_name_078,  mes_ribbon_name_078,  RIBBON_CATEGORY_PRESENT,  },  //シンオウクラシックリボン
  { ID_PARA_sinou_premiere_ribbon,  NARC_p_status_gra_ribbon79_NCGR,  0,  mes_ribbon_name_079,  mes_ribbon_name_079,  RIBBON_CATEGORY_PRESENT,  },  //シンオウプレミアリボン
};



//--------------------------------------------------------------------------------------------
/**
 * リボンデータ取得
 *
 * @param dat_id    データ番号
 * @param prm_id    取得パラメータID
 *
 * @return  指定パラメータ
 */
//--------------------------------------------------------------------------------------------
u32 RIBBON_DataGet( u8 dat_id, u8 prm_id )
{
  switch( prm_id ){
  case RIBBON_PARA_POKEPARA:  // PokeParaGetの取得ID
    return RibbonTable[dat_id].pp_id;

  case RIBBON_PARA_GRAPHIC: // グラフィックID
    return RibbonTable[dat_id].graphic;

  case RIBBON_PARA_PALNUM:  // パレット番号
    return RibbonTable[dat_id].pal_num;

  case RIBBON_PARA_NAME:    // リボン名
    return RibbonTable[dat_id].name;

  case RIBBON_PARA_INFO:    // リボン説明
    return RibbonTable[dat_id].info;

  case RIBBON_PARA_CATEGORY:    // リボン説明
    return RibbonTable[dat_id].category;
  }

  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * リボン説明取得
 *
 * @param sv      セーブデータ
 * @param dat_id    データ番号
 *
 * @return  リボン説明メッセージインデックス
 */
//--------------------------------------------------------------------------------------------
u32 RIBBON_InfoGet( u8 dat_id , u8 * sv )
{
  /*
  if( RibbonTable[dat_id].info & MSG_SP_RIBBON ){
    return ( mes_ribbon_haihu_text_000 + sv[ RibbonTable[dat_id].info & 0xff ] );
  }
  */
  return RibbonTable[dat_id].info;
}

//--------------------------------------------------------------------------------------------
/**
 * リボンIDから説明が格納されている位置を取得
 *
 * @param dat_id    データ番号
 *
 * @retval  "0xff = 配布リボン以外"
 * @retval  "0xff != 格納位置"
 */
//--------------------------------------------------------------------------------------------
u8 RIBBON_SaveIndexGet( u8 dat_id )
{
  if( RibbonTable[dat_id].info & MSG_SP_RIBBON ){
    return ( RibbonTable[dat_id].info & 0xff );
  }
  return 0xff;
}

//--------------------------------------------------------------------------------------------
/**
 * ID_PARA_リボンからmsgIDを取得(08.06.03)  //  PLATINUM MERGE
 *
 * @param para_id   ID_PARA_リボン
 *
 * @retval  "msgID、0xffff = 見つからなかった"
 */
//--------------------------------------------------------------------------------------------
u16 RIBBON_NameGet( u32 pp_id )
{
  int i;

  for( i=0; i < NELEMS(RibbonTable) ;i++ ){

    if( RibbonTable[i].pp_id == pp_id ){
      return ( RibbonTable[i].name );
    }

  }
  return 0xffff;
}


