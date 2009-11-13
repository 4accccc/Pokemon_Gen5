#!/usr/bin/ruby
#===================================================================
#
# @brief  簡易会話50音GMM データ生成
#
# @data   09.10.10
# @author genya_hosaka
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定
$KCODE = 'SJIS'
# String クラスの tr メソッドで日本語を使用可能にする。
require "jcode"
# GMM生成クラス
require File.dirname(__FILE__) + '/../../tools/gmm_make/gmm_make'

#====================================================
# 定数宣言
#====================================================
GMM_SOURCE = "../message/template.gmm"

SRC_RESOURCE = "pms_input.res"
DST_GMM_PATH = "../message/src/pms/abc/"

#====================================================
# GMM 行書き込み
#====================================================
def make_row( gmm )
  row_name = sprintf("search_abc_%02d",1)
  row_data = "あいう"
  gmm.make_row_kanji( row_name, row_data, row_data )
end

#====================================================
# GMM 吐き出し
#====================================================
def make_gmm( gmm_name )
  gmm = GMM::new
  gmm.open_gmm( GMM_SOURCE, DST_GMM_PATH + gmm_name )
  make_row( gmm )
  gmm.close_gmm
end


#====================================================
# データ変換
#====================================================

#====================================================
# リソースから文字列を抽出
#====================================================
def line_trim

  
end


$filenum = 0
$data = Array.new

#====================================================
# リソース 読み込み
#====================================================
def load_resource
  output_flag = 0
  row_idx = 0
  
  File::open( SRC_RESOURCE ) {|f|
    f.each { |line|
      if output_flag == 1
        if line.include?("PMS_WORDID_END")
          #出力終了判定
          output_flag = 0
          $filenum += 1
        else
          #項目出力
          $data[$filenum,row_idx] = line
          row_idx += 1;
          
        end
      elsif line.include?("u16 PMS_InitialTable_")
        #項目検索
        #puts line
        output_flag = 1
        row_idx = 0
      end
    }
  }
end



# カタカナをひらがなに
# 濁点を外す


#====================================================
# 主処理 開始
#====================================================

# データ解析
load_resource

#$data = $data.sort_by{ |id,elem|
#  id.downcase
#}




# 出力

#TODO eachにかえる 
for i in 1..50
  name = "pms_abc" + sprintf("%02d",i) + ".gmm"
  make_gmm( name )
end







