#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  same_pos_event_check.rb
#   @brief    同じ場所に配置されているイベントを洗い出す
#   @author		Masayuki Onoue
#   @date		  2011.01.30
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
require "mev_parser.rb"
require 'csv'

#ポジションタイプ定数
EVENTDATA_POS_TYPE_GRID = "EVENTDATA_POSTYPE_GRID"
EVENTDATA_POS_TYPE_RAIL = "EVENTDATA_POSTYPE_RAIL"

# チェックするmevのディレクトリ
PATH_TO_DIR     = "../data/"
TARGET_FILENAME = "*.mev"

# イベントデータ型定義
$pos_data = Struct.new("PosData", :mev_name, :label, :pos )
# same_pos_data
$same_data = Struct.new("SameData", :mev_name, :label1, :label2 )

# 結果出力先
result_file = "same_pos_event_check.csv"

# エラー配列
error_data_array = Array.new
 
# 全MEVをチェック
filenames = Dir[ PATH_TO_DIR + TARGET_FILENAME ]
filenames.each{|filename|
  puts filename
  mev_data = MEVFile.new
  mev_data.load( filename )

  # 位置データリスト
  pos_data_array = Array.new
 
  # =====================================
  # 位置データ収集
  mev_data.bg_section.event.each_with_index{|event,i|
    pos = event.position.split("\s")
    pos_data_array << $pos_data.new( File.basename(filename), event.Event_Label, pos )
  }
  mev_data.obj_section.event.each_with_index{|event,i|
    pos = event.position.split("\s")
    pos_data_array << $pos_data.new( File.basename(filename), event.ID_name, pos )
  }

  for i  in (0..(pos_data_array.size-2))
    for n  in ((i+1)..(pos_data_array.size-1))
      if ((pos_data_array[i].pos[0] == pos_data_array[n].pos[0] ) &&
          (pos_data_array[i].pos[1] == pos_data_array[n].pos[1] ) &&
          (pos_data_array[i].pos[2] == pos_data_array[n].pos[2] ) ) then
        error_data_array << $same_data.new( File.basename(filename) ,pos_data_array[i].label,pos_data_array[n].label)
      end 
    end
  end

}

# =====================================
# データ出力
CSV.open(result_file, 'w') do |writer|
  writer << [ "mev名", "ラベルA" ,"ラベルB" ]
  # 検索結果出力
  error_data_array.each{|data|
    writer << [ data.mev_name ,data.label1 ,data.label2 ]
  }
end

