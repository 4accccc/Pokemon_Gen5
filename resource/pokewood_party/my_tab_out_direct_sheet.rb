#!ruby -Ks
#---------------------------
# my_tab_out_direct_sheet.rb
#
# tab_out_direct_sheet.rb
# を参考にして持ってきた
# エラーチェック等は後回し
#---------------------------

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

# エクセルのアクセス
class MY_ExcelConv
  # 初期化
  def initialize()
    $xl = WIN32OLE.new('Excel.Application')
  end
  
  # 開く
  def open_file( filename )
    $xl.Workbooks.Open(filename)
  end
  
  # 閉じる
  def close_file()
    $xl.Workbooks.Close
#    $xl.Quit
  end
  
  # 終了
  def exit()
    $xl.Quit
  end
  
  # 情報の取得
  # (col,row)(縦、横)の情報を取得
  def get_cell( sheetnum, col, row )
    # CP_ACPで取得(通常のwindows)
    WIN32OLE.codepage = WIN32OLE::CP_ACP
    #
    sheet = $xl.Worksheets.Item( sheetnum+1 )
    sheet.extend Worksheet
    str = sheet.Cells( col, row ).Value
    return( str )
  end
  
  # 情報の取得
  # (col,row)(縦、横)の情報を取得
  def get_cell2( sheetnum, col, row )
    # UTF8で取得
    WIN32OLE.codepage = WIN32OLE::CP_UTF8
    #
    sheet = $xl.Worksheets.Item( sheetnum+1 )
    sheet.extend Worksheet
    str = sheet.Cells( col, row ).Value
    return( str )
  end
  #
  # 縦方向サイズ
  def get_row_size( sheetnum )
    sheet = $xl.Worksheets.Item( sheetnum+1 )
    return( sheet.usedRange.rows.count )
  end
  #
  # 横方向サイズ
  def get_col_size( sheetnum )
    sheet = $xl.Worksheets.Item( sheetnum+1 )
    return( sheet.usedRange.columns.count )
  end
end

