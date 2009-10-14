#===============================================================
# WBエンカウントデータコンバータ
# 09.09.27 iwasawa
#===============================================================

load "encount_common.def"
#require 'jstring'
$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require "../../tools/hash/monsno_hash.rb"

arg = $*

##############################################
#構造体定義
$_enc_poke = Struct.new("EncPoke", :monsno, :form, :lv_min, :lv_max)
#エンカウトモンスターデータ構造体長
ENC_MONS_DATA_SIZ = 4

##############################################
#グローバル変数定義

#出力パス
$output_path = [WFBIN_DIR,BFBIN_DIR]
#各種データテーブル長
$table_num = [GROUND_MONS_NUM,GROUND_MONS_NUM,GROUND_MONS_NUM,WATER_MONS_NUM,WATER_MONS_NUM,WATER_MONS_NUM,WATER_MONS_NUM]


#エンカウントテーブル
class CEncTable
  MONSNO_BIT_NUM = 11
  FORM_BIT_NUM = 5

  @zone_name

  @prob
  @list_num
  @mons_list

  #初期化
  def initialize zone_name
    @zone_name  = zone_name

    @prob = 0
    @list_num = 0
    @mons_list = Array.new
  end

  #データエラーチェック
  def check_disable column,enc_prob,dat_len,null_check_len,dat_name
    #データの有無チェック
    if enc_prob == 0 then
      #読み飛ばして終了
      column.slice!(0..(dat_len-1))
      return true
    end
    #エラーチェック
    null_idx = column.index("")
    if null_idx != nil && null_idx < null_check_len then
      printf("%s データエラー %d列目に不正な空欄があります <= %s 確率=%d\n",@zone_name,null_idx,dat_name,enc_prob)
      exit 1 
    end
    return false
  end

  #フォルム取得
  def form_get str_form
    form = str_form == "" ? 0 : str_form.to_i
    return form
  end

  #グランドデータ数値パース(破壊的メソッドなので注意)
  def perse_ground_val column,enc_prob,dat_name
    if check_disable(column,enc_prob,GROUND_MONS_DATA_LEN,GROUND_MONS_NUM,dat_name) == true then
      return 
    end

    #lvとformの取得
    GROUND_MONS_NUM.times{ |i|
      @mons_list << $_enc_poke.new 
      cp = @mons_list.last()
      cp.lv_min = cp.lv_max = column[i].to_i 
      cp.form = form_get(column[i+GROUND_MONS_NUM])
    }
    column.slice!(0..(GROUND_MONS_DATA_LEN-1))
  end

  #水上データ数値パース
  def perse_water_val column,enc_prob,dat_name
    if check_disable(column,enc_prob,WATER_MONS_DATA_LEN,WATER_MONS_NUM*2,dat_name) == true then
      return 
    end

    #lvとformの取得
    WATER_MONS_NUM.times{ |i|
      @mons_list << $_enc_poke.new 
      cp = @mons_list.last()
      cp.lv_min = column[i].to_i 
      cp.lv_max = column[i+1].to_i 
      cp.form = form_get(column[i+(WATER_MONS_NUM*2)])
    }
    column.slice!(0..(WATER_MONS_DATA_LEN-1))
  end

  #モンスター名パース
  def perse_monsno column,data_num
    if @mons_list.size == 0 then
      column.slice!(0..(data_num-1))
      return
    end

    data_num.times{ |i|
      pl = @mons_list[i]
      pl.monsno = $monsno_hash.fetch(column[i])
      print(column[i]+",")
      if pl.monsno == nil then
        printf("%s モンスター名エラー %s\n",@zone_name,column[i])
        exit 1
      end
    }
    column.slice!(0..(data_num-1))
    print "\n"
  end

  #エンカウントデータ出力 4byte
  def output_bin fp
    buf = Array.new
    buf.fill(0,0,4)
    for pl in @mons_list do
      buf[0] = ( pl.monsno | (pl.form << MONSNO_BIT_NUM)) 
      buf[1] = pl.lv_min
      buf[2] = pl.lv_max
      fp.write(buf.pack("S1C2"))
    end
  end

end

#1マップのエンカウントデータ
class CMapData
  @zone_name  #ゾーン名
  @enc_prob #エンカウント率テーブル
  @table  #データテーブル

  #初期化
  def initialize zone_name
    @zone_name = zone_name

    @enc_prob = Array.new
    @enc_prob.fill(0,0,IDX_MAX+1)

    @table = Array.new
    IDX_MAX.times{ |i|
      @table[i] = CEncTable.new(zone_name)
    }
  end

  #1マップのデータパース
  def perse column
    #エンカウント率取得
    @enc_prob[IDX_GROUND_L] = column[COL_ZONE_OFS_PROB_GROUND_L] == "" ? 0 : column[COL_ZONE_OFS_PROB_GROUND_L].to_i
    @enc_prob[IDX_GROUND_H] = column[COL_ZONE_OFS_PROB_GROUND_H] == "" ? 0 : column[COL_ZONE_OFS_PROB_GROUND_H].to_i
    @enc_prob[IDX_WATER] = column[COL_ZONE_OFS_PROB_WATER] == "" ? 0 : column[COL_ZONE_OFS_PROB_WATER].to_i
    @enc_prob[IDX_FISHING] = column[COL_ZONE_OFS_PROB_FISHING] == "" ? 0 : column[COL_ZONE_OFS_PROB_FISHING].to_i
    
    #配列の要素削除
    column.delete_at(COL_ZONE_OFS_PROB_GROUND_L)
    column.delete_at(COL_ZONE_OFS_PROB_GROUND_H)
    column.delete_at(COL_ZONE_OFS_PROB_WATER)
    column.delete_at(COL_ZONE_OFS_PROB_FISHING)

    print("\n"+@enc_prob.join(",")+"\n")
    
    #グランドデータvalueパース
    @table[IDX_GROUND_L].perse_ground_val(column,@enc_prob[IDX_GROUND_L],"通常1")
    @table[IDX_GROUND_H].perse_ground_val(column,@enc_prob[IDX_GROUND_H],"通常2")
    @enc_prob[IDX_GROUND_SP] = column[0] == "" ? 0 : 1
    @table[IDX_GROUND_SP].perse_ground_val(column,@enc_prob[IDX_GROUND_SP],"特殊")

    #水上・釣りデータvalueパース
    @table[IDX_WATER].perse_water_val(column,@enc_prob[IDX_WATER],"水上")
    @enc_prob[IDX_WATER_SP] = column[0] == "" ? 0 : 1
    @table[IDX_WATER_SP].perse_water_val(column,@enc_prob[IDX_WATER_SP],"特殊水上")

    @table[IDX_FISHING].perse_water_val(column,@enc_prob[IDX_FISHING],"釣り")
    @enc_prob[IDX_FISHING_SP] = column[0] == "" ? 0 : 1
    @table[IDX_FISHING_SP].perse_water_val(column,@enc_prob[IDX_FISHING_SP],"特殊釣り")
    
    #モンスター名パース
    IDX_MAX.times{ |i|
      @table[i].perse_monsno(column,$table_num[i])
    }
  end

  #エンカウント率出力 8byte
  def output_enc_prob fp
    fp.write(@enc_prob.pack("C8")) 
  end

  #エンカウントデータ出力
  def output_enc_data fp
    dmy_buf = Array.new
    dmy_buf.fill(0,0,ENC_MONS_DATA_SIZ*GROUND_MONS_NUM)

    IDX_MAX.times{ |i|
      if @enc_prob[i] == 0 then
        tmp  = "C" + $table_num[i].to_s
        fp.write(dmy_buf.pack(tmp))
        next
      end
      @table[i].output_bin(fp)
    }
  end

  #1マップデータ出力
  def output version
    path = $output_path[version] + "/" + @zone_name + ".bin" 
    fp = File.open(path,"wb")
    #エンカウント率出力
    output_enc_prob(fp)
    #エンカウントデータ出力
    output_enc_data(fp)
    fp.close()
  end
end

#1ゾーンのデータ
class CZoneData
  @zone_name  #ゾーン名
  @white      #ホワイトVerのデータ
  @black      #ブラックVerのデータ
  @v2_enable  #ブラックデータが有効かどうか

  #初期化
  def initialize
    @zone_name = ""
    @v2_enable = FALSE
  end #eof initialize

  #データ配列の走査
  def perse column
    @zone_name = column[COL_ZONE_NAME].downcase
    @v2_enable = column[COL_V2_ENABLE]  == "●" ? true : false
    
    column.slice!(COL_V2_ENABLE)  #フラグ部を取り除く
    column.slice!(0..COL_OFS_HEADER)  #ヘッダ部を取り除く

    white = column.slice!(0..COL_LEN_ZONE)
    black = column.slice!(0..COL_LEN_ZONE)

    @white = CMapData.new(@zone_name)
    @black = CMapData.new(@zone_name)

    @white.perse(white)
    if @v2_enable == false then
      return
    end
#    print black 
    @black.perse(black)
  end #eof perse

  #データ列の出力
  def output
    @white.output(VER_WHITE)
    if @v2_enable == true then
      @black.output(VER_BLACK)
    else
      @white.output(VER_BLACK)  #ホワイトのコピーを出力
    end
  end

  #ゾーン名取得
  def get_zone_name
    return @zone_name
  end

end #End of Class CMapData

#ポケモン
class CEncountData
  @zone_num
  @zone_list

  #初期化
  def initialize
    @zone_num = 0
    @zone_list = Array.new
  end

  #アーカイブリスト出力
  def output_arclist f_path,bin_path
    File.open(f_path,"w"){ |file|
      for n in @zone_list do
       buf = "\"%s/%s.bin\"\n" % [bin_path,n.get_zone_name()]
       file.print buf
      end
    }
  end

  #ファイルロード
  def src_load f_path
    File.open(f_path){ |file|
     while line = file.gets
       line = line.chomp #行末改行を取り除く
       line = line.gsub("\"","")  #"を取り除く
       work = line.split(",")

       @zone_list << CZoneData.new
       cp = @zone_list.last()
       #マップデータのパース
       cp.perse(work)
       #マップデータの出力
       cp.output()
     end
    }
  end
end

#---------------------------------------------
# コンバートメイン
#---------------------------------------------
def convert
  #コンバートメイン
  c_enc = CEncountData.new()
  c_enc.src_load("encount_wb.csv")
  c_enc.output_arclist(ARCW_LIST_FILENAME,WFBIN_DIR)
  c_enc.output_arclist(ARCB_LIST_FILENAME,BFBIN_DIR)
end

#実行
convert

