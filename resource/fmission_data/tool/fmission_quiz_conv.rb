#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フェスミッション 図鑑クイズデータコンバーター
#
# @data   11.11.18 
# @author miyuki iwasawa
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定


$KCODE = 'SJIS'

require "../../tools/hash/monsno_hash.rb"
require "../../tools/hash/special_mons_tbl.rb"
require "../../tools/hash/monstype_hash.rb"
require "../../tools/hash/chihou_zukanno_hash.rb"

require "tool/cdef_perser.rb"

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_demoid.h"  #出力ファイル名
DST_BRIEF_DAT = "各アニメテーブルを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_BRIEF_HEAD = "各アニメヘッダを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

DST_BRIEF_DATA_DEF_H  = "フェスミッションデータ　リテラル定義ファイル"
DST_NOTE_DATA_DEF_H = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

DST_BRIEF_MID_H  = "フェスミッションデータID定義ファイル"
DST_NOTE_MID_H = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

$type_hash = {
 "ノーマル"=>0,
 "格闘"=>1,
 "飛行"=>2,
 "毒"=>3,
 "地面"=>4,
 "岩"=>5,
 "虫"=>6,
 "ゴースト"=>7,
 "鉄"=>8,
 "鋼"=>8,
 "メタル"=>8,
 "炎"=>9,
 "水"=>10,
 "草"=>11,
 "電気"=>12,
 "エスパー"=>13,
 "氷"=>14,
 "ドラゴン"=>15,
 "悪"=>16,
}

LIST_TYPE_0 = 0
LIST_TYPE_1 = 1
LIST_TYPE_2 = 2
LIST_TYPE_3 = 3
LIST_TYPE_4 = 4
LIST_TYPE_5 = 5
LIST_TYPE_6 = 6
LIST_TYPE_7 = 7
LIST_TYPE_8 = 8 
LIST_TYPE_9 = 9
LIST_TYPE_10 = 10
LIST_TYPE_11 = 11
LIST_TYPE_12 = 12
LIST_TYPE_13 = 13
LIST_TYPE_14 = 14
LIST_TYPE_15 = 15
LIST_TYPE_16 = 16
LIST_ZENKOKU = 17 
LIST_LOCAL = 18
LIST_LEGEND = 19
LIST_MAX = 20
LIST_END = LIST_MAX-1 

arg = $*

$fpath_out_bin = arg[0]

class CMonsList
  attr :list

  def initialize
    @list = Array.new
  end
end

#====================================================
# 定義パース処理クラス
#====================================================
class CPokeFilter

  def initialize f_path
    @fpath_base = f_path
    
    @monsno_max = $monsno_hash.fetch('たまご')
    @monsno_end = @monsno_max-1

    @data = Array.new()

    for i in 0..LIST_END do
      @data << CMonsList.new()
    end
  end #initialzie

  #全国かローカルか など？
  def regist_search

    for i in 1..@monsno_end do
      name = $monsno_hash.index(i)  #ナンバーからモンスター名取得

      #幻ポケモンか？
      idx = $monsname_maboroshi_tbl.index(name)
      if idx != nil then
        print("pass->#{name}")
        next #幻ポケモンは無視
      end

      #タイプ
      type = $monstype_hash.fetch(i)
      if type[0] == type[1] then
        @data[type[0]+LIST_TYPE_0].list << i
      else
        @data[type[0]+LIST_TYPE_0].list << i
        @data[type[1]+LIST_TYPE_0].list << i
      end
      
      #地方か全国か？
      no = $chihou_zukanno_hash.fetch(i)
      if no == $NO_CHIHOU_ID then
        @data[LIST_LOCAL].list << i 
      end
      @data[LIST_ZENKOKU].list << i

      #伝説系か？
      idx = $monsname_legend_tbl.index(name)
      if idx != nil then
        @data[LIST_LEGEND].list << i
      end
    end
  end #regist_search

  def output
    buf = Array.new
    for i in 0..LIST_END do
      dat = @data[i]
      print("#{dat.list.size()}\n")
      
      fpath = "%s%02d.bin" % [@fpath_base, i]
      
      fp = File.open(fpath,"wb")

      for n in dat.list do
        buf[0] = n
        fp.write( buf.pack("S1") ) 
      end
      fp.close()
    end
  end

end #end CPokeFilter

def convert
  #モンスターリストデータ取得 
  c_filter = CPokeFilter.new($fpath_out_bin)
  c_filter.regist_search()
  c_filter.output()
end

convert()
