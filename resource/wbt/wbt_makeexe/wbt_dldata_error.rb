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
  "セルが空白です",                      # 空白
  "値が正しくありません",                # 数値入力
  "最大文字数を超えています",            # 文字入力
  "入力が正しくありません",              # 入力規則
  "特性の組み合わせが正しくありません",  # 特性エラー
  "努力値の合計が510を超えています",     # 努力値エラー
  "技の入力が正しくありません",          # 技
  "ポケモンの数が足りません",            # ポケモン数
  "トレーナーの数が足りません",          # トレーナー数
  "データ設定数が4を超えています",       # データ設定エラー
]

# mes2
$err_mes_index = [
  "●エラーメッセージ",
  "●場所",
  "●内容",
  "ファイル ",
  "シート   ",
  "セル     ",
  "値       ",
  "範囲     ",
  "( 範囲外 )",
  "( 文字数オーバー )",
  "( 該当なし )",
  "( 組み合わせエラー )",
  "( 設定エラー )",
  "( 重複エラー )",
  "トレーナー",
  "人目",
  "参加ポケモン数下限",
  "以上",
  "トレーナー管理のデータ設定数",
  "体",
  "人",
]

$em2 = [
  "行",
  "文字",
]

$em3 = [
  "ポケモン名",
  "フォルムNo",
  "特性      ",
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
    @subnum = 0
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
  
  #--------------------------------------------------
  # 空白エラー(ファイル名、シート名、col、row, str)
  #--------------------------------------------------
  def print_blank_error( filename, sheetnum, col, row, str )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[0] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] ( %s )\n", $err_mes_index[5],col, $cell_row[row-1],str)
    @errfile.printf("******************************************************\n")
  end
  
  #----------------------------------------------------------------
  # 範囲エラー(ファイル名、シート名、col、row、str、num、min、max)
  #----------------------------------------------------------------
  def print_range_error( filename, sheetnum, col, row, str, num, min, max )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[1] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] ( %s )\n\n", $err_mes_index[5],col, $cell_row[row-1], str)
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%d] %s\n", $err_mes_index[6], num, $err_mes_index[8])
    @errfile.printf("%s: [%d - %d]\n",$err_mes_index[7], min, max )
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 入力エラー(ファイル名、シート名、col、row、str、mstr)
  #----------------------------------------------------------------
  def print_hash_error( filename, sheetnum, col, row, str, mstr )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[3] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] ( %s )\n\n", $err_mes_index[5],col, $cell_row[row-1], str)
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%s] %s\n", $err_mes_index[6], mstr, $err_mes_index[10])
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
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[2] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] ( %s )\n\n", $err_mes_index[5],col, $cell_row[row-1], str)
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%d%s: %d%s, %d%s, %d%s] %s\n",
			$err_mes_index[6], array[0], $em2[0], 
			array[1],$em2[1],array[2],$em2[1],array[3],$em2[1],$err_mes_index[9])
    @errfile.printf("%s: [%d%s: %d%s, %d%s, %d%s]\n",
			$err_mes_index[7], line, $em2[0], 
			cbuf[0],$em2[1],cbuf[1],$em2[1],cbuf[2],$em2[1])
    @errfile.printf("******************************************************\n\n")
  end
  
  #-----------------------------------------------------------------------
  # 特性エラー(ファイル名、シート名、col、row、pokename、formid、tokusei)
  #-----------------------------------------------------------------------
  def print_tokusei_error( filename, sheetnum, col, row, pokename, formid, tokusei )
    
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[4] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s]\n\n", $err_mes_index[5],col, $cell_row[row-1])
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%s]\n", $em3[0], pokename)
    @errfile.printf("%s: [%d]\n", $em3[1], formid)
    @errfile.printf("%s: [%s]\n", $em3[2], tokusei)
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 努力値エラー(ファイル名、シート名、col、row、total)
  #----------------------------------------------------------------
  def print_effort_error( filename, sheetnum, col, row, total )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[5] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] - [%d, %s]\n\n", $err_mes_index[5], col, $cell_row[row-1], col, $cell_row[row-1+5])
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%d]\n", $err_mes_index[6], total)
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # 技エラー(ファイル名、シート名、col、row、str、mstr, mode)
  #----------------------------------------------------------------
  def print_skill_error( filename, sheetnum, col, row, str, mstr, mode )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[6] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: [%d, %s] ( %s )\n\n", $err_mes_index[5],col, $cell_row[row-1], str)
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%s] %s\n", $err_mes_index[6], mstr, $err_mes_index[12+mode])
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # ポケモン数エラー(ファイル名、シート名、trnum、pnum, minnum)
  #----------------------------------------------------------------
  def print_poke_error( filename, sheetnum, trnum, pnum, minnum )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[7] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: (%s %d%s)\n\n", $err_mes_index[5],$err_mes_index[14],(trnum+1),$err_mes_index[15])
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%d%s] %s\n", $err_mes_index[6], pnum, $err_mes_index[19],$err_mes_index[8])
    @errfile.printf("%s: [%s %d%s%s]\n", $err_mes_index[7], $err_mes_index[16],minnum,$err_mes_index[19],$err_mes_index[17])
    @errfile.printf("******************************************************\n\n")
  end
  
  #----------------------------------------------------------------
  # トレーナー数エラー(ファイル名、シート名、trnum、minnum)
  #----------------------------------------------------------------
  def print_tr_error( filename, sheetnum, trnum, minnum )
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[8] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
    @errfile.printf("%s: (%s %d%s)\n\n", $err_mes_index[5],$err_mes_index[14],(trnum+1),$err_mes_index[15])
    @errfile.printf("%s\n", $err_mes_index[2] )
    @errfile.printf("%s: [%d%s] %s\n", $err_mes_index[6], trnum, $err_mes_index[20],$err_mes_index[8])
    @errfile.printf("%s: [%s %d%s]\n", $err_mes_index[7], $err_mes_index[18],minnum,$err_mes_index[17])
    @errfile.printf("******************************************************\n\n")
  end
  
  #--------------------------------------------------
  # データ設定エラー(ファイル名、シート名、num)
  #--------------------------------------------------
  def print_dataset_error( filename, sheetnum, num)
    @num = @num+1
    @subnum = @subnum+1
    @errfile.printf("******************************************************\n")
    @errfile.printf("%s\n", $err_mes_index[0] )
    @errfile.printf("%s\n\n", $err_mes[9] )
    @errfile.printf("%s\n", $err_mes_index[1] )
    @errfile.printf("%s: [%s]\n", $err_mes_index[3],filename)
    @errfile.printf("%s: [%s]\n", $err_mes_index[4],$sheet_str[sheetnum])
#    @errfile.printf("%s: [%d, %s] ( %s )\n", $err_mes_index[5],col, $cell_row[row-1],str)
    @errfile.printf("******************************************************\n")
  end
  
  #----------
  # 終了処理
  #----------
  def close()
    write_fix( 1 )
    #
    @errfile.close()
    #
    if(@num != 0)
      puts sprintf("*******************************\n")
      puts sprintf("エラーが[ %d ]個あります\n",@num);
      puts sprintf("*******************************\n")
    end
  end
  
  #----------
  # 終了処理
  #----------
  def reset_sub_error()
    @subnum = 0
  end
  
  #----------
  # 終了処理
  #----------
  def chk_sub_error()
    flg = 0
    if(@subnum > 0)
      flg = -1
    end
    # エラーの数をチェック
    return( flg )
  end
end

