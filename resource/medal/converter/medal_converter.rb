##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		medal_converter.rb
#*	@brief  メダルデータを生成するコンバータ
#*	@author		Masayuki Onoue
#*	@data		2011.01.07
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE="s"
require "jcode" # String クラスの tr メソッドで日本語を使用可能にする。
require "csv"
require File.dirname(__FILE__) + '/rbdef.rb'
require File.dirname(__FILE__) + '/../../../tools/gmm_make/gmm_make'
require File.dirname(__FILE__) + '/recorddef.rb'
require File.dirname(__FILE__) + '/gratypeddef.rb'

class MEDAL_DATA
  attr_accessor :no ,:id ,:medal_name ,:record_id ,:count ,:hint_condition ,:hint_level ,:graphic_type ,:graphic_id ,:explanation_kana ,:explanation_kanzi ,:hint_kana ,:hint_kanzi ,:comment
end

# =====================================
# グラフィック名からIDを取得する
# =====================================
def GetGraphicID( name )
  if GRAPHICTYPE_HASH.key?(name) then
    puts name + "-" +GRAPHICTYPE_HASH[name].to_s
    return GRAPHICTYPE_HASH[name]
  else
    raise "未定義のグラフィックIDが使用されています->"+name
    return nil
  end
end

# =====================================
# ヒントタイプからIDを取得する
# =====================================
def GetHintConditionID( name )
  if MDL_HINT_CONDITION_ID.key?(name) then
    return MDL_HINT_CONDITION_ID[name]
  else
    raise "未定義のヒント開示条件が使用されています->"+name
    return nil
  end
end

# メダルデータCSV名
CSV_FILE_NAME             = ARGV[0]
ID_FILE_NAME              = ARGV[1]
MDLREC_FILE_NAME          = ARGV[2]
SORT_FILE_NAME            = ARGV[3]
TMPGMM_NAME               = ARGV[4]
MEDALNAMEGMM_NAME         = ARGV[5]
MEDALEXPLANATIONGMM_NAME  = ARGV[6]
MEDALHINTGMM_NAME         = ARGV[7]
CNV_DIR                   = ARGV[8]

# メダルデータリスト
medal_data_list = Array.new
puts CSV_FILE_NAME
# メダルデータの読み込み
medal_csv = CSV.open( CSV_FILE_NAME ,"r")

# データの読み飛ばし
3.times{
  medal_csv.shift
}

# データの読み込み
medal_csv.each { |item|

  # 終了文字列の検出
  if item[0].include?('#END') then
    break
  end

  # データを取得する
  medal_data = MEDAL_DATA.new
  medal_data.no             = item[COL_NO]
  medal_data.id             = "MDLID_" + item[COL_ID]
  medal_data.medal_name     = item[COL_NAME]
  medal_data.record_id      = item[COL_RECID].strip
  medal_data.count          = item[COL_COUNT]
  medal_data.hint_condition = item[COL_HINT_CONDHITION]
  medal_data.hint_level     = item[COL_HINT_LEVEL]
  medal_data.graphic_type   = item[COL_GRAPHIC_ID]
  medal_data.graphic_id     = GetGraphicID(item[COL_GRAPHIC_ID])
  medal_data.hint_kana      = item[COL_HINT_KANA]
  medal_data.hint_kanzi     = item[COL_HINT_KANZI]
  medal_data.explanation_kana   = item[COL_EXPLANATION_KANA]
  medal_data.explanation_kanzi  = item[COL_EXPLANATION_KANZI]

  unless item[COL_HINT_COMMENT] == nil then
    medal_data.comment = item[COL_HINT_COMMENT]
  else
    medal_data.comment = ""
  end

  medal_data_list << medal_data
}

# ======================================================
# エラーチェック
# ======================================================

# 重複チェック

# 大文字チェック

#=======================================================
#
# タイプヘッダファイル生成
#
#=======================================================
medal_type_def = open( ID_FILE_NAME ,"w")
medal_type_def.puts MDLID_CMT % ID_FILE_NAME
medal_data_list.each_with_index{ |item,index|
  # indexを3桁で表示している=>%03d"%index
  medal_type_def.puts '#define ' + item.id + "\t("+ index.to_s + ')' + "\t// " + item.medal_name
}
medal_type_def.puts '#define MDLID_MAX'  + "\t("+ medal_data_list.size.to_s + ')' + "\t// メダル最大数"
medal_type_def.puts MDLREC_CMT_END
medal_type_def.close

#=======================================================
#
# メダルレコードヘッダファイル生成
#
#=======================================================
medal_rec_id_def = open( MDLREC_FILE_NAME ,"w")
medal_rec_id_def.puts MDLREC_CMT % MDLREC_FILE_NAME
no_define_cnt = 10000 # 未定義レコードID数
medal_rec_id_def.puts "#define\tMDLRECID_START\t( " + no_define_cnt.to_s + " )"
medal_data_list.each_with_index{ |item,index|
  unless RECORD_H_HASH.key?(item.record_id) then
    medal_rec_id_def.puts "#define\t" + item.record_id + "\t( " + no_define_cnt.to_s + " )"
    RECORD_H_HASH[ item.record_id  ] = no_define_cnt
    no_define_cnt+=1
  end
}
medal_rec_id_def.puts "#define\tMDLRECID_MAX\t( " + no_define_cnt.to_s + " )"
medal_rec_id_def.puts ""
medal_rec_id_def.close

#=======================================================
#
# ソートヘッダファイル生成
#
#=======================================================
medal_sort_def = open( SORT_FILE_NAME ,"w")
medal_sort_def.puts MDLSORT_CMT % SORT_FILE_NAME
kana_data_list = Marshal.load(Marshal.dump(medal_data_list)); # deep copy

kana_data_list.each { |item|
  item.medal_name = item.medal_name.tr('ァ-ン', 'ぁ-ん')
}

sort_list = kana_data_list.sort_by{|item| item.medal_name }
sort_list.each_with_index{ |item,index|
  medal_sort_def.puts "    " + item.id + ",\t//" + item.medal_name
}
medal_sort_def.puts "};"
medal_sort_def.close

#=======================================================
#
# データファイルの生成
#
#=======================================================
=begin
u32		target_count		目標数																				
u8		cmp_type				比較データタイプ(イベントフラグを見るか、メダル用RECORDIDを使うか）																				
s32		cmp_id					比較対象のイベントフラグIDもしくはレコードID																				
u8		medal_type			メダルグラフィックタイプ																				
u8		hint_type				ヒント開示条件																				
u8		hint_level			ヒントレベル																				
u8		padding[3]																									
=end

medal_data_list.each_with_index{ |item,index|
  puts CNV_DIR+"medal_"+"%03d"%index+".bin"
  medal_bin = open( CNV_DIR+"medal_"+"%03d"%index+".bin", "wb" )

  #=====================
  # データの書き込み
  medal_bin << [ item.count.to_i ].pack( "I" )  # 目標数
  
  # ハッシュデータに存在するか判断
  unless RECORD_H_HASH.key?(item.record_id) then
    raise "ハッシュに定義されていないレコードIDが登録されています:"+item.record_id
  end
  medal_bin << [ RECORD_H_HASH[item.record_id] ].pack( "i" )  # レコードIDを解析した値を入れる必要がある

  if item.record_id.include?("MDLRECID_") then
    medal_bin << [ 1 ].pack( "C" )              # 比較データタイプ 
  else
    medal_bin << [ 0 ].pack( "C" )              # 比較データタイプ 
  end

  medal_bin << [ item.graphic_id ].pack( "C" ) # メダルグラフィックID
  medal_bin << [ GetHintConditionID(item.hint_condition) ].pack( "C" ) # ヒント開示条件
  medal_bin << [ item.hint_level.to_i ].pack( "C" ) # ヒントレベル
  medal_bin.close
}

#=======================================================
#
# メダル名GMM生成
#
#=======================================================
gmm = GMM.new
gmm.open_gmm( TMPGMM_NAME , MEDALNAMEGMM_NAME )
type_cnt = Hash.new
medal_data_list.each_with_index{ |item,index|
  # メダルタイプの取得
  mdl_type = item.graphic_type.gsub(/_LV./,"")
  
  # ハッシュに未登録のmdl_typeの場合は初期化
  unless type_cnt.key?(mdl_type) then
    type_cnt[mdl_type] = 0
  end
  type_cnt[mdl_type] += 1

  # GMMに書き込み
  gmm.make_row_index_kanji( "MDL_NAME_" + mdl_type + "_",type_cnt[mdl_type], item.medal_name , item.medal_name )
}
# ヒントメダル名を最後に出力
gmm.make_row_index_kanji( "MDL_NAME_HINT_",0, "ヒントメダル" , "ヒントメダル" )

gmm.close_gmm

#=======================================================
#
# メダル説明GMM生成
#
#=======================================================
gmm = GMM.new
gmm.open_gmm( TMPGMM_NAME , MEDALEXPLANATIONGMM_NAME )
type_cnt = Hash.new

medal_data_list.each_with_index{ |item,index|
  # メダルタイプの取得
  mdl_type = item.graphic_type.gsub(/_LV./,"")
  
  # ハッシュに未登録のmdl_typeの場合は初期化
  unless type_cnt.key?(mdl_type) then
    type_cnt[mdl_type] = 0
  end
  type_cnt[mdl_type] += 1

  # GMMに書き込み
  gmm.make_row_index_kanji( "MDL_EXP_" + mdl_type + "_",type_cnt[mdl_type], item.explanation_kana ,item.explanation_kanzi )
}
gmm.close_gmm

#=======================================================
#
# メダルヒントGMM生成
#
#=======================================================
gmm = GMM.new
gmm.open_gmm( TMPGMM_NAME , MEDALHINTGMM_NAME )
type_cnt = Hash.new

medal_data_list.each_with_index{ |item,index|
  # メダルタイプの取得
  mdl_type = item.graphic_type.gsub(/_LV./,"")
  
  # ハッシュに未登録のmdl_typeの場合は初期化
  unless type_cnt.key?(mdl_type) then
    type_cnt[mdl_type] = 0
  end
  type_cnt[mdl_type] += 1

  # GMMに書き込み
  gmm.make_row_index_kanji( "MDL_HINT_" + mdl_type + "_",type_cnt[mdl_type], item.hint_kana ,item.hint_kanzi )
}
gmm.close_gmm


medal_csv.close
