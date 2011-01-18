##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		gratype_data_maker.rb
#*	@brief  graphicタイプを連番でデータに変換する
#*	@author	Masayuki Onoue
#*	@data		2011.01.14
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"
require File.dirname(__FILE__) + '/rbdef.rb'

OUT_FILE = ARGV[0]
CSV_FILE = ARGV[1]

out_file = open( OUT_FILE ,"w" )

# メダルデータの読み込み
data_csv = CSV.open( CSV_FILE ,"r")

# データの読み飛ばし
1.times{
  data_csv.shift
}

out_file.puts GRATYPE_CMT % OUT_FILE

count = 0
# データの読み込み
data_csv.each { |item|
  
  if item[2].include?("#END") then
    break
  end

  out_file.puts '   "' + item[2] + '" => ' + count.to_s + ","
  count+=1
}
out_file.puts "}"

out_file.close
