#!/usr/bin/ruby
#===================================================================
#
# @brief  WBアイテム名リスト生成（50音ソート用データ)
#
# @data   09.10.10
# @author genya_hosaka
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
OUTPUT_ARC = 0    # 0=ヘッダ出力, 1=ARCで出力(u16ごとに生データ)

if OUTPUT_ARC == 1
  DST_ARCNAME = "itemsort_abc.dat"
else
  DST_FILENAME = "itemsort_abc_def.h"  #出力ファイル名
  DST_BRIEF = "50音並替用のデータテーブルです。"
  DST_AUTOR = "genya_hosaka"
  DST_NOTE = "このファイルはmakelist_sort_by_abc.rbによって自動生成されたものです。"
  DST_TBL_HEADER = "//50音昇順の優先度 添字:item_id \nstatic const u16 ItemSortByAbc[] ="
end

CSV_ROW_ID = 0 #IDの列
CSV_ROW_NAME = 2; #名前データの列

#===================================================================
# 主処理
#===================================================================
#----------------------------------------------------
# グローバル変数
#----------------------------------------------------
#取得データ保持用ハッシュテーブル
tbl = Hash.new
#idと昇順ソート結果を結びつけるハッシュテーブル
prio = Hash.new

#----------------------------------------------------
# 読み込み
#----------------------------------------------------

# CSVオープン
reader = CSV.open( 'wb_item.csv', 'r')

#一行スキップ
reader.shift

#文字列取り出し
reader.each { |row|
  tbl[ row[ CSV_ROW_NAME ].to_s ] = row[ CSV_ROW_ID ].to_i
}

#CSV クローズ
reader.close

# 昇順ソート
# "ァ-ン"を"ぁ-ん"として扱う
abc_sort = tbl.sort_by{|name, id| name.tr("ァ-ン","ぁ-ん") }

#idとソート結果を結びつけたハッシュを生成
count = 0;
abc_sort.each{|name,id|
  prio[id] = count;
  count += 1;
}

#NO順に並べる
no_sort = tbl.sort_by{|name, id| id }

#----------------------------------------------------
# 出力
#----------------------------------------------------

if OUTPUT_ARC == 1
  #生データで出力
  dstFile = File.open( DST_ARCNAME, "wb" ); 
  #データテーブル出力
  no_sort.each{ |name,id|
    dstFile.write( [prio[id]].pack("S") );
  }
  dstFile.close;
else
  #ヘッダファイルで出力
  File.open( DST_FILENAME ,"w"){ |file|

    #ヘッダ出力
    file.puts("//=========================================================================");
    file.puts("/**");
    file.puts(" * @file\t" + DST_FILENAME);
    file.puts(" * @brief\t" + DST_BRIEF);
    file.puts(" * @autor\t" + DST_AUTOR);
    file.puts(" * @note\t" + DST_NOTE);
    file.puts(" */");
    file.puts("//=========================================================================\n\n");
    file.puts(DST_TBL_HEADER);

    #データテーブル出力
    no_sort.each{|name,id|
      file.print("\t");
      file.print(prio[id]);
      file.print(",\t//No.");
      file.print(id);
      file.puts( " "+name )
    }

    file.puts("}");

    #比較できるようにしておくためにあいうえお順結果を出力
    file.puts("\n/*\n\t *** 50音順リスト ***\n\n");

    count=0;
    abc_sort.each{|name,id|
      file.print("  ");
      file.print(count);
      file.print("  ItemNo.");
      file.print(id);
      file.puts( "  "+name )
      count += 1;
    }
    file.puts("*/");
  }

end



