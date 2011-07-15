##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		achieve_data_cnv.rb
#*	@brief  達成用データコンバータ
#*	@author	Masayuki Onoue
#*	@data		2011.01.14
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要

require "csv"
require File.dirname(__FILE__) + '/rbdef.rb'
require File.dirname(__FILE__) + '/../../../tools/hash/monstype_hash.rb'
require File.dirname(__FILE__) + '/../../../tools/hash/monsno_hash.rb'
require File.dirname(__FILE__) + '/../../../tools/hash/chihou_zukanno_hash.rb'

OUT_FILE_NAME = ARGV[0]
CSV_FILE_NAME = ARGV[1]
MONSNO_FILE_NAME = ARGV[2]

# メダルデータの読み込み
data_csv = CSV.open( CSV_FILE_NAME ,"r")


# モンスターナンバーの解釈
MONS_MAX = 0
monsno_h = open( MONSNO_FILE_NAME ,"r")
monsno_h.each{|line|
  if line.include?("ZUKANNO_ISSHU_END") then
    if /\(.([0-9]+).\)/ =~ line then
      ZUKANNO_MONS_MAX = $1.to_i
    end
  end
  if line.include?("MONSNO_END") then
    if /\(.([0-9]+).\)/ =~ line then
      MONS_MAX = $1.to_i
    end
  end
}
monsno_h.close

# データの読み飛ばし
2.times{
  data_csv.shift
}

# データの読み込み
no_achieve_poke_list = Array.new
data_csv.each { |item|
  if item.include?("#END") then break end
  if $monsno_hash.key?(item[1]) then
    no_achieve_poke_list << $monsno_hash[item[1]]
  else
    raise "「%s」という名前のモンスターは未定義です"%item[1]
  end
}

# タイプデータアーカイブ作成
out_file = open( OUT_FILE_NAME ,"w" )
1.upto(MONS_MAX){|i|
  unless $chihou_zukanno_hash[i].to_i == $NO_CHIHOU_ID then
    puts $chihou_zukanno_hash[i].to_s+" "+$monsname[i].to_s
    out_file << [ i ].pack( "S" )  # 全国図鑑NO
    out_file << [ $monstype_hash[i][0] ].pack( "C" )  # タイプ１
    out_file << [ $monstype_hash[i][1] ].pack( "C" )  # タイプ２
    if no_achieve_poke_list.include?( i ) then
      out_file << [ 0 ].pack( "C" )  # チェックしない
    else
      out_file << [ 1 ].pack( "C" )  # チェックする
    end
    out_file << [ 0,0,0 ].pack( "C3" )  # パディング
  end
}
out_file.close

