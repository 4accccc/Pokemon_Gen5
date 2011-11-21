##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
###
##
##	@file		tutorial_data_cnv.rb
##	@brief  無限ダンジョンチュートリアルデータコンバータ
##	@author	Masayuki Onoue
##	@date		2011.07.27
##
###
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


require "csv"

# ○をTRUE,それ以外をFALSEに変換して出力する
def put_true_false( str , out_file )
  if str.include?("○") then
    out_file.print "    TRUE,\n"
  else
    out_file.print "    FALSE,\n"
  end
end

# データをcdatに出力する
def put_floor_data( struct_name , data_csv, out_file )

  last_index = 0
  out_file.printf "static const ROOM_DATA %s[ROOM_MAX] =\n{\n"%[struct_name]

  data_csv.each_with_index { |item,i|
    if item[0].include?("#STRUCTEND") then break end
    out_file.puts "  {"
    out_file.print "    "+item[1]+",\n"
    out_file.print "    "+item[2]+",\n"
    put_true_false( item[3] ,out_file )
    put_true_false( item[4] ,out_file )
    put_true_false( item[5] ,out_file )
    put_true_false( item[6] ,out_file )
    out_file.print "    "+i.to_s+",\n"
    out_file.puts "  },"
    last_index = i
  }
  out_file.print "};\n\n"
end

# データをcdatに出力する
def put_tr_data( struct_name1 ,struct_name2 , data_csv, out_file )

  n_tr_list_array = Array.new # 通常トレーナーリスト
  r_tr_list_array = Array.new # 回復トレーナーリスト

  # データ取得
  data_csv.each_with_index { |item,i|
    if item[0].include?("#STRUCTEND") then break end
    n_tr_list_array[i] = item[1].to_i.to_s
    r_tr_list_array[i] = item[2].to_i.to_s
  }
  
  # 通常トレーナ配列出力
  out_file.printf "static const u16 %s[INFINITY_DUNGEON_TRID_MAX] =\n{\n"%[struct_name1]
  n_tr_list_array.each_with_index { |item,i|
    out_file.print "  %d, // %d\n"%[item,i]
  }
  out_file.print "};\n\n"

   
  # 回復トレーナ配列出力
  out_file.printf "static const u16 %s[INFINITY_DUNGEON_TRID_MAX] =\n{\n"%[struct_name2]
  r_tr_list_array.each_with_index { |item,i|
    out_file.print "  %d, // %d\n"%[item,i]
  }
  out_file.print "};\n\n"
end

# データをenumの形式で出力する
def put_enum_data( enum_name ,suffix , data_csv, out_file )
  
  out_file.puts "typedef enum{"
  # データ取得
  data_csv.each_with_index { |item,i|
    if item[0].include?("#STRUCTEND") then break end
    out_file.puts "  %s = (%d), // %s"%[suffix+item[0],item[1].to_i,item[2]]
  }
  out_file.puts "} %s;"%[enum_name]
  
end

#define
AREA_NO_COLUMN_INDEX = 0
ROOM_TYPE_COLUMN_INDEX = 1
PROBABILITY_COLUMN_INDEX = 2

OUT_FILE          = ARGV[0]
TUTORIAL_DEF_FILE = ARGV[1]
CSV_FILE          = ARGV[2]

out_file = open( OUT_FILE ,"w" )

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン内部屋データ
 *	@author	Masayuki Onoue
 *	@date		2011.07.27
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS
out_file.puts header % [OUT_FILE]

# データの読み込み
data_csv = CSV.open( CSV_FILE ,"r")

# データの読み飛ばし
3.times{
  data_csv.shift
}

# データ出力(黒エリア1)
out_file.puts("// 黒 エリア1 部屋データ")
put_floor_data( "sc_tutorial_rooo_data_b1", data_csv ,out_file )
3.times{ data_csv.shift }

# データ出力(黒エリア2)
out_file.puts("// 黒 エリア2 部屋データ")
put_floor_data( "sc_tutorial_rooo_data_b2", data_csv ,out_file )
3.times{ data_csv.shift }

# データ出力(白エリア1)
out_file.puts("// 白 エリア1 部屋データ")
put_floor_data( "sc_tutorial_rooo_data_w1", data_csv ,out_file )
3.times{ data_csv.shift }

# データ出力(白エリア2)
out_file.puts("// 白 エリア2 部屋データ")
put_floor_data( "sc_tutorial_rooo_data_w2", data_csv ,out_file )
4.times{ data_csv.shift }

# トレーナーデータ出力(黒エリア1)
out_file.puts("// 黒 エリア1 トレーナーデータ")
put_tr_data( "sc_tutorial_tr_data_b1", "sc_tutorial_rectr_data_b1", data_csv ,out_file )
4.times{ data_csv.shift }

# トレーナーデータ出力(黒エリア2)
out_file.puts("// 黒 エリア2 トレーナーデータ")
put_tr_data( "sc_tutorial_tr_data_b2", "sc_tutorial_rectr_data_b2", data_csv ,out_file )
4.times{ data_csv.shift }

# トレーナーデータ出力(白エリア1)
out_file.puts("// 白 エリア1 トレーナーデータ")
put_tr_data( "sc_tutorial_tr_data_w1", "sc_tutorial_rectr_data_w1", data_csv ,out_file )
4.times{ data_csv.shift }

# トレーナーデータ出力(白エリア2)
out_file.puts("// 白 エリア2 トレーナーデータ")
put_tr_data( "sc_tutorial_tr_data_w2", "sc_tutorial_rectr_data_w2", data_csv ,out_file )
4.times{ data_csv.shift }
out_file.close

# チュートリアル用定義を出力
out_file = open( TUTORIAL_DEF_FILE ,"w" )
header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンTUTORIAL定義データ
 *	@author	Masayuki Onoue
 *	@date		2011.11.21
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS
out_file.puts header % [TUTORIAL_DEF_FILE]
put_enum_data( "INF_TUTORIAL_DEF_DATA","INF_TUTORIAL_",data_csv ,out_file )

out_file.close

