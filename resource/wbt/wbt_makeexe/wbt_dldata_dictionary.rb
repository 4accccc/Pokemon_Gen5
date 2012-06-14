#ruby -Ks
#-----------------------
# 辞書ファイルの読み込み
#-----------------------

$MONSNO_MAX = 649

class WBT_DICTIONARY
  POKE_START_ROW = 4
  POKE_START_COL = 2
  $row_ofs = [ 0, 2, 3, 4, 5, 6, 7 ]

  def initialize( conv )
    @conv = conv
    @dictionary = Array.new
    LANG_MAX_NUM.times{|lang_ofs|
      local_dict = Array.new
      ($MONSNO_MAX+1).times{|num|
        pos_row = POKE_START_ROW + $row_ofs[lang_ofs]  #行
        pos_col = POKE_START_COL + num       #列
        #STDERR.printf("(col,row) = (%d,%d)\n", pos_col, pos_row )
        local_dict << get_unicode( conv, 0, pos_col, pos_row )
      }
      @dictionary << local_dict
      #STDERR.printf("lang_id=%d ", lang_ofs )
      #STDERR.puts @conv.get_cell(0, POKE_START_COL+1, POKE_START_ROW + $row_ofs[lang_ofs])
      #STDERR.puts @dictionary[lang_ofs][1]
    }
  end

  def get_local( id )
    @dictionary[ id ]
  end
end
