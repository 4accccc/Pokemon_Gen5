#!ruby -Ks
#-----------------------------------------------
# pkwd_csv.rb
#------------------------------------------------

require "my_tab_out_direct_sheet.rb"
#
#XLSFILE = "pkwd_party01.xls"
PKWD_XLS = "pkwd_party"
EXT_XLS  = ".xls"

# 調べる場所
PKWD_UP_ROW = 3
PKWD_UP_COL = 5
#
PKWD_DOWN_ROW = 3
PKWD_DOWN_COL = 30
#
POKE_MAX_NUM = 6

# 除外する
$skip_hash = {"━"=>0, "-"=>1, "－"=>2, "ー"=>3, }

# 参照するデータ
# 撮影ポケモン、フォルム、技1、技2、技3、技4
DATA_LIST  = [0, 3, 8, 9, 10, 11]

# ファイル最大
FILE_MAX_NUM = 41

# debug(0:なし、1:情報表示)
$debug_flag = 0

#-------------
# ハッシュ
#-------------
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

#-------------------------
# 情報取得(1体分)
# 
# conv:
# scenario:シナリオ番号
# num:(番号)
# mode:(0:上、1:下)
#-------------------------
def get_poke_info( conv, scenario, num, mode )
  mlist = Array.new
  #
  if mode == 0
    row = PKWD_UP_ROW
    col = PKWD_UP_COL
  else
    row = PKWD_DOWN_ROW
    col = PKWD_DOWN_COL
  end
  #
  str = conv.get_cell( 0, col, row+num )
  if(str != nil)
    # シナリオ番号
    mlist.push( scenario.to_s )
    # 自分、相手
    mlist.push( mode.to_s )
    # ポケモン番号
    mlist.push( num.to_s )
    #
    size = DATA_LIST.size()
    size.times{|i|
      # 参照
      str = conv.get_cell( 0, col+DATA_LIST[i], row+num )
      # 技チェック
      if( hash2no( $skip_hash, str ) >= 0)
        str = "0"
      end
      mlist.push( str )
    }
    
    # 出力してしまう
    if($debug_flag == 1)
      printf("%d,%d,%d,%s,%d,%s,%s,%s,%s,0,\n",
        mlist[0].to_i, mlist[1].to_i, mlist[2].to_i, mlist[3], mlist[4].to_i,
        mlist[5],      mlist[6],      mlist[7],      mlist[8] );
    end
    # ファイル
    $outfile.printf("%d,%d,%d,%s,%d,%s,%s,%s,%s,0,\n",
        mlist[0].to_i, mlist[1].to_i, mlist[2].to_i, mlist[3], mlist[4].to_i,
        mlist[5],      mlist[6],      mlist[7],      mlist[8] );
  end
end


#======================================================
#------------
# 処理開始
#------------
#------------
# 引数チェック
#------------
if ARGV[0] == nil
  puts "引数1に出力(xls)ファイル名を指定してください"
  exit(1)
end

#------------
# 出力ファイル
#------------
$outfile = File.open( ARGV[0], "w" )
# ファイル名
if($debug_flag == 1)
  puts "出力ファイル: " + ARGV[0]
end

#------------------------
# エクセルから情報を取得
#------------------------
conv = MY_ExcelConv.new();
begin   # begin

# 開く
FILE_MAX_NUM.times{|m|
  # printfのようにも使用
  # [pkwd_party**.xls]としたい
  str = "%s%02d%s" % [PKWD_XLS, m+1, EXT_XLS]
  if($debug_flag == 1)
    puts str
  end
  #
  filename = getAbsolutePath( str );
  conv.open_file( filename );
  
  # 出力(上)
  POKE_MAX_NUM.times{|i|
    get_poke_info( conv, m+1, i, 0 );
  }
  # 出力(下)
  POKE_MAX_NUM.times{|i|
    get_poke_info( conv, m+1, i, 1 );
  }
  # 閉じる
  conv.close_file()
}

ensure
  if($debug_flag == 1)
    p sprintf("ensure !")
  end
  #
  conv.close_file()
  conv.exit()
end

#-----------------
# 出力ファイル終了
#-----------------
$outfile.close();

puts "end"

=begin

=end

