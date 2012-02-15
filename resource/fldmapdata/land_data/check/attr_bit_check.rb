require File.dirname(__FILE__) + "/attr_purser.rb"
require 'csv'

# 出力データ形式
class ATTR_USE_DATA
  attr_reader :file_name ,:attr_id_list ,:attr_bit_index_list

  def initialize( file_name, attr_id_list ,attr_bit_index_list )
    @file_name = file_name
    @attr_id_list = attr_id_list
    @attr_bit_index_list = attr_bit_index_list
  end
end

# 結果出力先
result_name = ARGV[0]
result_num_name = ARGV[1]

# ビット名
ATTR_BIN_NAME=[
  "進入不可(bit)",
  "水(bit)",
  "敵出現(bit)",
  "足跡有り(bit)",
  "水飛沫有り(bit)",
  "草揺れ有り(bit)",
  "映り込み有り(bit)",
  "影有り(bit)",
  "ライト有り(bit)",
]

# 3atファイルパス
ATTR_NAME_FILE = 'C:/home/swan_data3d/field/poke_swan.3at'

# 3atファイルが見つかるか
if FileTest.exist?(ATTR_NAME_FILE) == false then
  puts "[%s]が見つかりません。"%[ATTR_NAME_FILE]
  raise
end

# 3atファイル解析
attr_type_name = Array.new 
File.open(ATTR_NAME_FILE,"r").each{|line|
  split_data = line.split("\s")
  
  if split_data.size >= 4 then
    no  = split_data[3].to_i
    name = split_data[4]
    attr_type_name[no] = name
  end
}

output_data = Array.new

#ディレクトリ内binファイルリストアップ
bin_ary = Dir.glob(File.dirname(__FILE__)+"/../land_res/*.bin")
bin_ary.each{ | file_name |
 
  # 進捗表示
  puts file_name

  # アトリビュート読み込み
  attr_data = ATTR_PURSER.new( file_name )

  # アトリビュートIDリスト
  attr_id_list = Array.new
  # ビット解析
  attr_bit_index_list = Array.new

  # 全データ解析
  attr_data.attr_tbl.each{|data|
    # アトリビュートIDをリスト化
    attr_id_list  << data.attr_id
    # ビットをインデックス化
    data.attr_bit.each_with_index{|bit,index|
      if bit == 1 then
        attr_bit_index_list << index
      end
    }
  }

  # 出力形式データに整形
  output_data << ATTR_USE_DATA.new( file_name,attr_id_list ,attr_bit_index_list )
}

# 全データ出力
CSV.open(result_name,"w"){|writeer|
  
  # ヘッダーデータ作成
  header = Array.new
  header <<  "ファイル名"
  # ビット名登録
  ATTR_BIN_NAME.each{|bit_name|
      header << bit_name
  }
  # アトリビュート名登録
  attr_type_name.each{|attr_name|
    unless attr_name.include?("non") then
      header << attr_name
    end
  }

  writeer << header
  # 全データ出力
  output_data.each{|data|
    line = Array.new
    line << File.basename(data.file_name)

    # ビットデータ出力
    ATTR_BIN_NAME.each_with_index{|bit_name,index|
      if data.attr_bit_index_list.include?(index) then
        line << "○"
      else
        line << "×"
      end
    }

    # アトリビュートID出力
    attr_type_name.size.times{|i|
      unless attr_type_name[i].include?("non") then
        if data.attr_id_list.include?(i) then
          line << "○"
        else
          line << "×"
        end
      end
    }

    writeer << line
  }
}

# 全データ出力(数出力)
CSV.open(result_num_name,"w"){|writeer|
  
  # ヘッダーデータ作成
  header = Array.new
  header <<  "ファイル名"
  # ビット名登録
  ATTR_BIN_NAME.each{|bit_name|
      header << bit_name
  }
  # アトリビュート名登録
  attr_type_name.each{|attr_name|
    unless attr_name.include?("non") then
      header << attr_name
    end
  }

  writeer << header
  # 全データ出力
  output_data.each{|data|
    line = Array.new
    line << "%s(%d)"%[File.basename(data.file_name),data.attr_id_list.size]

    # ビットデータ出力
    ATTR_BIN_NAME.each_with_index{|bit_name,index|
      include_cnt = 0
      data.attr_bit_index_list.each{|idx|
        if idx == index then
          include_cnt += 1
        end
      }
      line << include_cnt
    }

    # アトリビュートID出力
    attr_type_name.size.times{|i|
      unless attr_type_name[i].include?("non") then
        include_cnt = 0
        data.attr_id_list.each{|idx|
          if idx == i then
            include_cnt += 1
          end
        }
        line << include_cnt
      end
    }

    writeer << line
  }
}

