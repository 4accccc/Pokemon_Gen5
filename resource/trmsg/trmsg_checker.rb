#!ruby -Ks
#-----------------------------------------------
# trmsg_checker.rb
#------------------------------------------------

require "my_tab_out_direct_sheet.rb"
#
# 対象となるTRIDの定義ファイル
TRMSGFILE = "../../prog/include/tr_tool/trno_def.h"
#TRMSGFILE = "c:/home/swan/prog/include/tr_tool/trno_def.h"

# メッセージファイル
MSGXLS = "wb_trainermsg.xls"
#MSGXLS = "c:/home/swan/resource/trmsg/wb_trainermsg.xls"

# エラー
ERRFILE = "err.txt"

# 検索条件
STR_LIST   = ["TRID_"]

# 除外する
SKIP_LIST  = ["TRID_NULL","TRID_MAX"]
SKIP_LIST2 = ["#","＃"]
#
$usage_str = [
  "\nusage\nruby trmsg_checker.rb\n",
]
#
$title_str = ["(trno_def.h)にあるが(wb_trainermsg.xls)にないID",
              "(wb_trainermsg.xls)にあるが(trno_def.h)にないID",
              "(wb_trainermsg.xls)にIDはあるがメッセージがない",
]

# debug(0:なし、1:情報表示)
$debug_flag = 0

# エラーフラグ
$err_flag = 0

#-----------------------
# 文字列を検出
# (TRID_**) => (**)
#-----------------------
def get_list_define( fname )
  mlist = Array.new
  
  # 判定
  File.open( fname, "r" ){|f|
    f.each{|i|
      if i.include?(STR_LIST[0]) == true
        i = i.strip
        i  = i.sub(/#define/, "")
        i  = i.gsub(/ /, "")
        i  = i.sub(/\t/, "")
        ary = i.split("(")
        #
        # skip部分をチェック
        if((ary[0] != SKIP_LIST[0])&&(ary[0] != SKIP_LIST[1]))
          # メッセージ部分をセット
          ary[0] = ary[0].sub(/TRID_/, "")
          ary[0] = ary[0].downcase
          mlist.push( ary[0] )
        end
      end
    }
  }
  return( mlist )
end

#-------------------------
# 文字列を検出
# (エクセルからtridを検出)
#-------------------------
def get_list_xls( conv )
  mlist = Array.new
  #
  # 縦方向のサイズ
  height = conv.get_row_size( 0 )
  
  # 縦方向を調べる
  height.times{|i|
    str = conv.get_cell(0, 1+i, 1 );
    if(str != nil)
      if((str.include?(SKIP_LIST2[0]) == false)&&
         (str.include?(SKIP_LIST2[1]) == false))
#        puts str
         # IDに該当する文字列を見つけた
         mlist.push( str )
      end
    end
  }
  return(mlist)
end

#-------------------------
# エラーをチェック
# (エクセルからtridを検出し、
# メッセージの有無を調べる)
#-------------------------
def chk_list_xls_error( conv )
  # 縦方向のサイズ
  height = conv.get_row_size( 0 )
  
  # 縦方向を調べる
  flg = 0
  num = 0
  idstr = ""
  height.times{|i|
    str = conv.get_cell(0, 1+i, 1 );
    if(str != nil)
      if((str.include?(SKIP_LIST2[0]) == false)&&
         (str.include?(SKIP_LIST2[1]) == false))
#        puts str
         # IDに該当する文字列を見つけた
         idstr = str
         flg = 1
         # メッセージチェック
         mstr = conv.get_cell(0, 1+i, 3)
         if(mstr == nil)
#          printf("%-10s : (%4d,C)\n",idstr, (1+i))
           $errfile.printf("%-10s : (%4d,C)\n", idstr, (1+i) )
           num = num+1
         end
         mstr = conv.get_cell(0, 1+i, 4)
         if(mstr == nil)
#          printf("%-10s : (%4d,D)\n",idstr, (1+i))
           $errfile.printf("%-10s : (%4d,D)\n", idstr, (1+i) )
           num = num+1
         end
      else
        # SKIPのコードがある場合
        # => 文字列チェックはしない
        flg = 0
      end
    else
      # IDがある場合は文字列のチェック
      # すでにない場合は処理しない
      if(flg == 1)
        mstr = conv.get_cell(0, 1+i, 3)
        if(mstr == nil)
#          printf("%-10s : (%4d,C)\n",idstr, (1+i))
           $errfile.printf("%-10s : (%4d,C)\n", idstr, (1+i) )
           num = num+1
        end
        mstr = conv.get_cell(0, 1+i, 4)
        if(mstr == nil)
#          printf("%-10s : (%4d,D)\n",idstr, (1+i))
           $errfile.printf("%-10s : (%4d,D)\n", idstr, (1+i) )
           num = num+1
        end
      end
    end
  }
  puts "total: " + num.to_s
  $errfile.printf("total: %d\n", num )
  return( num )
end

#======================================================
#------------
# 処理開始
#------------
#------------
# 引数チェック
#------------
inname = TRMSGFILE
unless FileTest.exists?( inname )
  puts "ファイル :" + inname + " が見つかりません"
  puts $usage_str[0]
  exit(1)
end

# ファイル名
if($debug_flag == 1)
  puts "ファイル: " + inname
end

#----------------
# エラーファイル
#----------------
$errfile = File.open( ERRFILE, "w" )

#---------------------
# ヘッダから情報を取得
#---------------------
list = Array.new
list = get_list_define( inname )

if($debug_flag == 1)
  puts list
end
puts "total(h)   = " + list.size.to_s
puts "-----------------------------"
$errfile.printf("total(h)   = %d\n", list.size )
$errfile.printf("-----------------------------\n")


#------------------------
# エクセルから情報を取得
#------------------------
conv = MY_ExcelConv.new();
begin   # begin

# 開く
filename = getAbsolutePath( MSGXLS );
conv.open_file( filename );

xlist = Array.new
xlist = get_list_xls( conv )

if($debug_flag == 1)
  puts xlist
end
puts "total(xls) = " + xlist.size.to_s
puts "-----------------------------"
$errfile.printf("total(xls) = %d\n", xlist.size )
$errfile.printf("-----------------------------\n")

# エラーチェック
puts $title_str[2]
$errfile.printf("%s\n", $title_str[2] )
num = chk_list_xls_error( conv );
if(num != 0)
  $err_flag = 1;
end
puts "-----------------------------"
$errfile.printf("-----------------------------\n")

ensure
  if($debug_flag == 1)
    p sprintf("ensure !")
  end
  #
  conv.close_file()
  conv.exit()
end


#------------------------
# 一致しないものを表示
#------------------------
# ヘッダ内の各IDがxls内にあるかをチェック
num = 0
puts $title_str[0]
$errfile.printf("%s\n", $title_str[0] )
size = list.size
size.times{|i|
  str = list[i];
  if(xlist.include?( str ) == true)
    if($debug_flag == 1)
      puts "ok    :" + list[i]
    end
  else
#    puts list[i]
    $errfile.printf("%s\n", list[i] )
    num = num+1
  end
}
puts "total: " + num.to_s
$errfile.printf("total: %d\n", num )
if(num != 0)
  $err_flag = 1;
end


# xls内の各IDがヘッダ内にあるかをチェック
num = 0
puts "-----------------------------"
puts $title_str[1]
$errfile.printf("-----------------------------\n")
$errfile.printf("%s\n", $title_str[1] )
size = xlist.size
size.times{|i|
  str = xlist[i];
  if(list.include?( str ) == true)
    if($debug_flag == 1)
      puts "ok    :" + xlist[i]
    end
  else
    puts xlist[i]
    $errfile.printf("%s\n", xlist[i] )
    num = num+1
  end
}
puts "total: " + num.to_s
$errfile.printf("total: %d\n", num )
if(num != 0)
  $err_flag = 1;
end

if($err_flag == 1)
  puts
  puts "==================================================="
  puts "エラーがありますので(err.txt)を確認してください"
  puts "==================================================="
end

#-----------------
# エラーファイル
#-----------------
$errfile.close()

=begin

=end

