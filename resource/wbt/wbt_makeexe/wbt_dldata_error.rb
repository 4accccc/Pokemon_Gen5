#!ruby -Ks
#---------------------------
# wbt_dldata_error.rb
#
# エラーファイル
#---------------------------

require "wbt_dldata_util.rb"

# 文字列
# header
$err_header = [
  "#----------------",
  "# error log start",
  "#----------------",
  "",
]
# footer
$err_footer = [
  "",
  "#----------------",
  "# error log end",
  "#----------------",
]

# mes
$err_mes = [
  "セルが空白です",                # 空白
  "入力値が正しくありません",      # 数値入力
  "最大文字数を超えています",      # 文字入力
  "入力が正しくありません",        # 入力規則
  "特性が正しくありません",        # 特性エラー
]

# mes2
$err_mes_index = [
  "●エラーメッセージ",
  "●場所",
  "●内容",
]

#------------------------------------------------
#------------------------------------------------
#------------------
# 設定ファイル
#------------------
class WBT_ERROR
  #----------
  # 初期化
  #----------
  def initialize()
    @num = 0
  end
  
  #----------
  # 読み込み
  #----------
  def open( name )
    @errfile = File.open( name, "w" )
    #
    write_fix( 0 )
  end
  
  #----------
  # 固定表示
  #----------
  def write_fix( mode )
    if(mode == 0)
      size = $err_header.size;
      size.times{|i|
        @errfile.printf("%s\n",$err_header[i])
      }
    elsif(mode == 1)
      size = $err_footer.size;
      size.times{|i|
        @errfile.printf("%s\n",$err_footer[i])
      }
    end
  end
  
  #----------------------------------------
  # 空白エラー(ファイル名、シート名、col、row)
  #----------------------------------------
  def print_blank_error( filename, sheetnum, col, row )
    @num = @num+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n", $err_mes[0] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("[%s] : [%s] : [ %d, %s ]\n", filename, $sheet_str[sheetnum], col, $cell_row[row-1] )
    @errfile.printf("******************************************************\n")
  end
  
  #----------------------------------------------------------------
  # 範囲エラー(ファイル名、シート名、col、row、str、num、min、max)
  #----------------------------------------------------------------
  def print_range_error( filename, sheetnum, col, row, str, num, min, max )
    @num = @num+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n", $err_mes[1] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("[%s] : [%s] : [ %d, %s ]\n", filename, $sheet_str[sheetnum], col, $cell_row[row-1] )
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("[%s] : [%d] : [ %d - %d ]\n", str, num, min, max )
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 入力エラー(ファイル名、シート名、col、row、str、mstr)
  #----------------------------------------------------------------
  def print_hash_error( filename, sheetnum, col, row, str, mstr )
    @num = @num+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n", $err_mes[3] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("[%s] : [%s] : [ %d, %s ]\n", filename, $sheet_str[sheetnum], col, $cell_row[row-1] )
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("[%s] : [%s]\n", str, mstr )
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 文字数エラー(ファイル名、シート名、col、row、str、array)
  #----------------------------------------------------------------
  def print_mes_error( filename, sheetnum, col, row, str, array, line, cnum )
    cbuf = [0, 0, 0]
    line.times{|i|
      cbuf[i] = cnum;
    }
    @num = @num+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n", $err_mes[2] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("[%s] : [%s] : [ %d, %s ]\n", filename, $sheet_str[sheetnum], col, $cell_row[row-1] )
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("[%s] : (limit)[%d:%d,%d,%d] => (error)[%d:%d,%d,%d]\n",
				str, line, cbuf[0], cbuf[1], cbuf[2], array[0],array[1],array[2],array[3] )
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 特性エラー(ファイル名、シート名、col、row)(仮)
  #----------------------------------------------------------------
  def print_tokusei_error( filename, sheetnum, col, row )
    @num = @num+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n", $err_mes[4] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("[%s] : [%s] : [ %d, %s ]\n", filename, $sheet_str[sheetnum], col, $cell_row[row-1] )
    @errfile.printf("%s\n", $err_mes_index[2] )
#    @errfile.printf("[%s] : [%s]\n", str, mstr )
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------
  # 終了処理
  #----------
  def close()
    write_fix( 1 )
    #
    @errfile.close()
  end
  
end

