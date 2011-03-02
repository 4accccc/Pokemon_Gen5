=begin
 * @file    tab_out_direct_sheet.rb
 * @brief   エクセルからタブ区切りファイルを出力するエクセル(シート指定オプション追加版）
 * @author  ariizumi / hosaka genya
 * @data    09/09/22

  tab_out.rbでは計算式がセルに入力されていた場合、その式を出力してしまったため作成。
  Ruby1.8から標準でついているwin32oleと言うライブラリを使って、エクセルに直接アクセスし、
　ファイルの出力を行っている。

　使っている範囲だけで出力しているので、A列や1行目が空行だと、B行や2列目から出力されるので注意。
　
　tab_out_direct_sheet.rb <ファイル名> オプション

  オプション (デフォルトはTAB区切り）
  -c
    カンマ区切り
  -s
    カンマ区切り＋空白を出す

  -n x
    x番号のシートを出力(0から)
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

def PrintSheet( xl, sheetNo, sep, isOutSpace )
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

sep = "\t"
isOutSpace = FALSE
sheetno = -1
noget = false 
filename = ""

#オプションの解釈処理
ARGV.each{ |command|
  if noget
    sheetno = command.to_i
    noget = false
  else
    case command
    when "-c"	then
      sep = ','
    when "-s"	then 
      sep = ','
      isOutSpace = TRUE
    when "-n" then
      noget = true
    else
      filename = getAbsolutePath(command)
      ##when /\-.+/		then raise CommnandLineArgumentError, "無効なオプションです"
    end
  end
}


xl = WIN32OLE.new('Excel.Application')

begin
  xl.Workbooks.Open(filename)

  if sheetno == -1 then
    sheetNum = xl.Worksheets.Count
#   print "シートは#{sheetNum}枚\n"
    1.upto(sheetNum) do |sheetNo|
      PrintSheet( xl, sheetNo, sep, isOutSpace )
    end
  else
    PrintSheet( xl, sheetno+1, sep, isOutSpace )
  end
  
ensure
  xl.Workbooks.Close
  xl.Quit
end
