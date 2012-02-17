#----------------------------------------------------------------------------
# ポケウッドあらすじ背景コンバーター
# 2011/10 by mori
# 
# 入力：ruby briefing_conv.rb <出力テキスト>
# 出力：<出力テキスト>に背景モデリングデータの定義リストが出力されます
#----------------------------------------------------------------------------
require 'kconv'
require "jcode"
require 'fileutils'

$KECODE = 'SJIS'

#-----------------------------------------------------------------------------
# データ定義
#-----------------------------------------------------------------------------

SCENARIO_NUM      = 41  #シナリオデータの数
SCENARIO_CSV_NAME = "pokewood_senario%02d.txt"
NOFILE_STRING     = "%sというファイルがありません\n"

# batt_bg_tbl.xlsから導かれるテーブル
$field_hash    = {
  "ビル屋上Ａ"=>"field01a", "ビル屋上Ａー１"=>"field01a", "ビル屋上Ａー２"=>"field01b","ビル屋上Ａー３"=>"field01c","ビル屋上Ａー４"=>"field01d","ビル屋上Ａー５"=>"field01e",
  "遊園地Ａ"=>"field02a", "遊園地Ａー１"=>"field02a","遊園地Ａー２"=>"field02b","遊園地Ａー３"=>"field02c","遊園地Ａー４"=>"field02d","遊園地Ａー５"=>"field02e",
  "都会・雑踏"=>"field03a", "都会・雑踏ー１"=>"field03a", "都会・雑踏ー２"=>"field03b", "都会・雑踏ー３"=>"field03c", "都会・雑踏ー４"=>"field03d", "都会・雑踏ー５"=>"field03d", 
  "未来都市"=>"field04a", "未来都市ー１"=>"field04a", "未来都市ー２"=>"field04b", 
  "砂浜"=>"field05a", "砂浜ー１"=>"field05a","砂浜ー２"=>"field05b","砂浜ー３"=>"field05c","砂浜ー４"=>"field05d",
  "波止場"=>"field06a", "波止場ー１"=>"field06a", "波止場ー２"=>"field06b", "波止場ー３"=>"field06c", "波止場ー４"=>"field06d", 
  "魔法の世界"=>"field08a", "魔法の世界ー１"=>"field08a",  "魔法の世界ー２"=>"field08b",  "魔法の世界ー３"=>"field08c",  "魔法の世界ー４"=>"field08d", 
  "墓地"=>"field09a", "墓地ー１"=>"field09a",  "墓地ー２"=>"field09b",  "墓地ー３"=>"field09c",  "墓地ー４"=>"field09d",  "墓地ー５"=>"field09e", 
  "研究施設"=>"field10a", "研究施設ー１"=>"field10a","研究施設ー２"=>"field10b","研究施設ー３"=>"field10c","研究施設ー４"=>"field10d","研究施設ー５"=>"field10e",
  "一般家屋室内"=>"field11a", "一般家屋室内ー１"=>"field11a", "一般家屋室内ー２"=>"field11b", "一般家屋室内ー３"=>"field11c", "一般家屋室内ー４"=>"field11d", "一般家屋室内ー５"=>"field11e", 
  "宇宙"=>"field13a", "宇宙ー１"=>"field13a", "宇宙ー２"=>"field13b", "宇宙ー３"=>"field13c", 
  "遊園地Ｂ"=>"field02f", "遊園地Ｂー１"=>"field02f", "遊園地Ｂー２"=>"field02g", "遊園地Ｂー３"=>"field02h", "遊園地Ｂー４"=>"field02i", 
  "都会・雑踏Ｂ"=>"field03f", "都会・雑踏Ｂー１"=>"field03f", "都会・雑踏Ｂー２"=>"field03g", "都会・雑踏Ｂー３"=>"field03h", 
}

# CDATファイルのヘッダ部
CDAT_HEADER= <<"HEARDOC"
//==============================================================================
/**
 * @file  pkwd_briefing_model.cdat
 * @brief ポケウッドあらすじ画面用モデリングデータ定義
 * @author  mori
 *
 * @note このファイルはresource/pokewoodで自動生成されています。
 *       またresource/battle/wb_battle_bg.xlsに登録されているモデリングデータを
 *       登録するようにしています。そのため上記のテーブルからファイルが
 *       無くなるとコンパイルが通らなくなりますので注意。
 *
 *
 */
//==============================================================================

//=======================================================
// 読み込む3Dデータ
//=======================================================
HEARDOC

# モデリングデータ定義部
MODEL_TBL_STRING = <<"HEARDOC"
//%s
static const GFL_G3D_UTIL_RES g3Dutil_resTbl%d[] = {
  { ARCID_BATTGRA, NARC_battgra_wb_pkwd_%s_NSBMD, GFL_G3D_UTIL_RESARC },
  { ARCID_BATTGRA, NARC_battgra_wb_pkwd_%s_NSBCA, GFL_G3D_UTIL_RESARC },
};
HEARDOC


# アニメテーブル定義部
ANIME_TBL_STRING = <<"HEARDOC"
// 「%s」 アニメ定義
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl%d[] = {
  { G3DRES_BCTA_BCA, 0  },
//  { G3DRES_01_BTP, 0  },
};
HEARDOC

# モデリングデータ設定テーブル定義部
G3D_UTIL_STRING = <<"HEARDOC"
// %s
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl%d[] = {
  {
    G3DRES_BCTA_BMD,         //モデルID
    0,                       //モデルデータID(データ内部INDEX)
    G3DRES_BCTA_BMD,         //テクスチャID
    g3Dutil_anmTbl%d,         //アニメテーブル(複数指定のため)
    NELEMS(g3Dutil_anmTbl%d), //アニメデータ数
  },
};
HEARDOC

# モデリングデータ初期化定義部
G3D_SETUP_STRING = <<"HEARDOC"
  { // シナリオ%d ( %s )
    g3Dutil_resTbl%d,           //データテーブル
    NELEMS(g3Dutil_resTbl%d),   //データ数
    g3Dutil_objTbl%d,           //オブジェクト設定テーブル
    NELEMS(g3Dutil_objTbl%d),   //オブジェクト数
  },
HEARDOC

#-----------------------------------
# タイトル文字列
#-----------------------------------
TITLE_3DANIME_STRING = "
//=======================================================
// 3Dアニメ定義
//=======================================================
"
TITLE_G3D_UTIL_STRING = "
//=======================================================
//3Dオブジェクト設定テーブル
//=======================================================
"

TITLE_G3D_SETUP_STRING = "
//=======================================================
// 3Dデータセットアップデータ
//=======================================================
static const GFL_G3D_UTIL_SETUP g3Dutil_setup[] = {
"



#-------------------------------------------------------------------------------------
# メイン
#-------------------------------------------------------------------------------------

$resource_name = Array.new

# シナリオデータのcsvファイルが存在しているかチェック
SCENARIO_NUM.times{|i|
  name = sprintf( SCENARIO_CSV_NAME, i+1 )
  #「pokewood_senario01.txt～」が存在しているかチェック
  if FileTest.exist?( name )==false then
    printf( NOFILE_STRING, name )
    exit(1)
  end
}


# シナリオフォーマット読み込み
SCENARIO_NUM.times{|i|
  name = sprintf( SCENARIO_CSV_NAME, i+1 )
  File.open(name){|file|
    13.times{ line=file.gets }  #14行目を読み込んで
    line=file.gets
    words = line.split(/,/)     #2番目のセルの文字列を取得
    $resource_name << words[1]
  }
}


# 最低限素材の取得
uniq_name = $resource_name.uniq

#cdat作成
File.open(ARGV[0],"w"){|file|
  #ヘッダ
  file.printf(CDAT_HEADER)

  #モデリングデータ
  uniq_name.each_with_index{|name, i|
    file.printf( MODEL_TBL_STRING, name, i, $field_hash[name], $field_hash[name])
  }
  
  #アニメ定義開始
  file.printf(TITLE_3DANIME_STRING)

  #アニメ定義データ
  uniq_name.each_with_index{|name, i|
    file.printf( ANIME_TBL_STRING, name, i )
  }

  # オブジェクト設定開始
  file.printf(TITLE_G3D_UTIL_STRING)
  #オブジェクト定義
  uniq_name.each_with_index{|name, i|
    file.printf( G3D_UTIL_STRING, name, i, i, i )
  }

  # シナリオ用初期化定義
  file.printf(TITLE_G3D_SETUP_STRING)
  $resource_name.each_with_index{|name, i|
    uniq_no = uniq_name.index(name)
    file.printf( G3D_SETUP_STRING, i+1, name, uniq_no, uniq_no, uniq_no, uniq_no)
  }

  file.printf("};\n")
}


# 取得結果の確認
SCENARIO_NUM.times{|i|
  printf("%i = %s → %s\n", i, $resource_name[i], $field_hash[$resource_name[i]])
}
printf("\nuniq\n")
uniq_name.each_with_index{|name, i|
  printf("%i = %s\n", i, uniq_name[i])
}