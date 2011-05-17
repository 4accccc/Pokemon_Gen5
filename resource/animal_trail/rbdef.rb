
# データ定義位置
PROBABILITY_ROW   = 4 # 確率テーブル
PROBABILITY_LINE  = 2 # 確率テーブル

ITEM1_ROW   = 19 # 通常アイテムテーブル
ITEM1_LINE  = 1

ITEM1_HIDE_ROW   = 19 # 隠しアイテムテーブル
ITEM1_HIDE_LINE  = 4

POKE1_ROW   = 19 # ポケモンテーブル1
POKE1_LINE  = 7

ITEM_TABLE_CNT      = 4 # アイテムテーブルの長さ
HIDE_ITEM_TABLE_CNT = 4 # アイテムテーブルの長さ
POKE_TABLE_CNT      = 4 # ポケモンテーブルの長さ

POKE_RANK_CNT  = 3 # データテーブル数
ITEM_RANK_CNT  = 4 # データテーブル数
HIDE_ITEM_RANK_CNT  = 4 # データテーブル数
NEXT_TABLE_POS = 7 # 次ランクテーブル位置

TABLE_PROBABILITY = 11 # テーブル抽選データ数

$header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  けものみち抽選データ
 *	@author	Masayuki Onoue
 *	@date		2011.05.17
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#define POKEMON_TABLE_LEN   (4) // ポケモン抽選テーブルの長さ
#define ITEM_TABLE_LEN      (4) // アイテム抽選テーブルの長さ

#define POKEMON_TABLE_CNT     (3) // ポケモン抽選テーブル数
#define NORMAL_ITEM_TABLE_CNT (4) // 通常配置アイテムの抽選テーブル数
#define HIDE_ITEM_TABLE_CNT   (4) // 隠しアイテム抽選テーブル数
#define TABLE_TABLE_CNT (POKEMON_TABLE_CNT+NORMAL_ITEM_TABLE_CNT+HIDE_ITEM_TABLE_CNT)

//-------------------------------------
///	アイテムデータ
//=====================================
typedef struct {
  u16 item_id[ITEM_TABLE_LEN];
}AT_ITEM_DATA;

//-------------------------------------
///	ポケモンデータ
//=====================================
typedef struct {
  u16 pokemon_id[POKEMON_TABLE_LEN];
  u8  max_lv[POKEMON_TABLE_LEN];
  u8  min_lv[POKEMON_TABLE_LEN];
  u8  female_probability[POKEMON_TABLE_LEN];
  u8  forum[POKEMON_TABLE_LEN];
}AT_POKEMON_DATA;

//-------------------------------------
///	けものみち抽選データ
//=====================================
typedef struct {
  u8 table_probability[TABLE_TABLE_CNT-1];  // 最後のテーブルの確率は保存しない
  AT_POKEMON_DATA pokemon_table[POKEMON_TABLE_CNT];
  AT_ITEM_DATA    normal_item_table[NORMAL_ITEM_TABLE_CNT];
  AT_ITEM_DATA    hide_item_table[HIDE_ITEM_TABLE_CNT];
}AT_DATA;

static const AT_DATA sc_animal_trail_data_table[20] = {
EOS

