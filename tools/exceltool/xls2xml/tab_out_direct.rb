=begin
 * @file    tab_out_direct.rb
 * @brief   エクセルからタブ区切りファイルを出力するエクセル
 * @author  ariizumi
 * @data    09/09/22

  tab_out.rbでは計算式がセルに入力されていた場合、その式を出力してしまったため作成。
  Ruby1.8から標準でついているwin32oleと言うライブラリを使って、エクセルに直接アクセスし、
　ファイルの出力を行っている。

　使っている範囲だけで出力しているので、A列や1行目が空行だと、B行や2列目から出力されるので注意。
　
　tab_out_direct.rb <ファイル名> : TAB区切り
　tab_out_direct.rb <ファイル名> -c : カンマ区切り
　tab_out_direct.rb <ファイル名> -s : カンマ区切り＋空白を出す
=end


require 'win32ole'


module Worksheet
  def [] y,x
    self.Cells.Item(y,x).Value
  end
end

def getAbsolutePath filename
  fso = WIN32OLE.new('Scripting.FileSystemObject')
  return fso.GetAbsolutePathName(filename)
end


filename = getAbsolutePath(ARGV[0])
sep = "\t"
isOutSpace = FALSE
if ARGV[1] == "-c"
  sep = ","
end
if ARGV[1] == "-s"
  sep = ","
  isOutSpace = TRUE
end


xl = WIN32OLE.new('Excel.Application')

begin
  xl.Workbooks.Open(filename)
  sheetNum = xl.Worksheets.Count
#  print "シートは#{sheetNum}枚\n"

  1.upto(sheetNum) do |sheetNo|
  
    sheet = xl.Worksheets.Item(sheetNo)
    sheet.extend Worksheet
    useRange = sheet.UsedRange
    colNum = useRange.Columns.Count
    rowNum = useRange.Rows.Count
#    print "#{colNum}列：#{rowNum}行\n"
#    print "#{useRange.Column}列：#{useRange.Row}行\n"
    
    1.upto(rowNum) do |rowNo|
      1.upto(colNum) do |colNo|
        if useRange.Cells(rowNo,colNo).Value != nil
          print useRange.Cells(rowNo,colNo).Value
          print sep
        else
          if isOutSpace == TRUE
          print sep
          end
        end
      end
      print "\n"
    end
    
#    print "行#{useRange.Columns.Count}列\n"
#    print "行#{useRange.Rows.Count}行\n"
    
    
  end
ensure
  xl.Workbooks.Close
  xl.Quit
end
