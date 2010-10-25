#/[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
 #
 #	@file		sp_ng_cmd_make.rb
 #	@brief  scrcmd_list.xlsより検索ファイルを作成する
 #	@author		Masayuki Onoue
 #	@data		2010.10.20
 #
 ##
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"

#/=============================================================================
##
 #					定数宣言
##
#/=============================================================================

OUT_DIR = "../"
INIT_FILENAME     = "init_ng_cmd.txt"
RECOVER_FILENAME  = "recover_ng_cmd.txt"
RESERVE_FILENAME  = "reserve_ng_cmd.txt"
ZONE_FILENAME     = "zone_ng_cmd.txt"

# 使用可能かが指定されている列を定義
NAME_SEQUENCE     = 0
RESERVE_SEQUENCE  = 1
INIT_SEQUENCE     = 2
RECOVER_SEQUENCE  = 2
ZONE_SEQUENCE     = 3

# ×がついているものをファイルごとに書き込んでいく ##############################
file = File.open( "scrcmd_list.txt", "r" )
file_lines = file.readlines
file_lines.slice!(0) # 先頭行を削除
file.close 

init_file     = File.open( OUT_DIR + INIT_FILENAME , "w" ) 
recover_file  = File.open( OUT_DIR + RECOVER_FILENAME , "w" ) 
reserve_file  = File.open( OUT_DIR + RESERVE_FILENAME , "w" ) 
zone_file     = File.open( OUT_DIR + ZONE_FILENAME , "w" ) 

file_lines.each do |line|
  line.gsub!( "\"", "" )
  items = line.split( /\t/ ) 
  puts items[NAME_SEQUENCE]

  if items[RESERVE_SEQUENCE].include?( "×" ) then
     reserve_file.puts( items[NAME_SEQUENCE] )
  end

  if items[INIT_SEQUENCE].include?( "×" ) then
     init_file.puts( items[NAME_SEQUENCE] )
  end 

  if items[RECOVER_SEQUENCE].include?( "×" ) then
     recover_file.puts( items[NAME_SEQUENCE] )
  end

  if items[ZONE_SEQUENCE].include?( "×" ) then
     zone_file.puts( items[NAME_SEQUENCE] )
  end  
end

