#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フェスミッション 獣道ミッション用データコンバーター
#
# @data   11.11.10 
# @author onoue_masayuki
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定


$KCODE = 'SJIS'

require File.dirname(__FILE__) + './../../personal/monsno_hash.rb'
require "tool/cdef_perser.rb"
require "csv"

#====================================================
# 定数宣言
#====================================================

TRAIL_DATA_CDAT_FNAME = ARGV[0]
TRAIL_DATA_DEF_FNAME  = ARGV[1]
MONS_NO_DEF_FNAME     = ARGV[2]
TRAIL_DATA_SRC_FNAME  = ARGV[3]

COLNO_DATA_NO = 0
COLNO_DATA_ID = 1
COLNO_MONS_ID = 2
COLNO_FORM_NO = 3

# 拡張パラメータプレフィックス
IDX_PREFIX = "FMISSION_EXP_"

# ヘッダ
HEADER_DATA =<<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  フェスミッション：隠し穴大調査用データ
 *	@author	Masayuki Onoue
 *	@date		2011.11.10
 *  @note   %sが出力しています
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

static const FES_TRAIL_DATA_SET DATA_TrailMonsTbl[FMISSION_EXP_TRAIL_MONS_MAX] = {
EOS

# データ
DATA_TEMPLATE = '  { %3d , %3d },   // %s'


#====================================================
# テーブル解釈クラス
#====================================================
class CTrailTbl

  # コンストラクタ
  def initialize data_def_fname ,monsno_def
    # データ初期化
    @table_key = nil
    # 定義値取得
    data_def_name =  File.basename(data_def_fname,".h")
    mons_def_name =  File.basename(data_def_fname,".h")
    @c_monsiddef_list = CDef_Perser.new( data_def_fname , data_def_name+"_log.txt" )
    @c_monsnodef_list = CDef_Perser.new( monsno_def , mons_def_name+"_log.txt" )
    # 構造体作成
    @monsdata = Struct.new("MonsData", :mons_no, :form_no )
    # データ配列
    @monsdata_arry = Array.new
  end

  # モンスターテーブル解釈
  def mons_table_list src_data
    # データインデックス検査
    index = @c_monsiddef_list.searchID( IDX_PREFIX+src_data[COLNO_DATA_ID] )
    if index.nil? then
      raise src_data[COLNO_DATA_ID]+":不正なモンスターデータインデックスです"
    end
    # モンスターNO検査
    monsid = @c_monsnodef_list.searchID(src_data[COLNO_MONS_ID])
    if monsid.nil? then
      if $monsno_hash.key?(src_data[COLNO_MONS_ID]) then
        monsid = $monsno_hash[src_data[COLNO_MONS_ID]]
      else
        raise src_data[COLNO_MONS_ID]+":不正なモンスターIDです"
      end
    end

    # フォルムNO検査
    form_no = 0
    if src_data[COLNO_FORM_NO] =~ /(\d)/ then
      form_no = $1
    else
      form_no = @c_monsnodef_list.searchID(src_data[COLNO_FORM_NO])
      if form_no.nil? then
        raise src_data[COLNO_FORM_NO]+":不正なフォルム番号です"
      end
    end
    # データ格納
    data = @monsdata.new
    data.mons_no  = monsid
    data.form_no  = form_no
    @monsdata_arry[index] = data
  end

  # データ取り込み
  def line_import line
      
    # キー判定
    if key_check( line ) == false then
      
      # テーブル解釈
      unless @table_key.nil? then
        case @table_key
        when "#LIST0"
          mons_table_list line
        end
      end

    end
    
  end

  # キー解釈(キーラインの場合はtrue)
  def key_check line

    # キーなし
    if line[0].nil? then
      @table_key = nil
      return false
    end

    # キー判断
    if line[0].include?("#") then
      if line[0].include?("#LIST_END") then
        @table_key = nil
      else
        @table_key = line[0]
      end
      return true
    end
      
    return false
  end

  # モンスターデータ出力
  def output_mons_data fname

    # データ数の整合性チェック
    mons_num = @c_monsiddef_list.searchID( "FMISSION_EXP_TRAIL_MONS_MAX" )
    if mons_num != @monsdata_arry.size then
      raise "trailシートに定義してあるモンスター数が、リストに定義してある数と異なっています\n"
    end

    # 書き込み
    fp = File.open( fname, "w" )
    fp.puts HEADER_DATA%[fname,File.basename(__FILE__)]
    @monsdata_arry.size.times{|i|
      no = @monsdata_arry[i].mons_no
      form_no = @monsdata_arry[i].form_no
      fp.puts DATA_TEMPLATE%[ no , form_no ,$monsname[no]]
    }
    fp.puts "};"
    fp.close
  end

end

#====================================================
# メイン処理
#====================================================

# 格納先
table = CTrailTbl.new( TRAIL_DATA_DEF_FNAME ,MONS_NO_DEF_FNAME )
# 格納データ
src_data = CSV.readlines( TRAIL_DATA_SRC_FNAME)

# モンスターテーブル解釈
src_data.each{|line|

  # キー取得
  unless line[0].nil? then
    # 終了判定
    if line[0].include?("#END") then break end
  end

  # ラインデータ入力
  table.line_import( line )
}

# モンスターデータを出力する
table.output_mons_data( TRAIL_DATA_CDAT_FNAME )


