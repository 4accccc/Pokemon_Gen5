#================================================================================
#
#   特性名の並びをあらわすためのテーブル生成
#
#   2011.02.05  tamada@gamefreak
#
#   
#   tools/tokusei/tokusei_conv.rb で生成されるspeabi.txtとtokusei_hash.rbに依存している
#
#================================================================================
require "jcode"
$KCODE="s"

require 'tokusei_hash.rb'

#----------- 定数定義
HEADSTR = "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわん"
FROM_STR = "がぎぐげござじずぜぞだぢづでどばびぶべごぱぴぷぺぽ"
  TO_STR = "かきくけこさしすせそたちつてとはひふへほはひふへほ"



# 並べ替え済み配列を用意する
hira_array = []

File.open(ARGV[0], "r"){|readfile|
  readfile.each{|line|
    line.chomp!
    hira_array << [ line.tr("ァ-ン", "ぁ-ん").tr(FROM_STR,TO_STR), line ]
  }
}

sorted_array = hira_array.sort

# 特性のあいうえお順テーブルを作成する

File.open("tokusei_aiueo_table.cdat","w"){|file|
  file.puts "//=============================="
  file.puts "// 特性あいうえお順テーブル"
  file.puts "//=============================="

  file.puts "static const u8 tokusei_aiueo_table[] = {"

  sorted_array.each{|items|
    org_name = items[1]
    file.printf( "\t%3d, //%s\n", $tokusei_hash[org_name], org_name )
  }
  file.puts "};"
}

# 特性のかな文字ごとの開始位置テーブルを作成する

File.open("tokusei_aiueo_header_table.cdat","w"){|file|
  file.puts "//=============================="
  file.puts "// 特性あいうえお頭だし位置テーブル"
  file.puts "//=============================="

  file.puts "static const u8 tokusei_header_aiueo_table[][2] = {"
  HEADSTR.split(//).each{|code|
    start_pos = -1
    end_pos = -1
    sorted_array.each_with_index{|items, index|
      str = items[0].split(//)[0]
      if start_pos == -1 then
        if str == code then
          start_pos = index
        end
      else
        if end_pos == -1 && str != code then
          end_pos = index
        end
      end
    }
    if start_pos != -1 && end_pos == -1 then
      end_pos = sorted_array.size - 1
    end
    file.printf(" { %3d, %3d },  //%s\n", start_pos + 1, end_pos + 1, code )
  }
  file.puts "};"
}



