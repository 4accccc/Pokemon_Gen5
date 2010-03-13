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
	{ID_PARA_champ_ribbon,	NARC_p_status_gra_ribbon_league_00_NCGR,	0,	mes_ribbon_name_league_001,	mes_ribbon_text_league_001,	RIBBON_CATEGORY_LEAGUE	},	//	ホウエン
	{ID_PARA_sinou_champ_ribbon,	NARC_p_status_gra_ribbon_league_01_NCGR,	0,	mes_ribbon_name_league_002,	mes_ribbon_text_league_002,	RIBBON_CATEGORY_LEAGUE	},	//	シンオウ
	{ID_PARA_stylemedal_normal,	NARC_p_status_gra_ribbon_contest_00_NCGR,	0,	mes_ribbon_name_contest_001,	mes_ribbon_text_contest_001,	RIBBON_CATEGORY_CONTEST	},	//	クールリボン
	{ID_PARA_stylemedal_super,	NARC_p_status_gra_ribbon_contest_01_NCGR,	0,	mes_ribbon_name_contest_002,	mes_ribbon_text_contest_002,	RIBBON_CATEGORY_CONTEST	},	//	クールリボンスーパー
	{ID_PARA_stylemedal_hyper,	NARC_p_status_gra_ribbon_contest_02_NCGR,	0,	mes_ribbon_name_contest_003,	mes_ribbon_text_contest_003,	RIBBON_CATEGORY_CONTEST	},	//	クールリボンハイパー
	{ID_PARA_stylemedal_master,	NARC_p_status_gra_ribbon_contest_03_NCGR,	0,	mes_ribbon_name_contest_004,	mes_ribbon_text_contest_004,	RIBBON_CATEGORY_CONTEST	},	//	クールリボンマスター
	{ID_PARA_beautifulmedal_normal,	NARC_p_status_gra_ribbon_contest_00_NCGR,	1,	mes_ribbon_name_contest_005,	mes_ribbon_text_contest_005,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボン
	{ID_PARA_beautifulmedal_super,	NARC_p_status_gra_ribbon_contest_01_NCGR,	1,	mes_ribbon_name_contest_006,	mes_ribbon_text_contest_006,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボンスーパー
	{ID_PARA_beautifulmedal_hyper,	NARC_p_status_gra_ribbon_contest_02_NCGR,	1,	mes_ribbon_name_contest_007,	mes_ribbon_text_contest_007,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボンハイパー
	{ID_PARA_beautifulmedal_master,	NARC_p_status_gra_ribbon_contest_03_NCGR,	1,	mes_ribbon_name_contest_008,	mes_ribbon_text_contest_008,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボンマスター
	{ID_PARA_cutemedal_normal,	NARC_p_status_gra_ribbon_contest_00_NCGR,	2,	mes_ribbon_name_contest_009,	mes_ribbon_text_contest_009,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボン
	{ID_PARA_cutemedal_super,	NARC_p_status_gra_ribbon_contest_01_NCGR,	2,	mes_ribbon_name_contest_010,	mes_ribbon_text_contest_010,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボンスーパー
	{ID_PARA_cutemedal_hyper,	NARC_p_status_gra_ribbon_contest_02_NCGR,	2,	mes_ribbon_name_contest_011,	mes_ribbon_text_contest_011,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボンハイパー
	{ID_PARA_cutemedal_master,	NARC_p_status_gra_ribbon_contest_03_NCGR,	2,	mes_ribbon_name_contest_012,	mes_ribbon_text_contest_012,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボンマスター
	{ID_PARA_clevermedal_normal,	NARC_p_status_gra_ribbon_contest_00_NCGR,	3,	mes_ribbon_name_contest_013,	mes_ribbon_text_contest_013,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボン
	{ID_PARA_clevermedal_super,	NARC_p_status_gra_ribbon_contest_01_NCGR,	3,	mes_ribbon_name_contest_014,	mes_ribbon_text_contest_014,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボンスーパー
	{ID_PARA_clevermedal_hyper,	NARC_p_status_gra_ribbon_contest_02_NCGR,	3,	mes_ribbon_name_contest_015,	mes_ribbon_text_contest_015,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボンハイパー
	{ID_PARA_clevermedal_master,	NARC_p_status_gra_ribbon_contest_03_NCGR,	3,	mes_ribbon_name_contest_016,	mes_ribbon_text_contest_016,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボンマスター
	{ID_PARA_strongmedal_normal,	NARC_p_status_gra_ribbon_contest_00_NCGR,	4,	mes_ribbon_name_contest_017,	mes_ribbon_text_contest_017,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボン
	{ID_PARA_strongmedal_super,	NARC_p_status_gra_ribbon_contest_01_NCGR,	4,	mes_ribbon_name_contest_018,	mes_ribbon_text_contest_018,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボンスーパー
	{ID_PARA_strongmedal_hyper,	NARC_p_status_gra_ribbon_contest_02_NCGR,	4,	mes_ribbon_name_contest_019,	mes_ribbon_text_contest_019,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボンハイパー
	{ID_PARA_strongmedal_master,	NARC_p_status_gra_ribbon_contest_03_NCGR,	4,	mes_ribbon_name_contest_020,	mes_ribbon_text_contest_020,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボンマスター
	{ID_PARA_trial_stylemedal_normal,	NARC_p_status_gra_ribbon_contest_04_NCGR,	0,	mes_ribbon_name_contest_021,	mes_ribbon_text_contest_021,	RIBBON_CATEGORY_CONTEST	},	//	クールリボン
	{ID_PARA_trial_stylemedal_super,	NARC_p_status_gra_ribbon_contest_05_NCGR,	0,	mes_ribbon_name_contest_022,	mes_ribbon_text_contest_022,	RIBBON_CATEGORY_CONTEST	},	//	クールリボングレート
	{ID_PARA_trial_stylemedal_hyper,	NARC_p_status_gra_ribbon_contest_06_NCGR,	0,	mes_ribbon_name_contest_023,	mes_ribbon_text_contest_023,	RIBBON_CATEGORY_CONTEST	},	//	クールリボンウルトラ
	{ID_PARA_trial_stylemedal_master,	NARC_p_status_gra_ribbon_contest_07_NCGR,	0,	mes_ribbon_name_contest_024,	mes_ribbon_text_contest_024,	RIBBON_CATEGORY_CONTEST	},	//	クールリボンマスター
	{ID_PARA_trial_beautifulmedal_normal,	NARC_p_status_gra_ribbon_contest_04_NCGR,	1,	mes_ribbon_name_contest_025,	mes_ribbon_text_contest_025,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボン
	{ID_PARA_trial_beautifulmedal_super,	NARC_p_status_gra_ribbon_contest_05_NCGR,	1,	mes_ribbon_name_contest_026,	mes_ribbon_text_contest_026,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボングレート
	{ID_PARA_trial_beautifulmedal_hyper,	NARC_p_status_gra_ribbon_contest_06_NCGR,	1,	mes_ribbon_name_contest_027,	mes_ribbon_text_contest_027,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボンウルトラ
	{ID_PARA_trial_beautifulmedal_master,	NARC_p_status_gra_ribbon_contest_07_NCGR,	1,	mes_ribbon_name_contest_028,	mes_ribbon_text_contest_028,	RIBBON_CATEGORY_CONTEST	},	//	ビューティリボンマスター
	{ID_PARA_trial_cutemedal_normal,	NARC_p_status_gra_ribbon_contest_04_NCGR,	2,	mes_ribbon_name_contest_029,	mes_ribbon_text_contest_029,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボン
	{ID_PARA_trial_cutemedal_super,	NARC_p_status_gra_ribbon_contest_05_NCGR,	2,	mes_ribbon_name_contest_030,	mes_ribbon_text_contest_030,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボングレート
	{ID_PARA_trial_cutemedal_hyper,	NARC_p_status_gra_ribbon_contest_06_NCGR,	2,	mes_ribbon_name_contest_031,	mes_ribbon_text_contest_031,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボンウルトラ
	{ID_PARA_trial_cutemedal_master,	NARC_p_status_gra_ribbon_contest_07_NCGR,	2,	mes_ribbon_name_contest_032,	mes_ribbon_text_contest_032,	RIBBON_CATEGORY_CONTEST	},	//	キュートリボンマスター
	{ID_PARA_trial_clevermedal_normal,	NARC_p_status_gra_ribbon_contest_04_NCGR,	3,	mes_ribbon_name_contest_033,	mes_ribbon_text_contest_033,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボン
	{ID_PARA_trial_clevermedal_super,	NARC_p_status_gra_ribbon_contest_05_NCGR,	3,	mes_ribbon_name_contest_034,	mes_ribbon_text_contest_034,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボングレート
	{ID_PARA_trial_clevermedal_hyper,	NARC_p_status_gra_ribbon_contest_06_NCGR,	3,	mes_ribbon_name_contest_035,	mes_ribbon_text_contest_035,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボンウルトラ
	{ID_PARA_trial_clevermedal_master,	NARC_p_status_gra_ribbon_contest_07_NCGR,	3,	mes_ribbon_name_contest_036,	mes_ribbon_text_contest_036,	RIBBON_CATEGORY_CONTEST	},	//	ジーニアスリボンマスター
	{ID_PARA_trial_strongmedal_normal,	NARC_p_status_gra_ribbon_contest_04_NCGR,	4,	mes_ribbon_name_contest_037,	mes_ribbon_text_contest_037,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボン
	{ID_PARA_trial_strongmedal_super,	NARC_p_status_gra_ribbon_contest_05_NCGR,	4,	mes_ribbon_name_contest_038,	mes_ribbon_text_contest_038,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボングレート
	{ID_PARA_trial_strongmedal_hyper,	NARC_p_status_gra_ribbon_contest_06_NCGR,	4,	mes_ribbon_name_contest_039,	mes_ribbon_text_contest_039,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボンウルトラ
	{ID_PARA_trial_strongmedal_master,	NARC_p_status_gra_ribbon_contest_07_NCGR,	4,	mes_ribbon_name_contest_040,	mes_ribbon_text_contest_040,	RIBBON_CATEGORY_CONTEST	},	//	パワフルリボンマスター
	{ID_PARA_winning_ribbon,	NARC_p_status_gra_ribbon_tower_00_NCGR,	0,	mes_ribbon_name_tower_001,	mes_ribbon_text_tower_001,	RIBBON_CATEGORY_TOWER	},	//	ウイニングリボン
	{ID_PARA_victory_ribbon,	NARC_p_status_gra_ribbon_tower_01_NCGR,	0,	mes_ribbon_name_tower_002,	mes_ribbon_text_tower_002,	RIBBON_CATEGORY_TOWER	},	//	ビクトリーリボン
	{ID_PARA_sinou_battle_tower_ttwin_first,	NARC_p_status_gra_ribbon_tower_02_NCGR,	0,	mes_ribbon_name_tower_003,	mes_ribbon_text_tower_003,	RIBBON_CATEGORY_TOWER	},	//	アビリティリボン
	{ID_PARA_sinou_battle_tower_ttwin_second,	NARC_p_status_gra_ribbon_tower_03_NCGR,	0,	mes_ribbon_name_tower_004,	mes_ribbon_text_tower_004,	RIBBON_CATEGORY_TOWER	},	//	グレートアビリティリボン
	{ID_PARA_sinou_battle_tower_2vs2_win50,	NARC_p_status_gra_ribbon_tower_04_NCGR,	0,	mes_ribbon_name_tower_005,	mes_ribbon_text_tower_005,	RIBBON_CATEGORY_TOWER	},	//	ダブルアビリティリボン
	{ID_PARA_sinou_battle_tower_aimulti_win50,	NARC_p_status_gra_ribbon_tower_05_NCGR,	0,	mes_ribbon_name_tower_006,	mes_ribbon_text_tower_006,	RIBBON_CATEGORY_TOWER	},	//	マルチアビリティリボン
	{ID_PARA_sinou_battle_tower_siomulti_win50,	NARC_p_status_gra_ribbon_tower_06_NCGR,	0,	mes_ribbon_name_tower_007,	mes_ribbon_text_tower_007,	RIBBON_CATEGORY_TOWER	},	//	ペアアビリティリボン
	{ID_PARA_sinou_battle_tower_wifi_rank5,	NARC_p_status_gra_ribbon_tower_07_NCGR,	0,	mes_ribbon_name_tower_008,	mes_ribbon_text_tower_008,	RIBBON_CATEGORY_TOWER	},	//	ワールドアビリティリボン
	{ID_PARA_bromide_ribbon,	NARC_p_status_gra_ribbon_memory_00_NCGR,	1,	mes_ribbon_name_memory_001,	mes_ribbon_text_memory_001,	RIBBON_CATEGORY_MEMORY	},	//	ブロマイドリボン
	{ID_PARA_ganba_ribbon,	NARC_p_status_gra_ribbon_memory_01_NCGR,	2,	mes_ribbon_name_memory_002,	mes_ribbon_text_memory_002,	RIBBON_CATEGORY_MEMORY	},	//	がんばリボン
	{ID_PARA_sinou_syakki_ribbon,	NARC_p_status_gra_ribbon_memory_02_NCGR,	2,	mes_ribbon_name_memory_003,	mes_ribbon_text_memory_003,	RIBBON_CATEGORY_MEMORY	},	//	しゃっきリボン
	{ID_PARA_sinou_dokki_ribbon,	NARC_p_status_gra_ribbon_memory_03_NCGR,	0,	mes_ribbon_name_memory_004,	mes_ribbon_text_memory_004,	RIBBON_CATEGORY_MEMORY	},	//	どっきリボン
	{ID_PARA_sinou_syonbo_ribbon,	NARC_p_status_gra_ribbon_memory_04_NCGR,	1,	mes_ribbon_name_memory_005,	mes_ribbon_text_memory_005,	RIBBON_CATEGORY_MEMORY	},	//	しょんぼリボン
	{ID_PARA_sinou_ukka_ribbon,	NARC_p_status_gra_ribbon_memory_05_NCGR,	2,	mes_ribbon_name_memory_006,	mes_ribbon_text_memory_006,	RIBBON_CATEGORY_MEMORY	},	//	うっかリボン
	{ID_PARA_sinou_sukki_ribbon,	NARC_p_status_gra_ribbon_memory_06_NCGR,	3,	mes_ribbon_name_memory_007,	mes_ribbon_text_memory_007,	RIBBON_CATEGORY_MEMORY	},	//	すっきリボン
	{ID_PARA_sinou_gussu_ribbon,	NARC_p_status_gra_ribbon_memory_07_NCGR,	0,	mes_ribbon_name_memory_008,	mes_ribbon_text_memory_008,	RIBBON_CATEGORY_MEMORY	},	//	ぐっすリボン
	{ID_PARA_sinou_nikko_ribbon,	NARC_p_status_gra_ribbon_memory_08_NCGR,	2,	mes_ribbon_name_memory_009,	mes_ribbon_text_memory_009,	RIBBON_CATEGORY_MEMORY	},	//	にっこリボン
	{ID_PARA_sinou_gorgeous_ribbon,	NARC_p_status_gra_ribbon_memory_09_NCGR,	1,	mes_ribbon_name_memory_010,	mes_ribbon_text_memory_010,	RIBBON_CATEGORY_MEMORY	},	//	ゴージャスリボン
	{ID_PARA_sinou_royal_ribbon,	NARC_p_status_gra_ribbon_memory_10_NCGR,	3,	mes_ribbon_name_memory_011,	mes_ribbon_text_memory_011,	RIBBON_CATEGORY_MEMORY	},	//	ロイヤルリボン
	{ID_PARA_sinou_gorgeousroyal_ribbon,	NARC_p_status_gra_ribbon_memory_11_NCGR,	0,	mes_ribbon_name_memory_012,	mes_ribbon_text_memory_012,	RIBBON_CATEGORY_MEMORY	},	//	ゴージャスロイヤルリボン
	{ID_PARA_sinou_ashiato_ribbon,	NARC_p_status_gra_ribbon_memory_12_NCGR,	0,	mes_ribbon_name_memory_013,	mes_ribbon_text_memory_013,	RIBBON_CATEGORY_MEMORY	},	//	あしあとリボン
	{ID_PARA_sinou_record_ribbon,	NARC_p_status_gra_ribbon_memory_13_NCGR,	1,	mes_ribbon_name_memory_014,	mes_ribbon_text_memory_014,	RIBBON_CATEGORY_MEMORY	},	//	レコードリボン
	{ID_PARA_sinou_legend_ribbon,	NARC_p_status_gra_ribbon_memory_14_NCGR,	0,	mes_ribbon_name_memory_015,	mes_ribbon_text_memory_015,	RIBBON_CATEGORY_MEMORY	},	//	レジェンドリボン
	{ID_PARA_country_ribbon,	NARC_p_status_gra_ribbon_present_00_NCGR,	3,	mes_ribbon_name_present_001,	mes_ribbon_text_present_001,	RIBBON_CATEGORY_PRESENT	},	//	カントリーリボン
	{ID_PARA_national_ribbon,	NARC_p_status_gra_ribbon_present_00_NCGR,	4,	mes_ribbon_name_present_002,	mes_ribbon_text_present_002,	RIBBON_CATEGORY_PRESENT	},	//	ナショナルリボン
	{ID_PARA_earth_ribbon,	NARC_p_status_gra_ribbon_present_01_NCGR,	0,	mes_ribbon_name_present_003,	mes_ribbon_text_present_003,	RIBBON_CATEGORY_PRESENT	},	//	アースリボン
	{ID_PARA_world_ribbon,	NARC_p_status_gra_ribbon_present_01_NCGR,	1,	mes_ribbon_name_present_004,	mes_ribbon_text_present_004,	RIBBON_CATEGORY_PRESENT	},	//	ワールドリボン
	{ID_PARA_sinou_classic_ribbon,	NARC_p_status_gra_ribbon_present_02_NCGR,	1,	mes_ribbon_name_present_005,	mes_ribbon_text_present_005,	RIBBON_CATEGORY_PRESENT	},	//	クラシックリボン
	{ID_PARA_sinou_premiere_ribbon,	NARC_p_status_gra_ribbon_present_03_NCGR,	0,	mes_ribbon_name_present_006,	mes_ribbon_text_present_006,	RIBBON_CATEGORY_PRESENT	},	//	プレミアリボン
	{ID_PARA_sinou_history_ribbon,	NARC_p_status_gra_ribbon_present_04_NCGR,	3,	mes_ribbon_name_present_007,	mes_ribbon_text_present_007,	RIBBON_CATEGORY_PRESENT	},	//	イベントリボン
	{ID_PARA_sinou_green_ribbon,	NARC_p_status_gra_ribbon_present_05_NCGR,	2,	mes_ribbon_name_present_008,	mes_ribbon_text_present_008,	RIBBON_CATEGORY_PRESENT	},	//	バースデーリボン
	{ID_PARA_sinou_blue_ribbon,	NARC_p_status_gra_ribbon_present_06_NCGR,	2,	mes_ribbon_name_present_009,	mes_ribbon_text_present_009,	RIBBON_CATEGORY_PRESENT	},	//	スペシャルリボン
	{ID_PARA_sinou_festival_ribbon,	NARC_p_status_gra_ribbon_present_07_NCGR,	1,	mes_ribbon_name_present_010,	mes_ribbon_text_present_010,	RIBBON_CATEGORY_PRESENT	},	//	メモリアルリボン
	{ID_PARA_sinou_carnival_ribbon,	NARC_p_status_gra_ribbon_present_08_NCGR,	0,	mes_ribbon_name_present_011,	mes_ribbon_text_present_011,	RIBBON_CATEGORY_PRESENT	},	//	ウィッシュリボン
	{ID_PARA_marine_ribbon,	NARC_p_status_gra_ribbon_present_09_NCGR,	3,	mes_ribbon_name_present_012,	mes_ribbon_text_present_012,	RIBBON_CATEGORY_PRESENT	},	//	バトルチャンプリボン
	{ID_PARA_land_ribbon,	NARC_p_status_gra_ribbon_present_10_NCGR,	3,	mes_ribbon_name_present_013,	mes_ribbon_text_present_013,	RIBBON_CATEGORY_PRESENT	},	//	エリアチャンプリボン
	{ID_PARA_sky_ribbon,	NARC_p_status_gra_ribbon_present_11_NCGR,	1,	mes_ribbon_name_present_014,	mes_ribbon_text_present_014,	RIBBON_CATEGORY_PRESENT	},	//	ナショナルチャンプリボン
	{ID_PARA_sinou_red_ribbon,	NARC_p_status_gra_ribbon_present_12_NCGR,	1,	mes_ribbon_name_present_015,	mes_ribbon_text_present_015,	RIBBON_CATEGORY_PRESENT	},	//	ワールドチャンプリボン
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


