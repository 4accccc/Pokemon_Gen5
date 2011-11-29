#------------------------------------------------------------------------------
#
# ポケウッド（仮）のトレーナータイプ指定から動作モデル指定を取り出すテーブルを作る
#
# @author  tamada GAMEFREAK inc.
# @date     2011.09.22
#------------------------------------------------------------------------------
require 'Date'

PROJECT_ROOT = ENV["PROJECT_ROOT"]
require PROJECT_ROOT + 'tools/constant'
require PROJECT_ROOT + 'tools/headerdata.rb'

headerArray = HeaderDataArray.new();
headerArray.load(PROJECT_ROOT + 'resource/trtype/trtype_def.h')
headerArray.load(PROJECT_ROOT + 'resource/fldmmdl/fldmmdl_objcode.h')

raw_data = Array.new
conv_data = Hash.new

File.open(ARGV[0],"r"){|readfile|
  read_start = false
  readfile.each{|line|
    if line =~ /\#START/ then
      read_start = true
    elsif line =~ /\#END/ then
      read_start = false
    elsif read_start == true then
      raw_data << line.chomp.split(",")
    end
  }
}

raw_data.each{|column|
  mmdl = column[2]
  trtype = column[3]
  if trtype == "未決定" then next end

  trtypeid = headerArray.search( 'TRTYPE_' + trtype, trtype )
  mmdlid = headerArray.search( mmdl, mmdl )
  if trtypeid == nil then 
    puts "trtype = #{trtype}"
    puts column
    exit -1
  end
  #mmdl = headerArray.search( column[2], column[2] )

  if conv_data.has_key?(trtype) then
    if conv_data[trtype] != mmdl then
      STDERR.puts("違う種類の変換があります!!\n")
      STDERR.printf("(A)%s --> %s\n", trtype, conv_data[trtype] )
      STDERR.printf("(B)%s --> %s\n", trtype, mmdl )
      exit -1
    end
  else
    conv_data[trtype] = mmdl
  end
}

File.open(ARGV[1],"w"){|writefile|
  writefile.puts "//#{DateTime.now.to_s}"
  writefile.puts("//ポケウッド：撮影時トレーナータイプ→動作モデル指定変換テーブル")
  writefile.puts("//このファイルはコンバータで自動生成しています")
  writefile.puts("//#{ARGV[0]} --> #{ARGV[1]}")

  writefile.puts("static const u16 TrType2MMdl[][2] = {")
  conv_data.each{|trtype, mmdl|
    writefile.printf("\t{ TRTYPE_%s, %s },\n", trtype, mmdl )
  }
  writefile.puts("};\n")
}
