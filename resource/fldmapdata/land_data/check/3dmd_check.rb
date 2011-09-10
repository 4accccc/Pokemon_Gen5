#!/usr/bin/ruby


#=================================================
# @file   3dmd_check.rb
#	@brief  3dmdファイルの差分チェック
#	@author	Katsuhiko=Ichiraku
#	@date		2011.09.07
#
#
# swan、swan_data3d、wb、wb_data3dの3dmdファイルの
# 差分一覧を作る
#
# 差分があると○
# 差分なしなら×
# ファイルがないなら「なし」
# 
# SWANを基準にチェックするときのみ
# WBとWB_DATA3Dに違いがあると◎
#
# 1つでも差分があるときのみ出力する
#
# *** 使い方 ***
#  グローバル変数に各プロジェクトへのパスを記述するだけ。
#
#  検索元を変更したいなら
#  $switchを変更する
#
#  生成されたcsvはxlsで保存し直すほうが使いやすい
#
#=================================================

require 'fileutils'   # FileUtils
require "csv"

#グローバル変数
# 3dmdの格納場所へのﾊﾟｽ
$SWAN       = "c:/home/swan/resource/fldmapdata/land_data/land_res/"
$SWAN_DATA3D= "c:/home/swan_data3d/field/"
$WB         = "e:/home/master_2nd/resource/fldmapdata/land_data/land_res"
$WB_DATA3D  = "c:/home/wb_data3d/field/"

# 検索元をSWANかWB
SWAN = 0
WB   = 1
$switch = SWAN

# 戻り値用
FILE_SAME = -1    # 見つかったが内容は同じだった
$ret = ""

# 3dmdファイル一覧を作る
def GetFileList( dir, dataArray )

  dir = dir.sub(/\/$/, '' ) # 最後に / があればとる

  # ディレクトリ内を検索
  Dir::foreach( dir ){ |file|
    if file == '..'
      next
    end
    if file == '.'
      next
    end
    if file == '.svn'
      next
    end

    # 3dmdファイル
    if file =~ /\.3dmd/
      dataArray.push( dir + '/' + file )
    end

    # サブディレクトリも検索
    if true == FileTest::directory?(dir + '/' + file)
      GetFileList( dir + '/' + file , dataArray)
    end
  }

end

# 配列の中にnameがあるか調べる
# 調べた後、内容が違うかチェック
#  ret != "" だと内容が違う
def SearchName( tmpArray, name )

  tmpArray.each do |tmp|
    basename1 = File.basename( name )
    basename2 = File.basename( tmp )
    # ファイルが見つかった
    if basename1 == basename2

      if true == FileTest.exist?(tmp) #コピー先が存在する
        if false == FileUtils.compare_file( tmp, name ) # 内容が違う
          $ret = tmp
        else
          $ret = FILE_SAME
        end
      end

      break
    end
  end
end

#-----------------------------------------
# メイン

# 検索の元
dataArrayBase = Array.new
# 検索される配列
dataArrayTmp1 = Array.new
dataArrayTmp2 = Array.new
dataArrayTmp3 = Array.new

# 検索元を変更する
if $switch == SWAN
  GetFileList( $SWAN, dataArrayBase )
  GetFileList( $WB, dataArrayTmp2 )
else
  GetFileList( $SWAN, dataArrayTmp2 )
  GetFileList( $WB, dataArrayBase )
end

#GetFileList( $SWAN, dataArrayBase )
GetFileList( $SWAN_DATA3D, dataArrayTmp1 )
#GetFileList( $WB, dataArrayTmp2 )
GetFileList( $WB_DATA3D, dataArrayTmp3 )


# csvに書き込み用
writer = CSV.open("3dmd_check.csv", 'w')

writer << ["ファイル名", "SWAN","SWAN_DATA3D", "WB", "WB_DATA3D","パス","補足"]

dataArrayBase.each do |sfile|

  row = ["","","","","","",""]
  is_insert = false
  wb_check = 0

  # 初期化
  row[0] = File.basename( sfile )
  #row[1] = "基準"
  row[2] = "なし"
  #row[3] = "-"
  row[4] = "なし"
  row[5] = ""

  if $switch == SWAN
    row[1] = "基準" # swan
    row[3] = "なし"    # wb
  else
    row[1] = "なし"    # swan
    row[3] = "基準" # wb
  end

  $ret = ''
  SearchName( dataArrayTmp1, sfile )
  if $ret != ''
    if $ret != FILE_SAME
      row[2] = "○"

      # バスのフォルダ名のみ出力
      tmp = File.dirname($ret)
      tmp = tmp.split("/")
      row[5] = "・" + tmp.pop

      is_insert = true
    else
      row[2] = "-"
    end
  end

  $ret = ''
  SearchName( dataArrayTmp2, sfile )
  if $ret != ''
    if $ret != FILE_SAME

      if $switch == SWAN
        row[3] = "○"   # swan
        wb_check += 1
      else
        row[1] = "○"   # wb
      end

      is_insert = true
    else
      if $switch == SWAN
        row[3] = "-"
      else
        row[1] = "-"
      end
    end
  end

  $ret = ''
  SearchName( dataArrayTmp3, sfile )
  if $ret != ''
    if $ret != FILE_SAME
      row[4] = "○"
      
      if $switch == SWAN
        wb_check += 1
      end

      # バスのフォルダ名のみ出力
      tmp = File.dirname($ret)
      tmp = tmp.split("/")
      row[5] += "・" + tmp.pop

      is_insert = true
    else
      row[4] = "-"
    end
  end

  # WBとWB_DATA3Dでの違いがあるかチェック
  if wb_check == 2
    SearchName( dataArrayTmp2, sfile )
    tmp1 = $ret
    SearchName( dataArrayTmp3, sfile )
    tmp2 = $ret

    if false == FileUtils.compare_file( tmp1, tmp2 ) # 内容が違う
      if false == FileUtils.compare_file( tmp1, sfile ) # 内容が違う
        row[3] = "◎"
      end
      if false == FileUtils.compare_file( tmp2, sfile ) # 内容が違う
        row[4] = "◎"
      end
    end
  end

  # 補足説明
  if row[2] == "○" && ( row[3] == "なし" || row[3] == "-" ) && ( row[4] == "なし" || row[4] == "-" ) then
    # ○ - -
    row[6] = "SWANかSWAN_DATA3Dにコミット忘れがある"
  elsif ( row[2] == "なし" || row[2] == "-" ) && row[3] == "○" && ( row[4] == "なし" || row[4] == "-" ) then
    # - ○ -
    row[6] = "WBかWB_DATA3Dにコミット忘れがある。SWANに反映する必要がある。"
  elsif ( row[2] == "なし" || row[2] == "-" ) && ( row[3] == "なし" || row[3] == "-" ) && row[4] == "○" then
    # - - ○
    row[6] = "WBかWB_DATA3Dにコミット忘れがある。SWANに反映する必要がある。"
  elsif row[2] == "○" && row[3] == "○" && ( row[4] == "なし" || row[4] == "-" ) then
    # ○ ○ -
    row[6] = "このパターンはない"
  elsif ( row[2] == "なし" || row[2] == "-" ) && row[3] == "○" && row[4] == "○" then
    # - ○ ○
    row[6] = "SWANで変更があるだけ。問題なし"
  elsif row[2] == "○" && ( row[3] == "なし" || row[3] == "-" ) && row[4] == "○" then
    # ○ - ○
    row[6] = "SWANかSWAN_DATA3Dにコミット忘れがある。WBかWB_DATA3Dにコミット忘れがある"
  elsif row[2] == "○" && row[3] == "○" && row[4] == "○" then
    # ○ ○ ○
    row[6] = "SWAN_DATA3Dにコミット忘れがある"
  elsif ( row[2] == "なし" || row[2] == "-" ) && row[3] == "◎" && row[4] == "◎" then
    # - ◎ ◎
    row[6] = "WBかWB_DATA3Dにコミット忘れがある。SWANに反映する必要がある。"
  elsif row[2] == "○" && row[3] == "◎" && row[4] == "◎" then
    # ○ ◎ ◎
    row[6] = "WWBかWB_DATA3Dにコミット忘れがある。SWANかSWAN_DATA3Dにコミット忘れがある。SWANに反映する必要がある。"
  end

  # 1つでもデータがあると代入
  if is_insert == true
    writer << row
  end

end

writer.close

