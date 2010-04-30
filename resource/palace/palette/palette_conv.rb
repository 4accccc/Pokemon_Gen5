#==========================================================
#	ミュージカル・演目データ変換Ruby
#	
#===========================================================
require 'fileutils'
require 'win32ole'
require 'kconv'
require 'nkf'

$KCODE = "UTF8"

#============================
#	define
#============================

SHEET_NUM = 4
TABLE_NUM = 32

HASH_ERROR = 255

DATA_OFS_Y = 3
LABEL_OFS_X = 1
AREA_OFS_X = 2
SEASON_OFS_X = 3
TABLE_OFS_X = 4
#============================
#	proto
#============================
SHEET_NAME = ["White_normal",
              "White_fog",
              "Black_normal",
              "Black_fog"]
FILE_NAME =  ["shade_white_normal.bin",
              "shade_white_fog.bin",
              "shade_black_normal.bin",
              "shade_black_fog.bin"]


SEASON_HASH_DATA = {
  "全部"=>255 ,
  "春"=>0 ,
  "夏"=>1 ,
  "秋"=>2 ,
  "冬"=>3 ,
}
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

module ProfanaryExcelConv

dataFileName = ARGV[0]
#hash生成
SEASON_HASH_DATA.default = HASH_ERROR

xl = WIN32OLE.new('Excel.Application')
begin
  
  filename = getAbsolutePath(dataFileName)
  xl.Workbooks.Open(filename)

  sheetNum = 0
  while sheetNum < SHEET_NUM
    print( SHEET_NAME[sheetNum] + "->" + FILE_NAME[sheetNum] + "\n" )
    outFile = File.open( FILE_NAME[sheetNum] , "wb" );

    sheet = xl.Worksheets.Item(SHEET_NAME[sheetNum])
    sheet.extend Worksheet
    
    posy = 0
    isLoop = 1
    while isLoop == 1
      
      if( sheet[posy+DATA_OFS_Y,LABEL_OFS_X] != nil )
        #エリア
        areaVal = 0
        if( sheet[posy+DATA_OFS_Y,AREA_OFS_X].to_s == "全部" )
          areaVal = 255
        else
          str = sheet[posy+DATA_OFS_Y,AREA_OFS_X].to_s
          areaVal = str[3..4].to_i
        end
        ary = Array( areaVal )
        outFile.write( ary.pack("C") )
        #print( "["+sheet[posy+DATA_OFS_Y,AREA_OFS_X].to_s + ":" + areaVal.to_s + "]")
        
        #季節
        seasonNo = SEASON_HASH_DATA[sheet[posy+DATA_OFS_Y,SEASON_OFS_X].to_s]
        #print( "["+sheet[posy+DATA_OFS_Y,SEASON_OFS_X].to_s + ":" + seasonNo.to_s + "]")
        ary = Array( seasonNo )
        outFile.write( ary.pack("C") )
        
        #Padding
        ary = Array( 0 )
        outFile.write( ary.pack("S") )

        posx = 0
        while posx < TABLE_NUM
          ary = Array( sheet[posy+DATA_OFS_Y,posx+TABLE_OFS_X].to_i )
          outFile.write( ary.pack("C") )
          
          #print( sheet[posy+DATA_OFS_Y,posx+TABLE_OFS_X].to_s + "," )
          posx = posx+1
        end #while posx < 32

      else  #if( sheet[posy+DATA_OFS_Y,posx+1] != nil )
        isLoop = 0
      end

      #print( "\n" )
      posy = posy+1
    end #while posy < DATA_NUM
    
    sheetNum = sheetNum+1
    outFile.close
  end #while sheetNum < SHEET_NUM
  
ensure
  #outFile.close
  xl.Workbooks.Close
  xl.Quit

end #begin

end