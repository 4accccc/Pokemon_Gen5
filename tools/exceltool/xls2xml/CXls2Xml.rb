# CXls2Xml.rb
# Excel(Open Office)で編集された.xlsファイルを、xml形式に変換するためのクラス
# .xlsからXMLへの変換は、poiライブラリを使用したjavaで行っている
# UTF-8からShiftJISへの変換は、NKFを使用している
# 

COLUMM_A = 0
COLUMM_B = 1
COLUMM_C = 2
COLUMM_D = 3
COLUMM_E = 4
COLUMM_F = 5
COLUMM_G = 6
COLUMM_H = 7
COLUMM_I = 8
COLUMM_J = 9
COLUMM_K = 10
COLUMM_L = 11
COLUMM_M = 12
COLUMM_N = 13
COLUMM_O = 14
COLUMM_P = 15
COLUMM_Q = 16
COLUMM_R = 17
COLUMM_S = 18
COLUMM_T = 19
COLUMM_U = 20
COLUMM_V = 21
COLUMM_W = 22
COLUMM_X = 23
COLUMM_Y = 24
COLUMM_Z = 25





### 定数定義
#
CMD_PATH = File.dirname($0)


# .xls を .xml に変換するためのコマンド式
#CMD_XLS2XML = "java -Xmx512M -jar #{CMD_PATH}/xls2xml.jar -s -windows "
#CMD_XLS2XML = "java -Xmx512M -jar #{CMD_PATH}/poidom.jar -s -windows "
CMD_XLS2XML = "java -classpath xls2xml.jar XLS2XML "
CMD_2SJIS = ""
#CMD_2SJIS = " | #{CMD_PATH}/nkf.exe -SsLw"

# 出力を抑制するオプション文字列
OPTION_NO_OUT = "NO_OUT"



### 文字列定義
# 入力xlsファイルが見つからないエラー
STR_FILE_NOT_FOUND = "File not Found! (ファイルが見つかりません) :"
# xlHtmlでのエラー
STR_BROKEN_XLS_FILE = "Broken .xls File! (xlsファイルが壊れています)"
# 正体不明エラー
STR_UNKNOWN_ERROR = "Unknown Error! (おそらくコンバータにバグがあります！)"


### 例外クラス
# 引数エラー
class ArgErrorException < Exception; end
# xlsファイルが見つからないエラー
class FileNotFoundException < Exception; end
# xlsファイルが壊れていた時のエラー
class BrokenFileException < Exception; end
# セルに予定外の値が入っていた時のエラー
class IllgalCellException < Exception; end
# 解析エラー
class AnalyzeException <  Exception; end

### .xls → XML変換クラス
class CXls2Xml
  ### 初期設定
  ### @param  なし
  ### @return 不定
  def initialize
    @numError = 0
  end

  ### .xlsをXMLに変換(バッファに出力)
  ### @param  .xlsファイル名
  ### @return XMLテキスト配列
  def xls2xml(xlsFileName)
    begin
      raise FileNotFoundException, xlsFileName if ! FileTest::exist? xlsFileName
      out = open("|" + CMD_XLS2XML + xlsFileName)
      lines = out.readlines
      out.close
      return lines
    rescue
      raise BrokenFileException
    end
  end

  ### xml文字列データからシートデータに変換
  ### @return sheets[シート番号][row][col]でアクセスできる配列
  def xml2sheet(xml)
    sheets = []
    iSheet = 0
    mode = 0
    lastRow = 0
    xml.each do |line|
      case mode
      when 0
	### シートを探す
	if line.include?("<sheet>")
	  sheets[iSheet] = []
	  mode = 1
	  lastRow = 0
	end
      when 1
	### row,colデータを得る
	if line.include?("</sheet>")
	  mode = 0
          iSheet += 1
          p iSheet
	end
	row, col = getRowCol(line)
	if (row != -1) and (col != -1)
	  while (lastRow <= row)
	    if (sheets[iSheet][lastRow] == nil)
	      sheets[iSheet][lastRow] = []
	      lastRow += 1
	    end
	  end
	  sheets[iSheet][row][col] = getValue(line)
	  next
	end
      else
	raise "コンバータにバグが潜んでいます！"
      end
    end
    return sheets
  end
  
  
  def xml2pagetitle(xml)
    tarray = []
    xml.each do |line|
      if line.include?("<pagetitle>")
        tarray.push(getValue(line))
      end
    end
    return tarray
  end
  
  
  ### 指定系統のデータを抽出する
  ### @param  抽出するデータの系統を表す文字列
  ### @param  抽出するデータの系統を表す文字列が書き込まれている.xlsの列数(COL)
  ### @param  XMLテキスト配列
  ### @param  系統別のデータを抽出するサブルーチン
  ### @return 抽出したデータ

  def getTypeData(type, typecol, xml, subFunc)
    begin
      sheets = xml2sheet(xml)
      #p sheets
      outArray = []        # 最終的に返す配列(配列の配列になる)
      objArray = []        # 1アイテムの情報を格納する配列
      mode = 0
      typeRow = -1
      subFunc += "(type, value, iRow, iCol, typeRow, objArray)"

      typeSheet = nil
      sheets.each do |sheet|
	iRow = 0
	sheet.each do |row|
	  #p row[typecol]
	  if row[typecol] == type
	    # 系統発見
	    #puts "====系統発見！===="
	    typeRow = iRow
	    typeSheet = sheet
	    break
	  end
	  iRow += 1
	end
	if typeSheet != nil
	  break
	end
      end

      if typeSheet == nil
	# エラー
	printError("Type not Found! (指定タイプがありません) #{type}")
	@numError += 1
      end

      if typeSheet != nil
	#puts "typeRow:#{typeRow}"
	#puts "typeSheet.size:#{typeSheet.size}"
	for iRow in typeRow ... typeSheet.size
	  row = typeSheet[iRow]
	  #puts "==row=="
	  #p row
	  if row.size == 0
	    break
	  end
	  iCol = 0
	  row.each do |cell|
	    value  = row[iCol]
	    break if eval subFunc
	    iCol += 1
	  end
	  outArray.push(objArray.clone)
	end
      end
    end
    #puts "outArray"
    #p outArray
    return outArray
  end

				     


  ### 改行文字を変換する
  ### @param  行文字列
  ### @return 不定
  def convLineFeed(line)
    if !(line.gsub!(/\r\n/, "\r\n"))
      if !(line.gsub!(/\n/, "\r\n"))
	if line.gsub!(/\r/, "\r\n")
	end
      end
    end
  end

  ### XML文字列の位置(Row, Col)を返す
  ### @param  調べる文字列
  ### @return RowとColの2要素の配列(Row, Colが無い場合は-1)
  def getRowCol(str)
    row = -1
    col = -1
    if str =~ /row\s*=\s*\"(\d+)\"\s+col\s*=\s*\"(\d+)\"/i	
      row = $1.to_i + 1
      col = $2.to_i
    end
    rowcol = [row, col]
  end

  ### 数値以外の文字が入っていないか調べる
  ### @param  調べる文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return TRUE 数値が入っている
  def checkInteger(value, row, col)
    if checkNil(value, row, col)
      return FALSE
    elsif value =~ /[^0-9\+\-\.]/
      errorStr = "Only Number! (数値しか受け付けません) #{value}"
      printAnalyzeError(errorStr, row, col)
      return FALSE
    elsif value.count(".") > 1
      errorStr = "Invalid Number! (数値表現が正しくありません) #{value}"
      printAnalyzeError(errorStr, row, col)
      return FALSE
    end
    return TRUE
  end

  ### 数値以外の文字が入っていないか調べる(範囲チェックあり)
  ### @param  調べる文字列
  ### @param  最小値
  ### @param  最大値
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return TRUE 数値が入っている
  def checkIntegerLimit(value, min, max, row, col)
    if checkInteger(value, row, col)
      i = value.to_i
      if i<min || i>max
	printAnalyzeError("#{min}から#{max}の値でなくてはなりません", row, col)
      end
    end
  end

  ### 数値以外の文字が入っていないか調べる(範囲チェックあり、浮動小数版)
  ### @param  調べる文字列
  ### @param  最小値
  ### @param  最大値
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return TRUE 数値が入っている
  def checkFloatLimit(value, min, max, row, col)
    if checkInteger(value, row, col)
      i = value.to_f
      if i<min || i>max
	printAnalyzeError("#{min}から#{max}の値でなくてはなりません", row, col)
      end
    end
  end

  ### C++のラベルに使用できる文字以外が入っていないか調べる
  ### @param  調べる文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return 不定
  def checkCppLabel(value, row, col)
    if checkNil(value, row, col)
    elsif value =~ /^[0-9]/
      # エラー
      errorStr = "Illegal Label! (ラベル名の先頭に数字は使えません) #{value}"
      printAnalyzeError(errorStr, row, col)
    elsif value =~ /[^A-Za-z0-9\_]/
      # エラー
      errorStr = "Illegal Label! (ラベル名には[A-Z0-9_]しか使えません) #{value}"
      printAnalyzeError(errorStr, row, col)
    end
  end

  ### ラベル = 値 という形式になっているか調べる
  ### @param  調べる文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return 不定
  def checkLabelValue(value, row, col)
    if checkNil(value, row, col)
    elsif value =~ /^[0-9]/
      # エラー
      errorStr = "Illegal Label! (ラベル名の先頭に数字は使えません) #{value}"
      printAnalyzeError(errorStr,row,col)
    elsif ! (value =~ /[A-Z0-9\_]+\s*=\s*\S+/)
      # エラー
      errorStr = "Illegal Label! (ラベル=値 という書式になっていません) #{value}"
      printAnalyzeError(errorStr, row, col)
    end
  end

  ### 登場ゲームタイプにふさわしくない文字列が入っていないか調べる
  ### @param  調べる文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return 不定
  def checkGameTypeLabel(value, row, col)
    if checkNil(value, row, col)
    elsif value != "ONLINE" and value != "FIRST" and value != "ROOM"
      # エラー
      errorStr = "Illegal Game Type! (ONLINE, FIRST, ROOM以外使えません) #{value}"
      printAnalyzeError(errorStr, row, col)
    end
  end


  ### nilじゃないか調べる
  ### @param  調べる文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return nilならTRUE
  def checkNil(value, row, col)
    if value.nil?
      # エラー
      errorStr = "No value! (セルが空白です) "
      printAnalyzeError(errorStr, row, col)
      return TRUE
    end
    return FALSE
  end

  ### セルの値を返す
  ### @param  セルの値が入っているxml文字列
  ### @return セルの値
  def getValue(str)
    str =~ /\>\s*(.+)\s*\<\//i
    value = $1
    # タグは全部削除(実際余計な位置に<FONTがはいっていたことがあった)
    value.gsub!(/<[^>]+>/, "") if (! value.nil?)
    return value
  end


  ### .xlsの解析結果を表示する
  ### @param  なし
  ### @return エラー個数
  def printAnalyzeResult
    str = ""
    if @numError == 0
      str = "#{File.basename($0)} : Converting Finished. (正常にコンバートできました)"
    else
      str = "#{File.basename($0)} : Number of Errors #{@numError} (エラーが#{@numError}個ありました)"
    end
    puts "+" + ("-" * (str.size+2)) + "+"
    puts "| " + str + " |"
    puts "+" + ("-" * (str.size+2)) + "+"
    raise AnalyzeException if @numError != 0
    puts ""
  end

  ### .xlsの解析エラーを表示し、エラーカウンタを進める
  ### @param  エラー文字列
  ### @param  元のセルのrow
  ### @param  元のセルのcol
  ### @return エラー個数
  def printAnalyzeError(errorStr, row, col)
    colStr = ""
    loop {
      colStr = "A" + colStr
      colStr[0] += (col % 26)
      col = (col / 26).floor - 1     # Floatで来たときのために切り捨て処理入れる
      if (col < 0) 
	break
      end
    }
    # colStr = "A"
    # colStr[0] += col
    str = "CELL:(" + colStr + ":" + row.to_s + ") " + errorStr
    printError(str)
    @numError += 1
  end

  ### エラー文字列を標準出力する
  ### @param  エラー文字列
  ### @return 不定
  def printError(str)
    print "Error: " + str + "\n"
  end
end


