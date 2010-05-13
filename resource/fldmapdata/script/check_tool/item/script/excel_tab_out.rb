######################################################### 
#
# ■brief:  エクセル タブ区切りコンバータ
# □file:   excel_tab_out.rb
# □author: obata
# □date:   2010.02.12
#
######################################################### 
require "win32ole"

RETURN_CODE = "\n" # 置換対象の改行コード
SPACE_CODE  = "　" # 置換対象の空白コード
DUMMY_RETURN_CODE  = "@"   # 改行コードとして埋め込むダミーコード
DUMMY_SPACE_CODE   = "□"  # 空白コードとして埋め込むダミーコード


#--------------------------------------------------------
# ■brief:  ファイルの絶対パスを取得する
# □param:  filename ファイル名
# □return: 指定したファイルの絶対パス
#--------------------------------------------------------
def GetAbsolutePath( filename )
  fso = WIN32OLE.new('Scripting.FileSystemObject')
  return fso.GetAbsolutePathName( filename )
end


#--------------------------------------------------------
# ■brief: エクセルファイルをタブ区切りテキストに変換する
# □param: ARGV[0] コンバート対象エクセルファイル名
# □param: ARGV[1] コンバート対象のシート番号
# □param: ARGV[2] コンバート後の出力ファイル名
#--------------------------------------------------------
EXCEL_FILENAME  = GetAbsolutePath( ARGV[0] )
SHEET_NO        = ARGV[1].to_i
OUTPUT_FILENAME = ARGV[2]

excel = WIN32OLE.new( "Excel.Application" )
book  = excel.Workbooks.Open( EXCEL_FILENAME )
sheet = book.WorkSheets( SHEET_NO )
file  = File.open( OUTPUT_FILENAME, "w" )

begin
  sheet.UsedRange.Rows.each do |row|
    record = Array.new
    row.Columns.each do |cell|
      if cell.Value != nil then record << cell.Value end
    end

    # セル内の文字列に含まれる改行コードを, ダミーコードに置き換える
    record.each do |value|
      if value.is_a? String then 
        if value.include?( RETURN_CODE ) then value.gsub!( RETURN_CODE, DUMMY_RETURN_CODE ) end
      end
    end
    # セル内の文字列に含まれる空白コードを, ダミーコードに置き換える
    record.each do |value|
      if value.is_a? String then 
        if value.include?( SPACE_CODE ) then value.gsub!( SPACE_CODE, DUMMY_SPACE_CODE ) end
      end
    end

    if record.size != 0 then
      data = record.join("\t")
      file.puts( data.strip )
    end
  end
ensure
  book.Close
  excel.Quit
  file.close
end
