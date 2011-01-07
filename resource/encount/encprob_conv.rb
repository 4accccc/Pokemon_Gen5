#===============================================================
# Swanエンカウント率制御データテーブルコンバータ
# 09.12.01 iwasawa
#===============================================================

load "encount_common.def"
#require 'jstring'
$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require "../../tools/hash/monsno_hash.rb"

arg = $*
$input_f = arg[0]
$output_f = arg[1]

##############################################
#構造体定義
$_enc_prob = Struct.new("EncProb", :comment, :up, :interval, :max, :safety )

##############################################
#グローバル変数定義

$header_buf = "/*\n\
\t@file\t%s\n\
\t@brief\tエンカウント率制御パターンデータテーブル\n\
\t@author\tこのファイルはコンバータから出力されています\n\n\
\tfrom resource/encount/encprob_conv.rb\n\
*/\n\
#pragma once\n\n"

#エンカウント率制御データパターンテーブル
class CEncProbData
  @prob_list

  #初期化
  def initialize
    @prob_list = Array.new
  end

  #アーカイブリスト出力
  def output f_path
    File.open(f_path,"w"){ |file|
      file.print( $header_buf % [f_path])

      file.print("static const ENCPROB_PATTERN DATA_EncProbPattern[] = {\n")
      file.print(" //up, interval, max, safety\n")
      for n in @prob_list do
        buf = " { %s, %d, %d, %d },  //%s\n" % [n.up, n.interval, n.max, n.safety, n.comment]
        file.print(buf)
      end
      file.print("};\n\n")
    }
  end

  #ファイルロード
  def src_load f_path
    File.open(f_path){ |file|
     file.gets  #1行読み飛ばし
     while line = file.gets
       line = line.chomp #行末改行を取り除く
       work = line.split(",")

       @prob_list << $_enc_prob.new
       cp = @prob_list.last()

       cp.comment = work[0]
       cp.up = work[1].to_i
       cp.interval = work[2].to_i
       cp.max = work[3].to_i
       cp.safety = work[4].to_i
     end
    }
  end
end

#---------------------------------------------
# コンバートメイン
#---------------------------------------------
def convert
  #コンバートメイン
  c_enc = CEncProbData.new()
  c_enc.src_load( $input_f )
  c_enc.output( $output_f )
end

#実行
convert

