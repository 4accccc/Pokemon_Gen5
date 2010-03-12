#!/usr/bin/ruby
#===================================================================
#
# @brief  make対象にするvpathを吐き出す
#
# @data   10.03.08
# @author miyuki iwasawa 
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定
$KCODE = 'SJIS'

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "make_vpath"  #出力ファイル名

#====================================================
# 主処理 開始
#====================================================

#カレントにあるファイルをリストアップ
pwd =  Dir::entries( Dir::pwd )

# 配列化
val = pwd.to_a;
# フォルダに厳選、特殊フォルダを排除
val = val.select{ |i| File::ftype(i) == "directory" && i != "." && i != ".." && i != ".svn" }

# 文字列生成
str = "RES_DIR = "
val.each{ |i|
  str += i
  str += " "
}
str += "\n\n"

File::open( DST_FILENAME ,"w"){ |file|
  file.print str
  file.print "vpath %imd $(RES_DIR)\n"
  file.print "vpath %itp $(RES_DIR)\n"
  file.print "vpath %ita $(RES_DIR)\n"
  file.print "vpath %ica $(RES_DIR)\n"
  file.print "vpath %ima $(RES_DIR)\n"
  file.print "vpath %iva $(RES_DIR)\n\n"
}

