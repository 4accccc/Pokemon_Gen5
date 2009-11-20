#=======================================================================
# fld_item.rb
# _EVENT_DATA記述をfld_item.evに出力する
#
# 引数 fld_item.rb path_csv path_id path_sc
# path_csv アイテム管理表csvファイルパス
# path_sc データに記述するスクリプトファイルパス
# path_id アイテムID定義ファイル
#
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================
FNAME_TARGET = "fld_item.ev"

CSVNO_MAPNAME = 0
CSVNO_MAPLABEL = 1
CSVNO_X = 2
CSVNO_Y = 3
CSVNO_ITEMNAME = 4
CSVNO_NUM = 5
CSVNO_ITEMSYM = 6
CSVNO_MAX = 7

FLAGNO_MAX = 328

STR_COMMENT0 = "//======================================================================"
STR_COMMENT1 = "//--------------------------------------------------------------"

#=======================================================================
# 関数
#=======================================================================
#------------------------------------------------------------
# エラー終了
#------------------------------------------------------------
def error_end( f0, f1, f2, f3, fpath )
  f0.close
  f1.close
  f2.close
  f3.close
  File.delete( fpath )
end

#------------------------------------------------------------
# アイテム名からアイテムIDシンボル取得
#------------------------------------------------------------
def get_itemsym( file, str_item )
	file.pos = 0
	
	while line = file.gets
		if( line =~ /\A#define/ && line.include?(str_item) )
			len = line.length
			str = line.split() #空白文字以外羅列
			return str[1] #1 シンボル名
    end
  end
	
	return nil
end

#------------------------------------------------------------
# csv配列　取得
#------------------------------------------------------------
def get_csvdata( data, pos, no )
  return data[ (pos*CSVNO_MAX)+no ]
end

#------------------------------------------------------------
# csv配列　セット
#------------------------------------------------------------
def set_csvdata( data, pos, no, set )
  data[ (pos*CSVNO_MAX)+no ] = set
end

#=======================================================================
# fld_item.ev生成
#=======================================================================
fpath_csv = ARGV[0]
fpath_id = ARGV[1]
fpath_sc = ARGV[2]
file_csv = File.open( fpath_csv, "r" )
file_sc = File.open( fpath_sc, "r" )
file_id = File.open( fpath_id, "r" )
file_ev = File.open( FNAME_TARGET, "w" )

file_ev.printf( "%s\n", STR_COMMENT0 )
file_ev.printf( "// %s\n", FNAME_TARGET )
file_ev.printf( "// %s,%sを元にコンバートしています\n", fpath_csv, fpath_id )
file_ev.printf( "// ●コンバート実行\n" )
file_ev.printf( "//   fld_item(.bat)\n" )
file_ev.printf( "// ●更新されるファイル\n" )
file_ev.printf( "//   fld_item.ev\n" )
file_ev.printf( "//   fld_item_def.h\n" )
file_ev.printf( "%s\n", STR_COMMENT0 )
file_ev.printf( "\n" )
file_ev.printf( "\t.text\n" )
file_ev.printf( "\t.include \"scr_seq_def.h\"\n " )
file_ev.printf( "\t.include \"../../message/dst/script/msg_fld_item.h\n" )
file_ev.printf( "\n" )
file_ev.printf( "%s\n", STR_COMMENT1 )
file_ev.printf( "// スクリプトテーブル\n" )
file_ev.printf( "%s\n", STR_COMMENT1 )

line = file_csv.gets #データ行まで飛ばす
line = file_csv.gets
line = file_csv.gets

#CSV抽出
count = 0
csvdata = Array.new()

while line = file_csv.gets
  line = line.strip
  str = line.split( "," )
  
  if( str == nil )
    break
  end
  
  set_csvdata( csvdata, count, CSVNO_MAPNAME, str[CSVNO_MAPNAME] )
  set_csvdata( csvdata, count, CSVNO_MAPLABEL, str[CSVNO_MAPLABEL] )
  set_csvdata( csvdata, count, CSVNO_X, str[CSVNO_X] )
  set_csvdata( csvdata, count, CSVNO_Y, str[CSVNO_Y] )
  set_csvdata( csvdata, count, CSVNO_ITEMNAME, str[CSVNO_ITEMNAME] )
  set_csvdata( csvdata, count, CSVNO_NUM, str[CSVNO_NUM] )

=begin
  printf( "count = %d\n", count )
  p get_csvdata( csvdata, count, CSVNO_MAPNAME )
  p get_csvdata( csvdata, count, CSVNO_MAPLABEL )
  p get_csvdata( csvdata, count, CSVNO_X )
  p get_csvdata( csvdata, count, CSVNO_Y )
  p get_csvdata( csvdata, count, CSVNO_ITEMNAME )
  p get_csvdata( csvdata, count, CSVNO_NUM )
=end

  #アイテムシンボル取得
  set_csvdata( csvdata, count, CSVNO_ITEMSYM,
    get_itemsym(file_id,get_csvdata(csvdata,count,CSVNO_ITEMNAME)) )
  
  if( get_csvdata(csvdata,count,CSVNO_ITEMSYM) == nil )
    printf( "ERROR アイテム名 %s はありません\n",
           get_csvdata(csvdata,count,CSVNO_ITEMNAME) )
    error_end( file_csv, file_sc, file_id, file_ev, fpath_ev )
    exit 1
  end

  #フラグマックスを超えていたらエラー
  strnum = get_csvdata( csvdata, count, CSVNO_NUM )
  if( strnum.to_i >= FLAGNO_MAX )
    printf( "ERROR フラグの最大数を超えています No=%s\n", strnum )
    error_end( file_csv, file_sc, file_id, file_ev, fpath_ev )
    exit 1
  end
  
  #同じ番号(フラグナンバーのオフセット)を使用していないかチェック
  no = 0
  while no < count
    if( get_csvdata(csvdata,no,CSVNO_NUM) ==
        get_csvdata(csvdata,count,CSVNO_NUM) )
      printf( "ERROR 同じ値の定義があります リスト位置 %d,%d\n", no, count )
      error_end( file_csv, file_sc, file_id, file_ev, fpath_ev )
      exit 1
    end
    no = no + 1
  end
  
  count = count + 1
end

#イベントデータ定義
no = 0

while no < count
  file_ev.printf( "_EVENT_DATA ev_fld_item_%s\n",
    get_csvdata(csvdata,no,CSVNO_NUM) )
  no = no + 1
end

file_ev.printf( "_EVENT_DATA ev_fld_item2\n" )
file_ev.printf( "_EVENT_DATA_END\n\n" )

#イベントデータ　スクリプト定義
no = 0
while no < count
  #見出し
  file_ev.printf( "%s\n", STR_COMMENT1 )
  file_ev.printf( "// MAP=%s X=%s Y=%s ITEM=%s FLAG=%s\n",
    get_csvdata(csvdata,no,CSVNO_MAPLABEL),
    get_csvdata(csvdata,no,CSVNO_X),
    get_csvdata(csvdata,no,CSVNO_Y),
    get_csvdata(csvdata,no,CSVNO_ITEMNAME),
    get_csvdata(csvdata,no,CSVNO_NUM) )
  file_ev.printf( "%s\n", STR_COMMENT1 )
  
  #スクリプト定義
  file_ev.printf( "EVENT_START ev_fld_item_%s\n",
    get_csvdata(csvdata,no,CSVNO_NUM) )
  file_ev.printf( "  _FLD_ITEM_EVENT %s,1\n",
    get_csvdata(csvdata,no,CSVNO_ITEMSYM) )
  file_ev.printf( "EVENT_END\n\n" )

  no = no + 1
end

#ベーススクリプト出力
file_ev.printf( "\n" )
file_ev.printf( "%s\n", STR_COMMENT0 )
file_ev.printf( "// %s\n", fpath_sc )
file_ev.printf( "%s\n", STR_COMMENT0 )
file_ev.write( file_sc.read )

file_ev.close
file_sc.close
file_id.close
file_csv.close
