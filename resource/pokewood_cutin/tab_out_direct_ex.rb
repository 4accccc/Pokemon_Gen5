=begin
 * @file    tab_out_direct.rb
 * @brief   エクセルからタブ区切りファイルを出力するエクセル
 * @author  ariizumi
 * @data    09/09/22

 * @brief   シートを別ファイルで保存したかったので色々改造
 * @author  toru nagihashi
 * @data    11/05/25

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
if ARGV[1] == "-t"
end

xl = WIN32OLE.new('Excel.Application')

begin
  xl.Workbooks.Open(filename)
  sheetNum = xl.Worksheets.Count
  print "シートは#{sheetNum}枚\n"

  sheetcnt = 0
  1.upto(sheetNum) do |sheetNo|
  

    newfile = File.new( ARGV[2]+"#{sheetcnt}.csv", "w" )

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
          newfile.print useRange.Cells(rowNo,colNo).Value
          newfile.print sep
        else
          if isOutSpace == TRUE
          newfile.print sep
          end
        end
      end
      newfile.print "\n"
    end
    
#    print "行#{useRange.Columns.Count}列\n"
#    print "行#{useRange.Rows.Count}行\n"
    
    newfile.close
    sheetcnt+=1
    
  end
ensure
  xl.Workbooks.Close
  xl.Quit
end
