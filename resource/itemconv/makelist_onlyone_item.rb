#!/usr/bin/ruby
#===================================================================
#
# @brief  ゲーム内で1つしか手に入らないアイテム(たいせつなもの、技マシン)と???の無効アイテムIDがTRUEのflagテーブルを出力
#
# @data   11.11.24
# @author iwasawa 
#
#===================================================================

# 文字コード指定
$KCODE = 'SJIS'

#CSV読み込み
require "csv"
# String クラスの tr メソッドで日本語を使用可能にする。
require "jcode"

#===================================================================
# 設定
#===================================================================
DST_FILENAME = "item_onlyone_flags.cdat"  #出力ファイル名

DST_BRIEF = "ゲーム内で1つしか手に入らないアイテムと???の無効アイテムがTRUEのu32[]flagテーブルです"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはmakelist_onlyone_item.rbによって自動生成されたものです。"
DST_TBL_HEADER_ONLYONE = "///ゲーム内で1つしか手に入らないアイテム\nstatic const u32 DATA_ItemOnlyOneFlagTbl[] = {"
DST_TBL_HEADER_DISABLE = "///データ設定のない無効アイテム\nstatic const u32 DATA_ItemDisableFlagTbl[] = {"

CSV_ROW_ID = 0 #IDの列
CSV_ROW_TYPE = 1  #分類の列
CSV_ROW_ICON_CHAR = 4 #アイコンキャラ名の列

#===================================================================
# 主処理
#===================================================================
#----------------------------------------------------
# グローバル変数
#----------------------------------------------------
$only_one_type = ["イベント","技マシン"]
$disable_name = "item_dumy"  #アイコンキャラ名がitem_dumyのものはSWANでは手に入らない

$out_file = DST_FILENAME

$onlyone_tbl = Array.new
$onlyone_tbl << 0 #0番は無効IDなのでフラグOffでいい

$disable_tbl = Array.new
$disable_tbl << 1 #0番はもともと無効ID

#----------------------------------------------------
# 読み込み
#----------------------------------------------------

def main
  # CSVオープン
  reader = CSV.open( 'wb_item.csv', 'r')

  #一行スキップ
  reader.shift

  #条件サーチ
  reader.each { |row|
    only_one_idx = $only_one_type.index( row[CSV_ROW_TYPE] )
    disable_name = row[CSV_ROW_ICON_CHAR].index($disable_name)

    if only_one_idx != nil then
      $onlyone_tbl << 1
    else
      $onlyone_tbl << 0
    end
    if disable_name != nil then
      $disable_tbl << 1
    else
      $disable_tbl << 0
    end
  }

  #CSV クローズ
  reader.close

  #----------------------------------------------------
  # 出力
  #----------------------------------------------------
  File.open( $out_file ,"w"){ |file|

    #ヘッダ出力
    file.puts("//=========================================================================");
    file.puts("/**");
    file.puts(" * @file\t" + $out_file);
    file.puts(" * @brief\t" + DST_BRIEF);
    file.puts(" * @autor\t" + DST_AUTOR);
    file.puts(" * @note\t" + DST_NOTE);
    file.puts(" */");
    file.puts("//=========================================================================\n");
    file.puts("#pragma once\n\n");

    out_tbl( file, DST_TBL_HEADER_ONLYONE, $onlyone_tbl )
    out_tbl( file, DST_TBL_HEADER_DISABLE, $disable_tbl )
  }
end #main

def out_tbl file, header, flags
  file.puts( header )

  data_len = flags.size()
  buf_len = data_len/32
  if (data_len % 32) != 0 then
    buf_len += 1  #割り切れなければ+1
  end

  for i in 0..(buf_len-1) do
    idx = i*32
    buf = 0

    for j in 0..31 do
      no = idx+j
      if no < data_len then
        buf |= (flags[no]<<j)
      end
    end
    str = "  0x%08x,  ///<%d - %d\n" % [buf,idx,(idx+31)]
    file.print( str )
  end
  file.puts("};\n");
end #out_tbl

main()
