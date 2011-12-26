#----------------------------------------------------------------------------
# ポケウッドシナリオ興行成績算出ツール
# 2011/12/24 by mori
# 
# 入力：ruby senario_move.rb <テンプレートシート> <出力ファイル名>
# 出力：<テンプレート>シートに<入力シナリオデータ>の内容をコピーしたものを<出力ファイル名>で
#       出力する
#----------------------------------------------------------------------------require 'excel_lib'
require 'excel_lib'
require 'fileutils'

USE_STRING    = "ruby senario_move.rb <テンプレ.xls> <出力.xls>\n"
NOFILE_STRING = "%sというファイルがありません\n"

ending_str = ["失敗","クリア"]

#ヘッダー部位置指定文字列
header_pos=[ "B4","B8","B9" ]  # 作品係数、最大ターン、ベストターン


#イベントデータ取り出し用y座標
event_start_pos=[ 70, 88, 106, 124, 142, 160, 178, 196, 214, 232, 250, 268 ]

#オフセット→位置指定文字列変換用テーブル
$pos_name = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", ]

$event_name_hash = { "イベント１"=>0,   "イベント２"=>1,   "イベント３"=>2,   "イベント４"=>3,   "イベント５"=>4, 
                     "イベント６"=>5,   "イベント７"=>6,   "イベント８"=>7,   "イベント９"=>8,   "イベント１０"=>9, 
                     "イベント１１"=>10,"イベント１２"=>11, 
                    }

#イベント内部データオフセット配列
$event_offset=[ 
  [3,0],[9,0],[11,13],[11,14],[11,15],[11,16],
]


#----------------------------------------------------------------
# セル内容コピー関数
#----------------------------------------------------------------
def cell_copy( inSheet, outSheet, inpos, out_x, out_y )
  inRange = inSheet.range(inpos).get_cursol
  if inRange.text!=nil then
    posstr = get_posstr( out_x, out_y )
    outRange = outSheet.range(posstr).get_cursol
    #通常コピー
    outRange.text = inRange.text
  end
end

#----------------------------------------------------------------
# イベント内容コピー関数
#----------------------------------------------------------------
def event_copy( outSheet, str, x, y )
  posstr = get_posstr( x,y )
  outRange = outSheet.range(posstr).get_cursol
  outRange.text = str
end

def get_event_value( inSheet, in_x, in_y, offset_x, offset_y )
  posstr = get_posstr( in_x+offset_x, in_y+offset_y )
  inRange = inSheet.range(posstr).get_cursol
  return inRange.text
end

def event_check_and_get( inSheet, outSheet, start_x, start_y, ending_str, i, success_event_str, no )

  #イベント種類を取得
  str = get_event_value( inSheet, start_x, start_y, $event_offset[0][0], $event_offset[0][1] )

  #失敗イベントか？
  if str==ending_str[0] then
    point = get_event_value( inSheet, start_x, start_y, $event_offset[1][0], $event_offset[1][1] )
    if point>0 then #ストレンジ
      event_copy( outSheet, point, 6, i+2 )
    else            #失敗
      event_copy( outSheet, point, 7, i+2 )
    end

  #成功イベントか
  elsif str==ending_str[1] then
    point = get_event_value( inSheet, start_x, start_y, $event_offset[1][0], $event_offset[1][1] )
    #予想で40以上だったらストレンジに入れる
    if point>40 then
      event_copy( outSheet, point, 6, i+2 )
    else
      printf("clear event=%d\n",no)
      if no==$event_name_hash[success_event_str] then
        event_copy( outSheet, point, 5, i+2 )
      end
    end
  end

  #選択肢の欄にポイントが入っているか
  choice_p = [0,0,0,0]
  choice_p[0] = get_event_value( inSheet, start_x, start_y, $event_offset[2][0], $event_offset[2][1] )
  choice_p[1] = get_event_value( inSheet, start_x, start_y, $event_offset[3][0], $event_offset[3][1] )
  choice_p[2] = get_event_value( inSheet, start_x, start_y, $event_offset[4][0], $event_offset[4][1] )
  choice_p[3] = get_event_value( inSheet, start_x, start_y, $event_offset[5][0], $event_offset[5][1] )

  #入っていたら選択肢ポイントとして集計
  4.times{|n|
    if choice_p[n]!=nil then
      if choice_p[n]>0 then
        event_copy( outSheet, choice_p[n], 12, i+2 )
      end
      if choice_p[n]<0 then
        event_copy( outSheet, choice_p[n], 13, i+2 )
      end
      
    end
  }
end
#----------------------------------------------------------------
# オフセットから位置文字列を生成する
#----------------------------------------------------------------
def get_posstr( x, y )
  return "" + $pos_name[x]+y.to_s
end



#引数チェック
if ARGV[0]==nil || ARGV[1]==nil then
  printf( USE_STRING )
  exit(1)
end

#ファイル存在チェック
if FileTest.exist?(ARGV[0])==false then
  printf(NOFILE_STRING,ARGV[0])
  exit(1)
end


#エクセルライブラリ起動
excel=Excel.new(false)

#シート開く
temp_book  = excel.copy_book( ARGV[0] )
temp_sheet  = temp_book.sheets[2]


40.times{|i|
  openfilename = sprintf( "pokewood_senario%02d.xls", i+1 )
  input_book = excel.open_book( openfilename )
  input_sheet = input_book.sheets[1]
  
  #ヘッダ項目コピー
  cell_copy( input_sheet, temp_sheet, header_pos[0], 2, i+2 )
  cell_copy( input_sheet, temp_sheet, header_pos[1], 3, i+2 )
  cell_copy( input_sheet, temp_sheet, header_pos[2], 4, i+2 )

  #成功イベント名を取得
  success_event_str = get_event_value( input_sheet, 1, 7, 0, 0 )
  
  
  #イベント項目移動
  event_start_pos.each_with_index{|y,no|
    event_check_and_get( input_sheet, temp_sheet, 0, y, ending_str, i, success_event_str,no )
  }
  printf( "シナリオ%02dを処理,", i+1)
  printf( "成功イベント=%s,no=%d\n",success_event_str, $event_name_hash[success_event_str])

}


#シート保存
temp_book.save( ARGV[1] )

#エクセルライブラリ終了
excel.quit
