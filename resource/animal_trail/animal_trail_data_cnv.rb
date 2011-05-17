##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		animal_trail_data_cnv.rb
##	@brief  けものみちデータコンバータ
##	@author	Masayuki Onoue
##	@data		2011.05.11
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require File.dirname(__FILE__) + '/rbdef.rb'
require File.dirname(__FILE__) + '/item_hash.rb'
require File.dirname(__FILE__) + '/monsno_hash.rb'
require "csv"

##================================
# データクラス
##================================
class POKEMON_DATA
  attr_accessor :id ,:name ,:max_lv ,:min_lv ,:female_probability ,:forum
end

class ITEM_DATA
  attr_accessor :id ,:name
end

class ANIMAL_TRAIL_DATA
  attr_accessor :pokemon ,:item ,:item_hide ,:probability
  def initialize
    @item       = Array.new(ITEM_RANK_CNT)
    ITEM_RANK_CNT.times{|i|
      @item[i] = Array.new(ITEM_TABLE_CNT)
    }
    @item_hide  = Array.new(HIDE_ITEM_RANK_CNT)
    HIDE_ITEM_RANK_CNT.times{|i|
      @item_hide[i] = Array.new(HIDE_ITEM_TABLE_CNT)
    }
    @pokemon    = Array.new(POKE_RANK_CNT)
    POKE_RANK_CNT.times{|i|
      @pokemon[i] = Array.new(POKE_TABLE_CNT)
    }
    @probability = Array.new(TABLE_PROBABILITY-1)
  end
end

##================================
# データ表示関数
##================================
def put_animal_trail_data( out_file ,data ,index )
  out_file.puts "  // けものみち%d"%index
  out_file.puts "  {"

  # 抽選データ
  out_file.print "    { "
  data.probability.each{|probability|
    out_file.print probability.to_s+","
  }
  out_file.puts "}, // テーブルごとの抽選確率"

  # ポケモンデータ
  out_file.puts "    {"
  data.pokemon.each{|pt|
    out_file.puts "    {"
    #ID
    out_file.puts "      {"
    pt.each{|pd|
      out_file.puts "        " + pd.id.to_s + ",\t// " + pd.name
    }
    out_file.puts "      },"
    #max_lv
    out_file.print "      { "
    pt.each{|pd|
      out_file.print pd.max_lv.to_s + ", "
    }
    out_file.puts "}, // 最大Lv"
    #min_lv
    out_file.print "      { "
    pt.each{|pd|
      out_file.print pd.min_lv.to_s + ", "
    }
    out_file.puts "}, // 最小Lv"
    #female_probability
    out_file.print "      { "
    pt.each{|pd|
      out_file.print pd.female_probability.to_s + ", "
    }
    out_file.puts "}, // メスになる確率"
    #forum
    out_file.print "      { "
    pt.each{|pd|
      out_file.print pd.forum.to_s + ", "
    }
    out_file.puts "}, // フォルム"
    out_file.puts "    },"
  }
  out_file.puts "    },"
 
  # 通常アイテムデータ
  out_file.puts "    { // 通常アイテムデータ"
  data.item.each{|it|
  out_file.puts "      {"
    it.each{|item_data|
      out_file.puts "        " + item_data.id.to_s + ",\t// " + item_data.name
    }
  out_file.puts "      },"
  }
  out_file.puts "    },"

  # 隠しアイテムデータ
  out_file.puts "    { // 隠しアイテムデータ"
  data.item_hide.each{|it|
  out_file.puts "      {"
    it.each{|item_data|
      out_file.puts "        " + item_data.id.to_s + ",\t// " + item_data.name
    }
  out_file.puts "      },"
  }
  out_file.puts "    },"

  out_file.puts "  },"
 
end

##================================
# main
##================================
dir_name = ARGV[0]
in_data_name = ARGV[1]
data_cnt = ARGV[2].to_i
out_data_name = ARGV[3]

animal_trail_data_list = Array.new(data_cnt)

data_cnt.times{|index|
  puts dir_name+"/"+in_data_name+index.to_s+".csv"
  data = CSV.readlines(dir_name+"/"+in_data_name+index.to_s+".csv")

  animal_trail_data_list[index] = ANIMAL_TRAIL_DATA.new
  animal_trail_data_list[index] = animal_trail_data_list[index]
  # 確率テーブル取得(テーブルの最後は100%固定なので取得しない)
  (TABLE_PROBABILITY-1).times{|i|
    animal_trail_data_list[index].probability[i] = data[PROBABILITY_ROW+i][PROBABILITY_LINE]
  }

  # アイテムテーブル1
  ITEM_RANK_CNT.times{|rank|
    ITEM_TABLE_CNT.times{|i|
      animal_trail_data_list[index].item[rank][i] = ITEM_DATA.new
      animal_trail_data_list[index].item[rank][i].name = data[ITEM1_ROW+i+NEXT_TABLE_POS*rank][ITEM1_LINE]
      if $item_hash.include?(animal_trail_data_list[index].item[rank][i].name) == FALSE then
        puts "???"+animal_trail_data_list[index].item[rank][i].name+"???"
        raise "シート%d:未登録のアイテム名が使用されています"%(index+1)
      end
      animal_trail_data_list[index].item[rank][i].id = $item_hash[animal_trail_data_list[index].item[rank][i].name]
    }
  }

  # アイテムテーブル(隠し)1
  HIDE_ITEM_RANK_CNT.times{|rank|
    HIDE_ITEM_TABLE_CNT.times{|i|
      animal_trail_data_list[index].item_hide[rank][i] = ITEM_DATA.new
      animal_trail_data_list[index].item_hide[rank][i].name = data[ITEM1_HIDE_ROW+i+NEXT_TABLE_POS*rank][ITEM1_HIDE_LINE]
      if $item_hash.include?(animal_trail_data_list[index].item_hide[rank][i].name) == FALSE then
        puts "???"+animal_trail_data_list[index].item_hide[rank][i].name+"???"
        raise "シート%d:未登録のアイテム名が使用されています"%(index+1)
      end
      animal_trail_data_list[index].item_hide[rank][i].id = $item_hash[animal_trail_data_list[index].item_hide[rank][i].name]
    }
  }
  
  # ポケモンテーブル1
  POKE_RANK_CNT.times{|rank|
    POKE_TABLE_CNT.times{|i|
      animal_trail_data_list[index].pokemon[rank][i] = POKEMON_DATA.new
      animal_trail_data_list[index].pokemon[rank][i].name   = data[POKE1_ROW+i+NEXT_TABLE_POS*rank][POKE1_LINE+0]
      animal_trail_data_list[index].pokemon[rank][i].max_lv = data[POKE1_ROW+i+NEXT_TABLE_POS*rank][POKE1_LINE+1]
      animal_trail_data_list[index].pokemon[rank][i].min_lv = data[POKE1_ROW+i+NEXT_TABLE_POS*rank][POKE1_LINE+2]
      animal_trail_data_list[index].pokemon[rank][i].female_probability = data[POKE1_ROW+i+NEXT_TABLE_POS*rank][POKE1_LINE+3]
      animal_trail_data_list[index].pokemon[rank][i].forum = data[POKE1_ROW+i+NEXT_TABLE_POS*rank][POKE1_LINE+4]

      if $monsno_hash.include?(animal_trail_data_list[index].pokemon[rank][i].name) == FALSE then
        puts "???"+animal_trail_data_list[index].pokemon[rank][i].name+"???"
        raise "シート%d:未登録のモンスター名が使用されています"%(index+1)
      end
      animal_trail_data_list[index].pokemon[rank][i].id = $monsno_hash[animal_trail_data_list[index].pokemon[rank][i].name]
    }
  }
}

# ファイルへの書き込み
out_file = open( out_data_name ,"w" )
out_file.puts $header % [out_data_name]

data_cnt.times{|index|
  put_animal_trail_data( out_file ,animal_trail_data_list[index] ,index )
}
 
out_file.puts "};"
out_file.close

