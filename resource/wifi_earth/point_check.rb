#!/usr/bin/ruby
#===================================================================
#
# @brief  wifi_earch 頂点チェックツール
#
# @data   10.05.28
# @author genya_hosaka
# 
# @note   同じ座標が存在するか調べるツール。（ただし[0,0]はチェックしない）
#
#===================================================================

# 文字コード指定
$KCODE = 'SJIS'

FILE_TBL = [
"ARG.txt",
"AUS.txt",
"BRA.txt",
"CAN.txt",
"CHN.txt",
"DEU.txt",
"ESP.txt",
"FIN.txt",
"FRA.txt",
"GBR.txt",
"IND.txt",
"ITA.txt",
"JPN.txt",
"NOR.txt",
"POL.txt",
"RUS.txt",
"SWE.txt",
"USA.txt",
]

FILE_WORLD = "world.txt"

#構造体定義
SPos = Struct.new('Pos', :file, :name, :px, :py)

#配列定義
pos = Array.new

#一行ずつ必要な情報を収集
def push_line f, pos, filename, e1, e2, e3
    cnt = 0
    f.each{ |line|
      if cnt >= 3 
        s = line.split(",")
        if ( ( s[e2] == "0" && s[e3] == "0" ) == false ) && s[e1] != "" && s[e2] != "" && s[e3] != ""
          pos.push( SPos.new( filename, s[e1], s[e2], s[e3] ) )
        end      
      end 
      cnt += 1
    }
end

puts "一致チェック開始..."

#集計
FILE_TBL.each{ |filename|
  File.open( filename, "r"){|f|
    push_line f, pos, filename, 1, 2, 3
  }
} 
  
#world
File.open( FILE_WORLD, "r" ){|f|
  push_line f, pos, FILE_WORLD, 1, 4, 5
}

puts pos

#配列コピー
pos2 = pos.dup

#重複チェック
pos.each{ |elem|
#  puts "! >>"
#  puts elem
  
  #対象から自分自身を消す
  pos2.shift
  
  pos2.each{ |elem2|
#   p elem2
    if elem.px == elem2.px && elem.py == elem2.py
      #重複したテーブルを出力
      puts  "一致 >> " + 
            "<" + elem.file + ", " + elem.name + " > == " +
            "<" + elem2.file + ", "+ elem2.name + " > " +
            "[" + elem.px + ", " + elem.py + "]" 
  #   p elem
  #   p elem2
    end
  }
}

puts "一致チェック終了！"

