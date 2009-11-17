#==========================================================
#	ミュージカル・演目データ変換Ruby
#	
#===========================================================
require 'fileutils'
require 'win32ole'

$KCODE = "SJIS"

#============================
#	define
#============================
STR_END = "#END"
HASH_ERROR = 255

#エクセルのデータ開始行
EXCEL_DATA_COL = 4

#データインデックス
DATA_IDX_MANAGE_NO = 1
DATA_IDX_BACK_NO = 2
DATA_IDX_CON_COOL = 3
DATA_IDX_CON_CUTE = 4
DATA_IDX_CON_ELEGANT = 5
DATA_IDX_CON_UNIQUE = 6
DATA_IDX_CON_RANDOM = 7
DATA_IDX_AWARD_TYPE = 8
DATA_IDX_AWARD_NO = 9
DATA_IDX_POKE_DATA_START = 10

POKEDATA_IDX_MONSNO = 0
POKEDATA_IDX_TRAINER = 1
POKEDATA_IDX_TR_NAME = 2
POKEDATA_IDX_GOODS_BASE = 3

GOODSDATA_IDX_NO = 0
GOODSDATA_IDX_POS = 1

#上の個数変えたらココも対応すること
GOODS_NUM = 8
GOODSDATA_NUM = 2
POKE_NUM = 3
POKEDATA_NUM = 3 + GOODSDATA_NUM*GOODS_NUM

#============================
#	proto
#============================

#============================
#	main
#============================
module Worksheet
  def [] y,x
    self.Cells.Item(y,x).Value
  end
end

def getAbsolutePath filename
  fso = WIN32OLE.new('Scripting.FileSystemObject')
  return fso.GetAbsolutePathName(filename)
end

module ProgramExcelConv

def self.conv( no , dataFileName , outFileName )

xl = WIN32OLE.new('Excel.Application')
begin

  filename = getAbsolutePath(dataFileName)
  xl.Workbooks.Open(filename)

  outFile = File.open( outFileName, "wb" );

  sheet = xl.Worksheets.Item("output")
  sheet.extend Worksheet
  
  dataCol = EXCEL_DATA_COL + no - 1
  
  padAry = Array(170) #(0xAA)

  #背景番号
  ary = Array( sheet[dataCol,DATA_IDX_BACK_NO].to_i )
  outFile.write( ary.pack("C") )

  #コンディション
  ary = Array( sheet[dataCol,DATA_IDX_CON_COOL].to_i )
  outFile.write( ary.pack("C") )
  ary = Array( sheet[dataCol,DATA_IDX_CON_CUTE].to_i )
  outFile.write( ary.pack("C") )
  ary = Array( sheet[dataCol,DATA_IDX_CON_ELEGANT].to_i )
  outFile.write( ary.pack("C") )
  ary = Array( sheet[dataCol,DATA_IDX_CON_UNIQUE].to_i )
  outFile.write( ary.pack("C") )
  ary = Array( sheet[dataCol,DATA_IDX_CON_RANDOM].to_i )
  outFile.write( ary.pack("C") )

  #ご褒美
  ary = Array( sheet[dataCol,DATA_IDX_AWARD_TYPE].to_i )
  outFile.write( ary.pack("C") )
  outFile.write( padAry.pack("C") ) #パディング
  ary = Array( sheet[dataCol,DATA_IDX_AWARD_NO].to_i )
  outFile.write( ary.pack("S") )

  #ぽけデータ
  pokeIdx = 0
  while pokeIdx < POKE_NUM

    rowBase = DATA_IDX_POKE_DATA_START + (POKEDATA_NUM*pokeIdx)
    ary = Array( sheet[dataCol,rowBase+POKEDATA_IDX_MONSNO].to_i )
    outFile.write( ary.pack("S") )
    ary = Array( sheet[dataCol,rowBase+POKEDATA_IDX_TRAINER].to_i )
    outFile.write( ary.pack("C") )
    ary = Array( sheet[dataCol,rowBase+POKEDATA_IDX_TR_NAME].to_i )
    outFile.write( ary.pack("C") )
    
    goodsIdx = 0
    while goodsIdx < GOODS_NUM
      goodsRowBase = DATA_IDX_POKE_DATA_START + (POKEDATA_NUM*pokeIdx) + POKEDATA_IDX_GOODS_BASE + (GOODSDATA_NUM*goodsIdx)
      ary = Array( sheet[dataCol,goodsRowBase+GOODSDATA_IDX_NO].to_i )
      outFile.write( ary.pack("S") )
      ary = Array( sheet[dataCol,goodsRowBase+GOODSDATA_IDX_POS].to_i )
      outFile.write( ary.pack("C") )
      outFile.write( padAry.pack("C") ) #パディング
      
      goodsIdx = goodsIdx + 1
    end

    pokeIdx = pokeIdx + 1
  end

ensure
  outFile.close
  xl.Workbooks.Close
  xl.Quit

end #begin

end #self.conv

=begin
dataFileName = ARGV[0]
outFileNameBase = ARGV[1]
if( dataFileName == nil )
	printf( "data file is not found!!\n" )
	exit 1
end
if( outFileNameBase == nil )
	printf( "output file is not found!!\n" )
	exit 1
end

printf("datafile   [%s]\n",dataFileName);
printf("outputfile [%s]\n",outFileNameBase);

dataFile = File.open( dataFileName, "r" );

#ファイルの解釈開始

line = dataFile.gets #１行目は注意書きなので抜かす
line = dataFile.gets #２行目は大ラベルなので抜かす
line = dataFile.gets #３行目は小ラベルなので抜かす
#hash生成
TEX_HASH_DATA.default = HASH_ERROR

no = 1
#パラメタコンバート
while line = dataFile.gets
	if( line.include?(STR_END) )
		break
	end
  
  pathName = sprintf("p%02d",no)
  outFileName = sprintf("%s/mus_prugram%02d.bin",pathName,no)

  FileUtils.mkdir_p(pathName) unless FileTest.exist?(pathName)

  outFile = File.open( outFileName, "wb" );
  printf("outputfile [%s]\n",outFileName);

  str = line.split( "\t" )

  no = no+1
  outFile.close
end

#ファイルの解釈終了

dataFile.close
=end

end #ProgramExcelConv