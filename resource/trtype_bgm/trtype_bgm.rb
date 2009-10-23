#=======================================================================
# trtype_bgm.rb
# トレーナータイプ別に再生する視線BGM表を作成
#
# 引数　tr_bgm.rb fpath_csv fpath_cdat fpath_def
# fpath_csv 管理表csvファイルパス
# fpath_cdat 作成するcdatファイルパス
# fpath_def 参照するトレーナータイプ定義ファイル
# print_error "1"=エラー出力を行う "0"=行わない
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================
STR_TRTYPE_MAX = "TRTYPE_MAX"

RET_TRUE = (1)
RET_FALSE = (0)

STR_TRUE = "1"
STR_FALSE = "0"

#=======================================================================
# サブルーチン
#=======================================================================
#-----------------------------------------------------------------------
#	ヘッダーファイル内検索
#	search 検索文字列
#	戻り値 RET_TRUE=存在する
#-----------------------------------------------------------------------
def hfile_search( hfile, search )
	hfile.pos = 0
	
	search = search.strip #先頭末尾の空白、改行削除
	
	while line = hfile.gets
		if( line =~ /\A#define/ )
			len = line.length
			str = line.split() #空白文字以外羅列
			
			if( str[1] == search )	#1 シンボル名
        return RET_TRUE
      end
		end
	end
  
	return RET_FALSE
end

#=======================================================================
# 変換
#=======================================================================
fpath_csv = ARGV[0]
fpath_cdat = ARGV[1]
fpath_def = ARGV[2]
print_error = ARGV[3]
file_csv = File.open( fpath_csv, "r" )
file_cdat = File.open( fpath_cdat, "w" )
file_def = File.open( fpath_def, "r" )

file_cdat.printf( "//%s\n", fpath_cdat )
file_cdat.printf( "//このファイルはコンバータから出力されました\n\n" )
file_cdat.printf( "//[][trainer type, trainer eye bgm]\n" )
file_cdat.printf( "unsigned short DATA_TrainerTypeToEyeBGMNo[][2] = {\n" )

line = file_csv.gets #データ開始位置まで進める

i = 0

while line = file_csv.gets
  line = line.strip
  str = line.split( "," )
  
  if( str[0] != nil && str[0] != "" &&
      str[1] != nil && str[1] != "" )
    type = "TRTYPE_" + str[0].upcase
    
    if( hfile_search(file_def,type) == RET_FALSE )
      if( print_error == STR_TRUE )
        printf( "トレーナー視線BGM対応表 %s は存在していません\n", type )
      end
      file_cdat.printf( "//" )
    end
    
    file_cdat.printf( "  {%s,%s},", type, str[1] )
    
    if( str[2] != nil && str[2] != "" )
      file_cdat.printf( " //%s\n", str[2] )
    else
      file_cdat.printf( "\n" )
    end
  end
  
  i = i + 1
end

file_cdat.printf( "  {%s,0}, //end\n", STR_TRTYPE_MAX )
file_cdat.printf( "};" )
file_def.close
file_cdat.close
file_csv.close
