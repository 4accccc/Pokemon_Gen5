//======================================================================
/**
 * @file  bsubway_scrwork.h
 * @bfief  バトルサブウェイスクリプトワーク
 * @author Satoshi Nohara
 * @date  07.05.28
 * @note プラチナより移植 kagaya
 */
//======================================================================
#ifndef  __SUBWAY_SCRWORK_H__
#define  __SUBWAY_SCRWORK_H__

#include "field/location.h"

#include "savedata/bsubway_savedata.h"
#include "savedata/bsubway_savedata_def.h"
#include "battle/bsubway_battle_data.h"
#include "bsubway_scr_def.h"

#include "event_field_proclink.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
///  LOCATION_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  int zone_id;
  int door_id;
  int grid_x;
  int grid_z;
  int dir;
}LOCATION_WORK;
#endif

//--------------------------------------------------------------
/// バトルサブウェイ　スクリプトワーク
//--------------------------------------------------------------
struct _BSUBWAY_SCRWORK
{
  int magicNo;  ///<マジックナンバー
  int heapID;  ///<ヒープ
  
  u8 member_num;  ///<メンバー数
  u8 play_mode;  ///<プレイデータ
  u8 my_sex;      ///<自分の性別
  u8 pare_sex;    ///<自分とパートナーの性別
  
  u8 clear_f:1;  ///<クリアフラグ
  u8 boss_f:2;  ///<ボスを倒したフラグ
  u8 retire_f:1;  ///<リタイアフラグ
  u8 prize_f:1;  ///<ご褒美フラグ
  u8 partner:3;  ///<パートナーNo
  u8 padding0; ///<ビット余り
  u8 padding1; ///<ビット余り
  
  u16 pare_poke[2];  ///<通信マルチパートナーが持つポケモンNo
  
  u16 pare_stage_no; ///<通信マルチパートナーの周回数
  u16 rec_turn; ///<かかったターン数
  u16 rec_damage; ///<食らったダメージ
  u16 rec_down; ///<ポケモンのダウン数
  
  u8  member[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンの手持ちNo
  u16  mem_poke[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンのmonsno
  u16  mem_item[BSUBWAY_STOCK_MEMBER_MAX];  ///<選んだポケモンのアイテムno
  u16  trainer[BSUBWAY_STOCK_TRAINER_MAX];  ///<対戦トレーナーNo
  
  LOCATION now_location; //現在のロケーション
  
  BSUBWAY_PLAYDATA *playData;  ///<セーブデータへのポインタ
  BSUBWAY_SCOREDATA *scoreData;  ///<セーブデータへのポインタ
  
  u8 pokelist_select_num[6]; ///<ポケモン選択で取得した手持ちNo
  u16 pokelist_result_select; ///<ポケモンリスト戻り値
  u16 pokelist_return_mode;
  
  //対戦相手データ格納場所
  BSUBWAY_PARTNER_DATA  tr_data[2];
  
  u16  send_buf[35];  //通信用データバッファ
  u16  recv_buf[35];  //通信用データバッファ
  
  #if 0 //wb null
  //プラチナで追加
  int winlose_flag; //(LOCATION_WORKを消して、そこに入れることも可能？)
  void *p_work; //(LOCATION_WORKを消して、そこに入れることも可能？)
  u8  recieve_count;
  u8  mode;
  u8  first_btl_flag;    //初戦をおこなったかフラグ
  u8  dummy;
  u16  check_work;
  u16  ret_wkno;
  #endif
};

#endif  //__SUBWAY_SCRWORK_H__
