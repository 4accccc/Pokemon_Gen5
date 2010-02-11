#==========================================================
#	非通信テレビトランシーバー スクリプト変換スクリプト
#===========================================================

require 'win32ole'
$KCODE = "SJIS"

module Worksheet
  def [] y,x
    self.Cells.Item(y,x).Value
  end
end

def getAbsolutePath filename
  fso = WIN32OLE.new('Scripting.FileSystemObject')
  return fso.GetAbsolutePathName(filename)
end

filename = getAbsolutePath("tvt_script.xls")

xl = WIN32OLE.new('Excel.Application')
book = xl.Workbooks.Open(filename)

begin
  book.Worksheets.each do |sheet|

    sheet.extend Worksheet
    
    if( sheet.Range('A1').Value == "DataSheet" )
      no = sheet.Range('C2').Value.to_i
      if( no != -1 )
        print("[" + no.to_s + "][" + sheet.Range('C3').Value + "]\n" )
        
        outFileName = sprintf("tvt_script_%02d.bin",no)
        
        begin
          outFile = File.open( outFileName, "wb" );
          
          #人物と背景番号
          ary = Array( sheet.Range('M3').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('N3').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('M4').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('N4').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('M5').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('N5').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('M6').Value.to_i )
          outFile.write( ary.pack("C") )
          ary = Array( sheet.Range('N6').Value.to_i )
          outFile.write( ary.pack("C") )
          
          loopFlg = TRUE
          idx = 0
          while loopFlg == TRUE
            
            comNo   = sheet[9+idx,6]
            charaNo = sheet[9+idx,3]
            subNo   = sheet[9+idx,5]

            #引数の空白回避
            if subNo == nil 
              subNo = 0
            end
            if charaNo == nil 
              charaNo = 0
            end
            
            if comNo == nil 
              print( "Error 終了コマンドがありません\n" )
              loopFlg = FALSE
            end

            ary = Array( comNo.to_i )
            outFile.write( ary.pack("C") )
            ary = Array( charaNo.to_i )
            outFile.write( ary.pack("C") )
            ary = Array( subNo.to_i )
            outFile.write( ary.pack("S") )
            
            if comNo.to_i == 0
              loopFlg = FALSE
            end
            
            idx = idx+1
          end
          
        ensure
          outFile.close
        end

      end
    end

#    tempStr = sheet.Range('A1').Value
#    if tempStr == nil
#      tempStr = "nil"
#    end
#    print( sheet.Name + "[" + tempStr + "]\n" )
  end


ensure
  xl.Workbooks.Close
  xl.Quit

end #begin
