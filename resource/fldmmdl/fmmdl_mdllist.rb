#=======================================================================
#  fmmdl_mdllist.rb
#  フィールド動作モデル　モデルリスト作成
#  引数 fmmdl_mdllist.rb xlstxt residx file_binname file_codename file_codestrname
#  xlstxt 動作モデルリスト表 テキスト変換済みファイル名
#  residx リソースアーカイブデータインデックスファイル名
#  file_binname 作成するバイナリファイル名
#  file_codename 作成するOBJコードファイル名
#  file_codestrname 作成するOBJコード文字列ファイル名
#  symbolfilename 読み込むシンボルファイル名
#=======================================================================
$KCODE = "SJIS"
load "rbdefine"

#=======================================================================
#  パラメタフォーマット
#  0-3 モデル総数
#  4-  以下総数分のモデルパラメータ
#
#  モデルパラメタフォーマット
#  0-1 OBJコード
#  2  表示タイプ
#  3  処理関数
#  4  影表示
#  5  足跡種類
#  6  映り込み
#  7  モデルサイズ
#  8  テクスチャサイズ
#  9  アニメID
#  10 性別
#  11 4バイト境界余り
#  12 横幅サイズ
#  13 奥行サイズ
#  14-27 表示タイプ別データバッファ
#=======================================================================

#=======================================================================
#  作成されるOBJコード文字列ファイル構造
#  32byte単位でコードの数分格納されている。
#  0..31  コード0番のコード文字列 32byte ASCII 終端ヌル文字
#  32..63 コード1番文字列
#=======================================================================

#=======================================================================
#  定数
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

STR_CODESTART_BBD = "OBJCODESTART_BBD" #コード開始 ビルボード
STR_CODEEND_BBD = "OBJCODEEND_BBD" #コード終了 ビルボード
STR_CODECOUNT_BBD = "OBJCODECOUNT_BBD" #コード数　ビルボード

STR_CODESTART_POKE = "OBJCODESTART_TPOKE" #コード開始 ポケモン
STR_CODEEND_POKE = "OBJCODEEND_TPOKE" #コード終了 ポケモン
STR_CODECOUNT_POKE = "OBJCODECOUNT_TPOKE" #コード数　ポケモン

STR_CODESTART_MDL = "OBJCODESTART_MDL" #コード開始 モデル
STR_CODEEND_MDL = "OBJCODEEND_MDL" #コード開始 モデル
STR_CODECOUNT_MDL = "OBJCODECOUNT_MDL" #コード数 モデル

STR_CODETOTAL = "OBJCODETOTAL" #コード総数
STR_CODEMAX = "OBJCODEMAX" #コード最大

#コード開始位置
CODE_STARTNUM_BBD = 0x0000 #ビルボード
CODE_STARTNUM_POKE = 0x1000 #ポケモン
CODE_STARTNUM_MDL = 0x2000 #モデル
CODE_MAX = 0xffff #最大コード

#表示コード文字列 一文字列最長 ヌル文字含む
CODESTRBUF = (16)

#引数
ARGVNO_FNAME_LISTCSV = 0 #リストcsvファイルネーム
ARGVNO_FNAME_RESIDX = 1 #リソースアーカイブインデックスファイルネーム
ARGVNO_FNAME_BIN = 2 #バイナリファイルネーム
ARGVNO_FNAME_CODE = 3 #コードファイルネーム
ARGVNO_FNAME_CODESTR = 4 #コード文字列ファイルネーム
ARGVNO_FNAME_SYMNBOL = 5 #シンボルファイルネーム
ARGVNO_FLAG_SELRES = 6 #リソース選択
ARGVNO_FNAME_LISTCSV_POKE = 7 #リストcsvファイルネーム　ポケモン
ARGVNO_FNAME_LISTCSV_MDL = 8 #リストcsvファイルネーム　モデル

#=======================================================================
#  関数
#=======================================================================
#-----------------------------------------------------------------------
#  異常終了
#-----------------------------------------------------------------------
def error_end(
  path_delfile0, path_delfile1, path_delfile2,
  file0, file1, file2, file3, file4, file5, file6, file7 )
  printf( "ERROR fldmmdl list convert\n" )
  file0.close
  file1.close
  file2.close
  file3.close
  file4.close
  file5.close
  file6.close
  file7.close
  File.delete( path_delfile0 )
  File.delete( path_delfile1 )
  File.delete( path_delfile2 )
end

#-----------------------------------------------------------------------
#  ファイルを指定の値で埋める
#-----------------------------------------------------------------------
def file_write_filldata( file, size, data )
  i = 0
  ary = Array( data )
  
  while i < size
    file.write( ary.pack("C*") )
    i = i + 1
  end
end

#-----------------------------------------------------------------------
#  ニトロG3Dファイル　コンバート後のアーカイブインデックス名を返す
#-----------------------------------------------------------------------
def nitro_anime_convert_name( name )
  extname = File.extname( name )
  
  if( extname == "\.itp" )
    name = name.gsub( "\.itp", "_nsbtp" )
  elsif( extname == "\.ica" )
    name = name.gsub( "\.ica", "_nsbca" )
  elsif( extname == "\.ima" )
    name = name.gsub( "\.ima", "_nsbma" )
  elsif( extname == "\.ita" )
    name = name.gsub( "\.ita", "_nsbta" )
  elsif( extname == "\.iva" )
    name = name.gsub( "\.iva", "_nsbva" )
  end

  name = sprintf( "_%s", name ) #例 test_nsbtx -> _test_nsbtx
  return name
end

#-----------------------------------------------------------------------
#  モデルリスト用ヘッダーファイル内検索
#  search 検索文字列
#  戻り値 指定文字列の数値 RET_ERROR=ヒット無し
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
      
      if( str[1] == search )  #1 シンボル名
        str_num = str[2]  #2 数値
        
        if( str_num =~ /\A0x/ ) #16進表記
          /([\A0x0-9a-fA-F]+)/ =~ str_num
          str_num = $1
          num = str_num.hex
        else          #10進表記
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
#  モデルリスト用アーカイブインデックスファイル内検索
#  search 検索文字列
#  戻り値 指定文字列の数値 RET_ERROR=ヒット無し
#-----------------------------------------------------------------------
def arcidx_search( idxfile, search )
  pos = idxfile.pos
  idxfile.pos = 0
  num = RET_ERROR
  
  search = search.strip #先頭末尾の空白、改行削除
  
  while line = idxfile.gets
    if( line =~ /^enum.*\{/ )
      while line = idxfile.gets
        if( line.index(search) != nil )
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
#  コードファイル作成
#=======================================================================
#-----------------------------------------------------------------------
#  コードファイル記述　メイン
#  戻り値　RET_FALSE=異常終了
#-----------------------------------------------------------------------
def file_code_write_main(
  file_code, file_codestr, file_listcsv, code_no, count, code_num, start_no )
  start_code = code_no
  
  pos = file_listcsv.pos
  file_listcsv.pos = 0 #先頭に
  line = file_listcsv.gets #一行飛ばし
  
  loop{
    line = file_listcsv.gets
    
    if( line == nil )
      return nil
    end
    
    if( line.include?(STR_END) )
      break
    end
    
    str = line.split( "," )
    
    file_code.printf( "#define %s (0x%x) //%d(%d,total %d) %s\n",
      str[code_num], code_no, code_no, code_no-start_no, count, str[RBDEF_NUM_CODENAME]  )
    
    codestr = Array.new( CODESTRBUF );  #文字列バッファ
    codestr.fill( "\000".unpack('C*'), 0..CODESTRBUF ) #ヌル文字で埋め尽くし
    
    i = 0
    strbuf = str[code_num]
    while strbuf[i]
      codestr[i] = strbuf[i]
      i = i + 1
      if( i >= (CODESTRBUF-1) )
        break
      end
    end
    
    i = 0
    while i < CODESTRBUF    #バイト単位でバッファ長分書き込み
      c = Array( codestr[i] )
      file_codestr.write( c.pack("C*") )
      i = i + 1
    end
    
    count = count + 1
    code_no = code_no + 1
  }
  
  if( code_no == start_code )
    printf( "OBJコードが定義されていません\n" );
    return nil
  end
  
  file_listcsv.pos = pos
  
  param = Hash.new
  param.store( "str_code_no", code_no )
  param.store( "str_count", count )
  return param
end  

#-----------------------------------------------------------------------
#  コードファイル記述
#  戻り値　RET_FALSE=異常終了
#-----------------------------------------------------------------------
def file_code_write( file_code, file_codestr,
      file_listcsv, file_listcsv_poke, file_listcsv_mdl )
  pos = file_listcsv.pos
  file_listcsv.pos = 0 #先頭に
  line = file_listcsv.gets #一行飛ばし
  
  file_code.printf( "//動作モデル OBJコード定義\n" )
  file_code.printf( "//このファイルはコンバータから作成されました\n" )
  
  path = file_code.path
  name = File.basename( path, "\.*" )
  name = name.upcase
  name = sprintf( "_%s_", name )
  file_code.printf( "#ifndef %s\n#define %s\n\n", name, name )
  
  count = 0

  #ビルボードコード記述
  code_no = CODE_STARTNUM_BBD
  
  file_code.printf( "//ビルボード\n" )
  file_code.printf( "#define %s (0x%x)\n", STR_CODESTART_BBD, code_no )
  
  ret = file_code_write_main(
    file_code, file_codestr, file_listcsv, code_no, count,
    RBDEF_NUM_BBD_CODE, CODE_STARTNUM_BBD )
  
  if( ret == nil )
    return RET_FALSE
  end
  
  code_no = ret['str_code_no']
  count = ret['str_count']
  
  file_code.printf(
    "#define %s (0x%x) //%d(total %d) %s\n",
    STR_CODEEND_BBD, code_no, code_no, count, "ビルボードコード終端" );

  file_code.printf(
    "#define %s (0x%x) //%d %s\n\n", STR_CODECOUNT_BBD,
    code_no-CODE_STARTNUM_BBD, code_no-CODE_STARTNUM_BBD,
    "ビルボードコード数" );

  #ポケモンコード記述
  code_no = CODE_STARTNUM_POKE
  
  file_code.printf( "//連れ歩きポケモン\n" )
  file_code.printf( "#define %s (0x%x)\n", STR_CODESTART_POKE, code_no )
  
  ret = file_code_write_main(
    file_code, file_codestr, file_listcsv_poke, code_no, count,
    RBDEF_NUM_BBD_CODE, CODE_STARTNUM_POKE )
  
  if( ret == nil )
    return RET_FALSE
  end
  
  code_no = ret['str_code_no']
  count = ret['str_count']
  
  file_code.printf(
    "#define %s (0x%x) //%d(total %d) %s\n",
    STR_CODEEND_POKE, code_no, code_no, count, "ポケモンコード終端" );
  
  file_code.printf(
    "#define %s (0x%x) //%d %s\n\n", STR_CODECOUNT_POKE,
    code_no-CODE_STARTNUM_POKE, code_no-CODE_STARTNUM_POKE,
    "ポケモンコード数" );

  #モデルコード記述
  code_no = CODE_STARTNUM_MDL
  
  file_code.printf( "//モデル\n" )
  file_code.printf( "#define %s (0x%x)\n", STR_CODESTART_MDL, code_no )
  
  ret = file_code_write_main(
    file_code, file_codestr, file_listcsv_mdl, code_no, count,
    RBDEF_NUM_MDL_CODE, CODE_STARTNUM_MDL )
  
  if( ret == nil )
    return RET_FALSE
  end
  
  code_no = ret['str_code_no']
  count = ret['str_count']
  
  file_code.printf(
    "#define %s (0x%x) //%d(total %d) %s\n",
    STR_CODEEND_MDL, code_no, code_no, count, "モデルコード終端" );
  
  file_code.printf(
    "#define %s (0x%x) //%d %s\n\n", STR_CODECOUNT_MDL,
    code_no-CODE_STARTNUM_MDL, code_no-CODE_STARTNUM_MDL,
    "モデルコード数" );
  
  #終端記述
  file_code.printf(
    "#define %s (0x%x) //%d %s\n", STR_CODETOTAL, count, count, "コード総数" )
  file_code.printf(
    "#define %s (0x%x) //%d %s\n\n", STR_CODEMAX, CODE_MAX, CODE_MAX, "無効コード" )
  
  file_code.printf( "#endif //_%s_", name );
  return RET_TRUE
end

#=======================================================================
#  動作モデルパラメタコンバート
#=======================================================================
#-----------------------------------------------------------------------
#  動作モデルデータ一行コンバート　ビルボード
#-----------------------------------------------------------------------
def convert_line_bbd( no, line, wfile, idxfile, file_symbol, flag_selres )
  str = line.split( "," )
  
  #OBJコード 2
  ary = Array( no )
  wfile.write( ary.pack("S*") )
  
  #表示タイプ 1 (3)
  word = str[RBDEF_NUM_DRAWTYPE]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 表示タイプ異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #処理関数 1 (4)
  word = str[RBDEF_NUM_BBD_DRAWPROC]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 処理関数異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #影表示 1 (5)
  word = str[RBDEF_NUM_BBD_SHADOW]
  if( word != "○" )
    ret = 0 
  else
    ret = 1
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #足跡種類 1 (6)
  word = str[RBDEF_NUM_BBD_FOOTMARK]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 足跡種類異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #映り込み 1 (7)
  word = str[RBDEF_NUM_BBD_REFLECT]
  if( word != "○" )
    ret = 0
  else
    ret = 1
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #モデルサイズ 1 (8)
  word = str[RBDEF_NUM_BBD_MDLSIZE]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR モデルサイズ異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #テクスチャサイズ 1 (9)
  word = str[RBDEF_NUM_BBD_TEXSIZE]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR テクスチャサイズ異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #アニメID 1 (10)
  word = str[RBDEF_NUM_BBD_ANMID]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR アニメID異常 _%s_\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #性別 1 (11)
  word = str[RBDEF_NUM_BBD_SEX]
  if( word == "男" )
    ret = 0
  elsif( word == "女" )
    ret = 1
  else
    ret = 2
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #４ビット境界用ダミーデータ 1 (12)
  ret = 0
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #横幅サイズ 1 (13)
  ret = 0
  word = str[RBDEF_NUM_BBD_WIDTH_SIZE]
  if( word != nil )
    ret = word.to_i
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )

  #奥行サイズ 1 (14)
  ret = 0
  word = str[RBDEF_NUM_BBD_DEPTH_SIZE]
  if( word != nil )
    ret = word.to_i
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #リソースインデックス 2 (15)
  word = str[RBDEF_NUM_DRAWTYPE]
  
  if( word == RBSTR_DRAWTYPE_NON ) #表示タイプ　無し
    ret = 0
  else
    xlsline = RBDEF_NUM_BBD_RESFILE_NAME_0
    
    if( flag_selres != "0" )
      xlsline = RBDEF_NUM_BBD_RESFILE_NAME_1
    end
    
    word = str[xlsline]
    
    #正規表現で文字列".imd"を削除する。が、何故か'm'文字まで削除してしまう
    #/(\A.*[^\.imd])/ =~ word
    #mdlname = $1
    #
    #代わりとしてgsubで".imd"を削除する。
    #mdlname = word.gsub( "\.imd", "" )
    
    #imd -> nsbtx
    mdlname = word.gsub( "\.imd", "_nsbtx" )
    mdlname = sprintf( "_%s", mdlname ) #例 test_nsbtx -> _test_nsbtx
    
    ret = arcidx_search( idxfile, mdlname )
    if( ret == RET_ERROR )
      printf( "ERROR モデルファイル名異常 %s\n", word )
      return RET_FALSE
    end
  end
  ary = Array( ret )
  wfile.write( ary.pack("S*") )
  
  #４ビット境界用ダミーデータ 12 (28)
  file_write_filldata( wfile, 12, 0 )
  
  return RET_TRUE
end

#-----------------------------------------------------------------------
#  動作モデルデータ一行コンバート　モデル
#-----------------------------------------------------------------------
def convert_line_mdl( no, line, wfile, idxfile, file_symbol, flag_selres )
  str = line.split( "," )
  
  #OBJコード 2
  ary = Array( no )
  wfile.write( ary.pack("S*") )
  
  #表示タイプ 1 (3)
  word = str[RBDEF_NUM_DRAWTYPE]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 表示タイプ異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #処理関数 1 (4)
  word = str[RBDEF_NUM_MDL_DRAWPROC]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 処理関数異常異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #影表示 1 (5)
  word = str[RBDEF_NUM_MDL_SHADOW]
  if( word != "○" )
    ret = 0 
  else
    ret = 1
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #足跡種類 1 (6)
  word = str[RBDEF_NUM_MDL_FOOTMARK]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR 足跡種類異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #映り込み 1 (7)
  ret = 1 #非表示
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #モデルサイズ 1 (8)
  word = str[RBDEF_NUM_MDL_MDLSIZE]
  ret = hfile_search( file_symbol, word )
  if( ret == RET_ERROR )
    printf( "ERROR モデルサイズ異常 %s\n", word )
    return RET_FALSE
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #テクスチャサイズ 1 (9)
  ret = 0 #ダミー
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #アニメID 1 (10)
  ret = 0 #ダミー
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #性別 1 (11)
  word = str[RBDEF_NUM_MDL_SEX]
  if( word == "男" )
    ret = 0
  elsif( word == "女" )
    ret = 1
  else
    ret = 2
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #４ビット境界用ダミーデータ 1 (12)
  ret = 0
  ary = Array( ret )
  wfile.write( ary.pack("C*") )

  #横幅サイズ 1 (13)
  ret = 0
  word = str[RBDEF_NUM_MDL_WIDTH_SIZE]
  if( word != nil )
    ret = word.to_i
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )

  #奥行サイズ 1 (14)
  ret = 0
  word = str[RBDEF_NUM_MDL_DEPTH_SIZE]
  if( word != nil )
    ret = word.to_i
  end
  ary = Array( ret )
  wfile.write( ary.pack("C*") )
  
  #リソースインデックス 各2 (22)
  word = str[RBDEF_NUM_DRAWTYPE]
  
  if( word == RBSTR_DRAWTYPE_NON ) #表示タイプ　無し
    file_write_filldata( wfile, 12, 0 )
  else
    #imd
    word = str[RBDEF_NUM_MDL_RESFILE_NAME_IMD]
    name = word.gsub( "\.imd", "_nsbmd" )
    name = sprintf( "_%s", name )
    
    ret = arcidx_search( idxfile, name )
    if( ret == RET_ERROR )
      printf( "ERROR モデルファイル名異常 %s\n", name )
      return RET_FALSE
    end
    
    ary = Array( ret )
    wfile.write( ary.pack("S*") )
    
    #tex
    word = str[RBDEF_NUM_MDL_RESFILE_NAME_TEX]
    
    if( word == "" )
      file_write_filldata( wfile, 2, 0xff )
    else
      name = word.gsub( "\.imd", "_nsbtx" )
      name = sprintf( "_%s", name )
      ret = arcidx_search( idxfile, name )
      
      if( ret == RET_ERROR )
        printf( "ERROR ファイル名異常 %s\n", name )
        return RET_FALSE
      end
    
      ary = Array( ret )
      wfile.write( ary.pack("S*") )
    end

    #anime 0
    word = str[RBDEF_NUM_MDL_RESFILE_NAME_ANM0]
    
    if( word == "" )
      file_write_filldata( wfile, 2, 0xff )
    else
      name = nitro_anime_convert_name( word )
      ret = arcidx_search( idxfile, name )
      
      if( ret == RET_ERROR )
        printf( "ERROR ファイル名異常 %s\n", name )
        return RET_FALSE
      end
    
      ary = Array( ret )
      wfile.write( ary.pack("S*") )
    end
    
    #anime 1
    word = str[RBDEF_NUM_MDL_RESFILE_NAME_ANM1]
    
    if( word == "" )
      file_write_filldata( wfile, 2, 0xff )
    else
      name = nitro_anime_convert_name( word )
      ret = arcidx_search( idxfile, name )
      
      if( ret == RET_ERROR )
        printf( "ERROR ファイル名異常 %s\n", name )
        return RET_FALSE
      end
    
      ary = Array( ret )
      wfile.write( ary.pack("S*") )
    end

    #anime 2
    word = str[RBDEF_NUM_MDL_RESFILE_NAME_ANM2]
    
    if( word == "" )
      file_write_filldata( wfile, 2, 0xff )
    else
      name = nitro_anime_convert_name( word )
      ret = arcidx_search( idxfile, name )
      
      if( ret == RET_ERROR )
        printf( "ERROR ファイル名異常 %s\n", name )
        return RET_FALSE
      end
    
      ary = Array( ret )
      wfile.write( ary.pack("S*") )
    end
  end
  
  #４ビット境界用ダミーデータ 4 (28)
  file_write_filldata( wfile, 4, 0 )
  
  return RET_TRUE
end

#-----------------------------------------------------------------------
#  動作モデルデータ一行コンバート
#-----------------------------------------------------------------------
def convert_line( no, line, wfile, idxfile, file_symbol, flag_selres )
  ret = RET_FALSE

  str = line.split( "," )
  
  if( str[RBDEF_NUM_DRAWTYPE] == RBSTR_DRAWTYPE_NON ||
      str[RBDEF_NUM_DRAWTYPE] == RBSTR_DRAWTYPE_BBD )
    ret = convert_line_bbd(
      no, line, wfile, idxfile, file_symbol, flag_selres )
  elsif( str[RBDEF_NUM_DRAWTYPE] == RBSTR_DRAWTYPE_MDL )
    ret = convert_line_mdl(
      no, line, wfile, idxfile, file_symbol, flag_selres )
  end
   
  return ret
end

#-----------------------------------------------------------------------
#  動作モデル　データコンバート　メイン
#-----------------------------------------------------------------------
def convert_code_param_main( start_code,
  file_bin, file_listcsv, file_residx, file_symbol, flag_selres )
  count = 0
  no = start_code
  
  file_listcsv.pos = 0 #先頭行に
  line = file_listcsv.gets #先頭行抜かし
  
  while line = file_listcsv.gets
    if( line.include?(STR_END) )
      break
    end
    
    ret = convert_line(
      no, line, file_bin, file_residx, file_symbol, flag_selres )
    
    if( ret == RET_FALSE )
      return RET_ERROR #ERROR
    end
    
    no = no + 1
    count = count + 1
  end
  
  return count
end

#-----------------------------------------------------------------------
#  動作モデル　データコンバート　メイン
#-----------------------------------------------------------------------
def convert_code_param( file_bin,
    file_listcsv, file_listcsv_poke, file_listcsv_mdl,
    file_residx, file_symbol, flag_selres )
  count = 0 #ダミー総数記述
  ary = Array( count )
  file_bin.pos = 0
  file_bin.write( ary.pack("I*") )
  
  #ビルボード
  ret = convert_code_param_main( CODE_STARTNUM_BBD,
    file_bin, file_listcsv, file_residx, file_symbol, flag_selres )
  
  if( ret == RET_ERROR )
    return RET_FALSE
  end
  
  count = count + ret
  
  #ポケモン
  ret = convert_code_param_main( CODE_STARTNUM_POKE,
    file_bin, file_listcsv_poke, file_residx, file_symbol, flag_selres )
  
  if( ret == RET_ERROR )
    return RET_FALSE
  end
  
  count = count + ret
  
  #モデル
  ret = convert_code_param_main( CODE_STARTNUM_MDL,
    file_bin, file_listcsv_mdl, file_residx, file_symbol, flag_selres )
  
  if( ret == RET_ERROR )
    return RET_FALSE
  end
  
  count = count + ret
  
  file_bin.pos = 0 #総数記述
  ary = Array( count )
  file_bin.write( ary.pack("I*") )
  return RET_TRUE
end

#=======================================================================
#  モデルリスト作成
#=======================================================================
#---------------------------------------------------------------
# 引数取得
#---------------------------------------------------------------
fname_listcsv = ARGV[ARGVNO_FNAME_LISTCSV]

if( fname_listcsv == nil )
  printf( "ERROR fmmdl_mdllist xlstxt filename\n" )
  exit 1
end

fname_listcsv_poke = ARGV[ARGVNO_FNAME_LISTCSV_POKE]

if( fname_listcsv_poke == nil )
  printf( "ERROR fmmdl_mdllist xlstxt poke filename\n" )
  exit 1
end

fname_listcsv_mdl = ARGV[ARGVNO_FNAME_LISTCSV_MDL]

if( fname_listcsv_mdl == nil )
  printf( "ERROR fmmdl_mdllist xlstxt poke filename\n" )
  exit 1
end

fname_residx = ARGV[ARGVNO_FNAME_RESIDX]

if( fname_residx == nil )
  printf( "ERROR fmmdl_mdllist residx filename\n" )
  exit 1
end

fname_bin = ARGV[ARGVNO_FNAME_BIN]

if( fname_bin == nil )
  printf( "ERROR fmmdl_mdllist bin filename\n" )
  exit 1
end

fname_code = ARGV[ARGVNO_FNAME_CODE]

if( fname_code == nil )
  printf( "ERROR fmmdl_mdllist code filename\n" )
  exit 1
end

fname_codestr = ARGV[ARGVNO_FNAME_CODESTR]

if( fname_codestr == nil )
  printf( "ERROR fmmdl_mdllist code str filename\n" )
  exit 1
end

fname_symbol = ARGV[ARGVNO_FNAME_SYMNBOL]

if( fname_symbol == nil )
  printf( "ERROR fmmdl_mdllist symbol filename\n" )
  exit 1
end

flag_selres = ARGV[ARGVNO_FLAG_SELRES]

if( flag_selres != "0" && flag_selres != "1" )
  printf( "ERROR!! fmmdl_mdllist.rb flag_selres\n" )
  exit 1
end

#---------------------------------------------------------------
# ファイルオープン
#---------------------------------------------------------------
file_listcsv = File.open( fname_listcsv, "r" );
file_listcsv_poke = File.open( fname_listcsv_poke, "r" );
file_listcsv_mdl = File.open( fname_listcsv_mdl, "r" );
file_residx = File.open( fname_residx, "r" );
file_bin = File.open( fname_bin, "wb" );
file_code = File.open( fname_code, "w" );
file_codestr = File.open( fname_codestr, "wb" );
file_symbol = File.open( fname_symbol, "r" );

#---------------------------------------------------------------
# 表示コードヘッダーファイル作成
#---------------------------------------------------------------
ret = file_code_write( file_code, file_codestr,
  file_listcsv, file_listcsv_poke, file_listcsv_mdl )

if( ret == RET_FALSE )
  error_end( fname_bin, fname_code, fname_codestr,
    file_listcsv, file_listcsv_poke, file_listcsv_mdl,
    file_residx, file_bin, file_code, file_codestr, file_symbol )
  exit 1
end

#---------------------------------------------------------------
# コードパラメタファイル作成
#---------------------------------------------------------------
ret = convert_code_param( file_bin,
  file_listcsv, file_listcsv_poke, file_listcsv_mdl,
  file_residx, file_symbol, flag_selres )

if( ret == RET_FALSE )
  error_end( fname_bin, fname_code, fname_codestr,
    file_listcsv, file_listcsv_poke, file_listcsv_mdl,
    file_residx, file_bin, file_code, file_codestr, file_symbol )
  exit 1
end

#---------------------------------------------------------------
# ファイルクローズ
#---------------------------------------------------------------
file_listcsv.close
file_listcsv_poke.close
file_listcsv_mdl.close
file_residx.close
file_bin.close
file_code.close
file_codestr.close
file_symbol.close
