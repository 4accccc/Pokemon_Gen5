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
def make_row( gmm, data )
  cnt  =0;
  
  data.each{|i|
    #空を見つけたら抜ける
    if(i == nil )
      break
    end
    
    row_name = sprintf("search_abc_%02d", cnt)
    row_data = i
    gmm.make_row_kanji( row_name, row_data, row_data )
    
    cnt += 1
  }
end

#====================================================
# GMM 吐き出し
#====================================================
def make_gmm( gmm_name, data )
  gmm = GMM::new
  gmm.open_gmm( GMM_SOURCE, DST_GMM_PATH + gmm_name )
  make_row( gmm, data )
  gmm.close_gmm
end

#====================================================
# リソースから文字列を抽出
#====================================================
def str_trim( str )
  start = str.index("//") + 2
  finish = str.index("\n") - 1

  str = str[start..finish]

  # カタカナをひらがなに
  # 濁点を外す
  origin = "ａ-ｚＡ-Ｚァ-ン０-９ぁぃぅぇぉがぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽゃゅょっゎァィゥェォガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポャュョッヮ"
  normalize = "a-zA-Zぁ-ん0-9あいうえおかきくけこさしすせそたちつてとはひふへほはひふへほやゆよつわあいうえおかきくけこさしすせそたちつてとはひふへほはひふへほやゆよつわ"
  str = str.tr(origin,normalize);

  return str
end


#多次元配列
def make_nm_array(n,m)
  (0...n).map { Array.new(m) }
end

$filenum = 0
$data = make_nm_array( 50, 200 )

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
          #余分な文字列を除去
          line = str_trim( line )
          #項目出力
          $data[$filenum][row_idx] = line
          #要素を進める
          row_idx += 1;
          
        end
      elsif line.include?("u16 PMS_InitialTable_")
        #項目検索
        output_flag = 1
        row_idx = 0
      end
    }
  }
end

#====================================================
# 主処理 開始
#====================================================

# データ解析
load_resource

# GMM出力
cnt = 0
$data.each{|data|
  #空を見つけたら抜ける
  if(data[0] == nil )
    break
  end

  name = "pms_abc" + sprintf("%02d",cnt) + ".gmm"
  make_gmm( name, data )
  
  cnt += 1
}

#ヘッダファイル出力
DST_FILENAME = "pms_abc_gmm_def.h"  #出力ファイル名
DST_BRIEF = "50音並替用のデータテーブルです。"
DST_AUTOR = "genya_hosaka"
DST_NOTE = "このファイルはpms_gmm_conv.rbによって自動生成されたものです。"
DST_MAX_DEF = "PMS_ABC_GMMTBL_MAX"

File::open( DST_FILENAME ,"w"){ |file|
    file.puts("//=========================================================================");
    file.puts("/**");
    file.puts(" * @file\t" + DST_FILENAME);
    file.puts(" * @brief\t" + DST_BRIEF);
    file.puts(" * @autor\t" + DST_AUTOR);
    file.puts(" * @note\t" + DST_NOTE);
    file.puts(" */");
    file.puts("//=========================================================================\n");
    file.puts("#pragma once\n\n");
    file.puts("#include \"message.naix\"\n\n");

    file.puts("#define " + DST_MAX_DEF + " (" + $filenum.to_s + ") // 最大数\n\n" );

    file.puts("static const u16 pms_abc_gmmtbl[ " + DST_MAX_DEF + " ] = {")
    
    #データテーブル出力

    for cnt in 0..$filenum-1
      line = "\tNARC_message_pms_abc" + sprintf("%02d",cnt) + "_dat,";
      file.puts( line );
    end
    
    file.puts("};");
    
}
