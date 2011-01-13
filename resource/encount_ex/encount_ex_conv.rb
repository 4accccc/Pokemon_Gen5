#===============================================================
# 特殊エンカウント(Nポケ)データコンバータ
# 11.01.07 iwasawa
#===============================================================

#require 'jstring'
$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require "../../tools/hash/monsno_hash.rb"
require "../../tools/hash/seikaku_hash.rb"

arg = $*
$input_f = arg[0]
$output_f = arg[1]

##############################################
#構造体定義
$_enc_poke = Struct.new("EncPoke", :no, :zone, :zone_num, :mons_no, :mons_name, :lv, :char, :char_name, :sex, :tokusei, :comment, :tokusei_com )
$_oya_prm = Struct.new("OyaParam", :id, :sex, :pow_rnd, :natsuki )

##############################################
#グローバル変数定義

$header_buf = "/*\n\
\t@file\t%s\n\
\t@brief\t特殊エンカウント(Nポケ)データテーブル\n\
\t@author\tこのファイルはコンバータから出力されています\n\n\
\tfrom resource/encount_ex/encount_ex_conv.rb\n\
*/\n\
#pragma once\n\n"

#エンカウント率制御データパターンテーブル
class CEncDataEx
  @poke_list
  @oya_prm

  #初期化
  def initialize
    @poke_list = Array.new
    @oya_prm = $_oya_prm.new
  end

  #アーカイブリスト出力
  def output f_path
    File.open(f_path,"w"){ |file|
      file.print( $header_buf % [f_path])

      file.print("static const ENCOUNT_DATA_N_POKE DATA_EncountDataNPoke[] = {\n")
      file.print(" //zone, zone_num, monsno, lv, char, sex, tokusei\n")
      for n in @poke_list do
        buf = " { ZONE_ID_%s, %d, %d, %d, %d, %s, %d },  //%s, %s, %s\n" % 
          [n.zone, n.zone_num, n.mons_no, n.lv, n.char, n.sex, n.tokusei, n.mons_name, n.char_name, n.tokusei_com]
        file.print(buf)
      end
      file.print("};\n\n")
    }
    header_path = File::basename(f_path,'.cdat');
    header_path += "_def.h"
    File.open(header_path,"w"){ |file|
      file.print( $header_buf % [header_path])
      file.print("#define ENC_N_POKE_MAX\t( %d )\n" % [@poke_list.length()])
      file.print("#define ENC_N_POKE_NONE\t( 0xFF )\n\n")
      
      file.print("//Ｎポケモン共通パラメータ\n")
      file.print("#define N_POKE_OYA_ID\t( 0x%08x )\n" % [@oya_prm.id])
      file.print("#define N_POKE_OYA_SEX\t( %s )\n" % [@oya_prm.sex])
      file.print("#define N_POKE_POW_RND\t( %d )\n" % [@oya_prm.pow_rnd])
      file.print("#define N_POKE_NATSUKI\t( %d )\n\n" % [@oya_prm.natsuki])
    }
  end

  def sex_conv value 
    case value 
    when "♂" then
      return "PM_MALE"
    when "♀" then
      return "PM_FEMALE"
    when "---" then
      return "PM_NEUTRAL"
    else
      print("ERROR! 性別指定が不正です -> #{value}\n")
      exit
    end
    return "PM_MALE"
  end

  #ファイルロード
  def src_load f_path
    File.open(f_path){ |file|
     #親情報取得
     file.gets  #1行読み飛ばし
     line = file.gets
     line.chomp!
     work = line.split(",")
     @oya_prm.id = work[1].to_i
     @oya_prm.sex = sex_conv(work[2])
     @oya_prm.pow_rnd = work[3].to_i
     @oya_prm.natsuki = work[4].to_i

     #ポケモン情報取得
     file.gets  #2行読み飛ばし
     file.gets 
     while line = file.gets
       line = line.chomp #行末改行を取り除く
       work = line.split(",")

       @poke_list << $_enc_poke.new
       cp = @poke_list.last()

       cp.no = work[0].to_i
       cp.mons_name = work[1]
       cp.mons_no = $monsno_hash.fetch( cp.mons_name )
       if cp.mons_no == nil then
         print("ERROR! モンスター名が不正です -> #{cp.mons_name}\n")
         exit 1
       end
       cp.zone = work[2]
       cp.zone_num = work[3].to_i
       cp.lv = work[4].to_i
       cp.char_name = work[5]
       cp.char = $seikaku_hash.fetch( cp.char_name )
       if cp.char == nil then
         print("ERROR! 性格名が不正です -> #{cp.char_name}\n")
         exit 1
       end
      
       cp.sex = sex_conv(work[6])
       
       cp.tokusei = work[7].to_i
       if cp.tokusei < 0 || cp.tokusei > 1 then
         print("ERROR! 特性は0or1<特性1or特性2>で指定してください\n")
         exit 1
       end
       cp.tokusei_com = work[8]
     end
    }
  end
end

#---------------------------------------------
# コンバートメイン
#---------------------------------------------
def convert
  #コンバートメイン
  c_enc = CEncDataEx.new()
  c_enc.src_load( $input_f )
  c_enc.output( $output_f )
end

#実行
convert

