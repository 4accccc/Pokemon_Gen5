##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		monsno_check.rb
##	@brief  モンスターナンバーチェック
##	@author	Masayuki Onoue
##	@data		2011.06.01
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require File.dirname(__FILE__) + '/../../personal/monsno_hash.rb'
require "csv"

NAME_INDEX    = 1
MONSNO_INDEX  = 3

data = open(ARGV[0],"r")

data.each_with_index{|line,index|
  if index == 0 || line.include?("#END") then
     next
  end
  
  data_list = line.split("\t")
  name = data_list[NAME_INDEX]
  if name =~ /(.*)[・,／]/ then
    name = $1
  end

  if $monsno_hash.include?( name ) == FALSE then
    raise "モンスター名が不正です->%s"%name
  end

  # 記述ポケモンナンバー取得
  data_list[MONSNO_INDEX] =~ /ポケモンNo\.(.*)/
  if $monsno_hash[name] != $1.to_i then
    raise "モンスターNoが不正です->%s"%name
  end

}
data.close

