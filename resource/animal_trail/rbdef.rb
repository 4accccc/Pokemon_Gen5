
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

# 帰る先データインデックス
ZONE_ID_INDEX = 1
X_INDEX = 2
Y_INDEX = 3
Z_INDEX = 4
TITLE_CNT = 2 # 読み飛ばす行数

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

//-------------------------------------
///	けもの道ごとの抽選確率テーブル
//=====================================
static const u8 sc_table_probability[%d][TABLE_TABLE_CNT-1] =
{
EOS

$pos_data_header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  けものみち位置データ
 *	@author	Masayuki Onoue
 *	@date		2011.05.24
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
///	けものみち帰る先データ
//=====================================
static const struct 
{
  u16 zone_id;
  s32  gx;
  s32  gy;
  s32  gz;
}	sc_exit_data[%d]	=
{
EOS
