#=======================================================================
#	fmmdl_mdllist.rb
#	フィールド動作モデル　モデルリスト作成
#	引数 fmmdl_mdllist.rb xlstxt residx binfilename codefilename codestrfilename
#	xlstxt 動作モデルリスト表 テキスト変換済みファイル名
#	residx リソースアーカイブデータインデックスファイル名
#	binfilename 作成するバイナリファイル名
#	codefilename 作成するOBJコードファイル名
#	codestrfilename 作成するOBJコード文字列ファイル名
#	symbolfilename 読み込むシンボルファイル名
#=======================================================================
$KCODE = "SJIS"
load "rbdefine"

#=======================================================================
#	パラメタフォーマット
#	0-3 モデル総数
#	4-  以下総数分のモデルパラメータ
#
#	パラメーターフォーマット
#	0-1 OBJコード
#	2-3 リソースアーカイブインデックス
#	4	表示タイプ
#	5	処理関数
#	6	影表示
#	7	足跡種類
#	8	映り込み
#	9	モデルサイズ
#	10	テクスチャサイズ
#	11	アニメID
#	12 性別
#	13-15 4bit差分用ダミー
#=======================================================================

#=======================================================================
#	作成されるOBJコード文字列ファイル構造
#	32byte単位でコードの数分格納されている。
#	0..31  コード0番のコード文字列 32byte ASCII 終端ヌル文字
#	32..63 コード1番文字列
#=======================================================================

#=======================================================================
#	定数
#=======================================================================
#戻り値
RET_TRUE = (1)
RET_FALSE = (0)
RET_ERROR = (0xfffffffe)

#リスト変換用ヘッダーファイル
FLDMMDL_LIST_H = "fldmmdl_list.h"

#固定文字列
STR_NULL = "" #NULL文字
STR_END = "#END" #終端文字列
STR_CODEMAX = "OBJCODEMAX" #コード最大
STR_DRAWTYPE_NON = "DRAWTYPE_NON" #表示タイプ無し

#表示コード文字列 一文字列最長 ヌル文字含む
CODESTRBUF = (16)

#=======================================================================
#	関数
#=======================================================================
#-----------------------------------------------------------------------
#	異常終了
#-----------------------------------------------------------------------
def error_end(
	path_delfile0, path_delfile1, path_delfile2,
	file0, file1, file2, file3, file4, file5 )
	printf( "ERROR fldmmdl list convert\n" )
	file0.close
	file1.close
	file2.close
	file3.close
	file4.close
	file5.close
	File.delete( path_delfile0 )
	File.delete( path_delfile1 )
	File.delete( path_delfile2 )
end

#-----------------------------------------------------------------------
#	モデルリスト用ヘッダーファイル内検索
#	search 検索文字列
#	戻り値 指定文字列の数値 RET_ERROR=ヒット無し
#-----------------------------------------------------------------------
def hfile_search( hfile, search )
	pos = hfile.pos
	hfile.pos = 0
	num = RET_ERROR
	
	search = search.strip #先頭末尾の空白、改行削除
	
	while line = hfile.gets
		if( line =~ /\A#define/ )
			len = line.length
			str = line.split() #空白文字以外羅列
			
			if( str[1] == search )	#1 シンボル名
				str_num = str[2]	#2 数値
				
				if( str_num =~ /\A0x/ ) #16進表記
					/([\A0x0-9a-fA-F]+)/ =~ str_num
					str_num = $1
					num = str_num.hex
				else					#10進表記
					/([\A0-9]+)/ =~ str_num
					str_num = $1
					num = str_num.to_i
				end
				
				break
			end
		end
	end
	
	hfile.pos = pos
	return num
end

#-----------------------------------------------------------------------
#	モデルリスト用アーカイブインデックスファイル内検索
#	search 検索文字列
#	戻り値 指定文字列の数値 RET_ERROR=ヒット無し
#-----------------------------------------------------------------------
def arcidx_search( idxfile, search )
	pos = idxfile.pos
	idxfile.pos = 0
	num = RET_ERROR
	
	search = search.strip #先頭末尾の空白、改行削除
	check = sprintf( "_%s_", search )
	
	while line = idxfile.gets
		if( line =~ /^enum.*\{/ )
			while line = idxfile.gets
				if( line.index(check) != nil )
					/(\s[0-9]+)/ =~ line
					str = $1
					num = str.to_i
					break
				end
			end
			
			break
		end
	end
	
	idxfile.pos = pos
	return num
end

#=======================================================================
#	コードファイル作成
#=======================================================================
#-----------------------------------------------------------------------
#	コードファイル記述
#	戻り値　RET_FALSE=異常終了
#-----------------------------------------------------------------------
def codefile_write( codefile, codestrfile, txtfile )
	no = 0
	
	pos = txtfile.pos
	txtfile.pos = 0 #先頭に
	line = txtfile.gets #一行飛ばし
	
	codefile.printf( "//動作モデル OBJコード定義\n" );
	
	path = codefile.path
	name = File.basename( path, "\.*" )
	name = name.upcase
	name = sprintf( "_%s_", name )
	codefile.printf( "#ifndef %s\n#define %s\n\n", name, name );
	
	loop{
		line = txtfile.gets
		
		if( line == nil )
			return RET_FALSE
		end
		
		if( line.include?(STR_END) )
			break
		end
		
		str = line.split( "," )
		
		codefile.printf( "#define %s (0x%x) //%d %s\n",
			str[RBDEF_NUM_CODE], no, no, str[RBDEF_NUM_CODENAME] );
		
		codestr = Array.new( CODESTRBUF );	#文字列バッファ
		codestr.fill( "\000".unpack('C*'), 0..CODESTRBUF ) #ヌル文字で埋め尽くし
		
		i = 0
		strbuf = str[RBDEF_NUM_CODE]
		while strbuf[i]
			codestr[i] = strbuf[i]
			i = i + 1
			if( i >= (CODESTRBUF-1) )
				break
			end
		end
		
		i = 0
		while i < CODESTRBUF		#バイト単位でバッファ長分書き込み
			c = Array( codestr[i] )
			codestrfile.write( c.pack("C*") )
			i = i + 1
		end
		
		no = no + 1
	}
	
	if( no == 0 )
		printf( "OBJコードが定義されていません\n" );
		return RET_FALSE
	end
	
	codefile.printf(
		"#define %s (0x%x) //%d %s\n\n", STR_CODEMAX, no, no, "最大" );
	
	codefile.printf( "#endif" );
	
	txtfile.pos = pos
	return RET_TRUE
end

#=======================================================================
#	動作モデルパラメタコンバート
#=======================================================================
#-----------------------------------------------------------------------
#	動作モデルデータ一行コンバート
#-----------------------------------------------------------------------
def convert_line( no, line, wfile, idxfile, symfile, sel_res )
	str = line.split( "," )
	
	#OBJコード 2
	ary = Array( no )
	wfile.write( ary.pack("S*") )
  
	#アーカイブインデックス テクスチャ 2 (4)
	word = str[RBDEF_NUM_DRAWTYPE]
	if( word == STR_DRAWTYPE_NON ) #表示タイプ　無し
		ret = 0
	else
    xlsline = RBDEF_NUM_RESFILE_NAME_0
    
    if( sel_res != "0" )
      xlsline = RBDEF_NUM_RESFILE_NAME_1
    end
    
		word = str[xlsline]
    
    #正規表現で文字列".imd"を削除する。が、何故か'm'文字まで削除してしまう
		#/(\A.*[^\.imd])/ =~ word
		#mdlname = $1
    #
    #代わりとしてgsubで".imd"を削除する。
    mdlname = word.gsub( "\.imd", "" )
    
		ret = arcidx_search( idxfile, mdlname )
		if( ret == RET_ERROR )
			printf( "ERROR モデルファイル名異常 %s\n", word )
			return RET_FALSE
		end
	end
	ary = Array( ret )
	wfile.write( ary.pack("S*") )
	
	#表示タイプ 1 (5)
	word = str[RBDEF_NUM_DRAWTYPE]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR 表示タイプ異常 %s\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#処理関数 1 (6)
	word = str[RBDEF_NUM_DRAWPROC]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR 処理関数異常異常 %s\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
  
	#影表示 1 (7)
	word = str[RBDEF_NUM_SHADOW]
	if( word != "○" )
		ret = 0 
	else
		ret = 1
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#足跡種類 1 (8)
	word = str[RBDEF_NUM_FOOTMARK]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR 足跡種類異常 %s\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#映り込み 1 (9)
	word = str[RBDEF_NUM_REFLECT]
	if( word != "○" )
		ret = 0
	else
		ret = 1
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#モデルサイズ 1 (10)
	word = str[RBDEF_NUM_MDLSIZE]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR モデルサイズ異常 %s\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#テクスチャサイズ 1 (11)
	word = str[RBDEF_NUM_TEXSIZE]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR テクスチャサイズ異常 %s\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
	#アニメID 1 (12)
	word = str[RBDEF_NUM_ANMID]
	ret = hfile_search( symfile, word )
	if( ret == RET_ERROR )
		printf( "ERROR アニメID異常 _%s_\n", word )
		return RET_FALSE
	end
	ary = Array( ret )
	wfile.write( ary.pack("C*") )
	
  #性別 1 (13)
  word = str[RBDEF_NUM_SEX]
  if( word == "男" )
    ret = 0
  elsif( word == "女" )
    ret = 1
  else
    ret = 2
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #４ビット境界用ダミーデータ 3 (14-16)
  ret = 0
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  wfile.write( ary.pack("C*") )
  wfile.write( ary.pack("C*") )
  
	return RET_TRUE
end

#=======================================================================
#	モデルリスト作成
#=======================================================================
xlstxt_filename = ARGV[0]

if( xlstxt_filename == nil )
	printf( "ERROR fmmdl_mdllist xlstxt filename\n" )
	exit 1
end

residx_filename = ARGV[1]

if( xlstxt_filename == nil )
	printf( "ERROR fmmdl_mdllist residx filename\n" )
	exit 1
end

bin_filename = ARGV[2]

if( bin_filename == nil )
	printf( "ERROR fmmdl_mdllist bin filename\n" )
	exit 1
end

code_filename = ARGV[3]

if( code_filename == nil )
	printf( "ERROR fmmdl_mdllist code filename\n" )
	exit 1
end

codestr_filename = ARGV[4]

if( codestr_filename == nil )
	printf( "ERROR fmmdl_mdllist code str filename\n" )
	exit 1
end

symbol_filename = ARGV[5]

if( symbol_filename == nil )
	printf( "ERROR fmmdl_mdllist symbol filename\n" )
	exit 1
end

sel_res = ARGV[6]
if( sel_res != "0" && sel_res != "1" )
  printf( "ERROR!! fmmdl_mdllist.rb sel_res\n" )
  exit 1
end

txtfile = File.open( xlstxt_filename, "r" );
residxfile = File.open( residx_filename, "r" );
binfile = File.open( bin_filename, "wb" );
codefile = File.open( code_filename, "w" );
codestrfile = File.open( codestr_filename, "wb" );
symfile = File.open( symbol_filename, "r" );

ret = codefile_write( codefile, codestrfile, txtfile ) #表示コードヘッダー作成

if( ret == RET_FALSE )
	error_end( bin_filename, code_filename, codestr_filename,
		 txtfile, residxfile, binfile, codefile, codestrfile, symfile )
	exit 1
end

no = 0
ary = Array( no )
binfile.write( ary.pack("I*") )

ret = RET_FALSE
line = txtfile.gets #先頭行抜かし

while line = txtfile.gets			#パラメタコンバート
	if( line.include?(STR_END) )
		break
	end
	
	ret = convert_line( no, line, binfile, residxfile, symfile, sel_res )
	
	if( ret == RET_FALSE )
		error_end( bin_filename, code_filename, codestr_filename,
			 txtfile, residxfile, binfile, codefile, codestrfile, symfile )
		exit 1
	end

	no = no + 1
end

binfile.pos = 0
ary = Array( no )
binfile.write( ary.pack("I*") ) #総数

txtfile.close
residxfile.close
binfile.close
codefile.close
codestrfile.close
symfile.close
