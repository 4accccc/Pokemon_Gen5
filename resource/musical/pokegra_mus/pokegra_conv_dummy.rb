#==========================================================
#	ミュージカル用ポケモン画像変換
#	このRubyが各Rubyとかexeとかを呼び、色々コンバートしてくれます。
#===========================================================

require 'fileutils'
load 'pokegra_conv_dummy.lst'

#変換perlのディレクトリ
PERL_TOOL_DIR = ENV['PROJECT_ROOT'] + "tools/pokegra/"
#変換元データのディレクトリ
POKEGURA_DATA_DIR = ENV['PROJECT_ROOT'] + "../pokemon_wb_doc/pokegra_mus/"
#変換後データのディレクトリ
OUTPUT_DIR = "./data/"
TEMP_DIR = "./temp/"

#アーカイバー
NARC_CONVERT = "nnsarc -r -l -n -i"
NARC_CONVERT_OPT = "-S pokegra_wb_dummy.scr"
NARC_NAME = "pokegra_mus_dummy.narc"

#リストファイル
NARC_LIST = "pokegra_wb_dummy.scr"
DEBUG_STR_FILE = "pokegura_mus_path.cdat"

$isRefresh = false

#ファイル更新日比較
#file1の方が新しければ1を返す
def CompFileDate( file1 , file2 )
  retVal = 0
  
  unless( FileTest.exist?(file2) )
    retVal = 1
  else
    state1 = File::stat(file1)
    state2 = File::stat(file2)
    
    if( state1.mtime > state2.mtime )
      retVal = 1
    end
  end

  retVal
end

#---------------------------------------------------------
#1個の変換(親フォルダと子フォルダ
#---------------------------------------------------------
def convGraFileFunc( mainFolder , subFolder )
  searchName = POKEGURA_DATA_DIR + mainFolder + subFolder + "/*.*"

  print(subFolder + " " + searchName + "\n")

  Dir::glob(searchName).each{|fileName|
    #print("[" + fileName + "]\n")
    fileType = File::extname(fileName)
    case
    #ncl
    when fileType == '.ncl'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCLR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "ncl.pl " + fileName + " " + OUTPUT_DIR )
        system( "cp " + TEMP_DIR + "*.NCLR" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        $isRefresh = TRUE
      end

    #ncg
    when fileType == '.ncg'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCGR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "comp/ncg.pl " + fileName + " " + OUTPUT_DIR )
        #system( "cp " + TEMP_DIR + "*.NCGR" + " " + OUTPUT_DIR )
        #system( "rm " + TEMP_DIR + "*.*" )
        $isRefresh = TRUE
      end
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCBR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "comp/ncgc.pl " + fileName + " " + OUTPUT_DIR )
        #system( "cp " + TEMP_DIR + "*.NCBR" + " " + OUTPUT_DIR )
        #system( "rm " + TEMP_DIR + "*.*" )
        $isRefresh = TRUE
      end

    #nce
    when fileType == '.nce'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCEC'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "nce_mus.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NCEC" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        $isRefresh = TRUE
      end

    #nce
    when fileType == '.nmc'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NMCR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "comp/nmc.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NMCR" + " " + OUTPUT_DIR )
        system( "cp " + TEMP_DIR + "*.NCER" + " " + OUTPUT_DIR )
        system( "cp " + TEMP_DIR + "*.NANR" + " " + OUTPUT_DIR )
        system( "cp " + TEMP_DIR + "*.NMAR" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        $isRefresh = TRUE
      end
    end
    system( "pause" )
  }
end

#---------------------------------------------------------
#1個の変換(定義名より
#---------------------------------------------------------
def convGraFile( str )

  noStr = str[0..2]
  dataNo = noStr.to_i
  countryName = ""
  
  if( dataNo <= 151 )
    countryName = "1_kanto/"
  elsif( dataNo <= 251 )
    countryName = "2_jyoto/"
  elsif( dataNo <= 386 )
    countryName = "3_houen/"
  elsif( dataNo <= 493 )
    countryName = "4_shinou/"
  else
    countryName = "5_issyu/"
  end

  convGraFileFunc( countryName , noStr )
end

#---------------------------------------------------------
#通常変換
#---------------------------------------------------------
def convAll()
  data_idx = 0

  while POKEGURA_CONV_LIST[data_idx] != "end"
    
    convGraFile( POKEGURA_CONV_LIST[data_idx] )
    data_idx = data_idx+1

    #isRefresh = TRUE
  end #while

  convGraFileFunc( "" , "egg" )
  convGraFileFunc( "" , "migawari" )

  print($isRefresh.to_s + "\n")

  
  if( $isRefresh == TRUE || FileTest.exist?(NARC_NAME) == FALSE )
    
    #リストの作成
    data_idx = 0
    outFile = File.open( NARC_LIST, "w" );

    #通常リスト
    while POKEGURA_CONV_LIST[data_idx] != "end"
      len = POKEGURA_CONV_LIST[data_idx].length
      noStr = POKEGURA_CONV_LIST[data_idx][0..2]
      formStr = ""
      if( len > 3 )
        formStr = POKEGURA_CONV_LIST[data_idx][3..(len-1)]
      end
      #print(noStr + " " + formStr + "\n")

      outFile.write( "\"data/pfwb_" + noStr + formStr + "_m.NCGR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + "_f.NCGR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCER\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NANR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMCR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMAR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCEC\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + "_m.NCGR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + "_f.NCGR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCER\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NANR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMCR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMAR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCEC\"\n" )
      outFile.write( "\"data/pmwb_" + noStr + formStr + "_n.NCLR\"\n" )
      outFile.write( "\"data/pmwb_" + noStr + formStr + "_r.NCLR\"\n" )
      data_idx = data_idx+1
    end #while

    #タマゴ追加
    noStr = "egg"
    formStr = "_normal"
    outFile.write( "\"data/pfwb_" + noStr + formStr + "_m.NCGR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + "_f.NCGR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + "_m.NCGR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + "_f.NCGR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pmwb_" + noStr + formStr + "_n.NCLR\"\n" )
    outFile.write( "\"data/pmwb_" + noStr + formStr + "_r.NCLR\"\n" )
    formStr = "_manafi"
    outFile.write( "\"data/pfwb_" + noStr + formStr + "_m.NCGR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + "_f.NCGR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + "_m.NCGR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + "_f.NCGR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pmwb_" + noStr + formStr + "_n.NCLR\"\n" )
    outFile.write( "\"data/pmwb_" + noStr + formStr + "_r.NCLR\"\n" )

    #アナザーフォルム
    data_idx = 0
    while POKEGURA_CONV_LIST_ANOTHER[data_idx] != "end"
      len = POKEGURA_CONV_LIST_ANOTHER[data_idx].length
      noStr = POKEGURA_CONV_LIST_ANOTHER[data_idx][0..2]
      formStr = ""
      if( len > 3 )
        formStr = POKEGURA_CONV_LIST_ANOTHER[data_idx][3..(len-1)]
      end
      #print(noStr + " " + formStr + "\n")

      outFile.write( "\"data/pfwb_" + noStr + formStr + "_m.NCGR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + "_f.NCGR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCER\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NANR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMCR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMAR\"\n" )
      outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCEC\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + "_m.NCGR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + "_f.NCGR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_m.NCBR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + "c" + formStr + "_f.NCBR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCER\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NANR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMCR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMAR\"\n" )
      outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCEC\"\n" )
      outFile.write( "\"data/pmwb_" + noStr + formStr + "_n.NCLR\"\n" )
      outFile.write( "\"data/pmwb_" + noStr + formStr + "_r.NCLR\"\n" )
      data_idx = data_idx+1
    end #while
    #身代わり追加
    noStr = "migawari"
    formStr = ""
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCGR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pfwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCGR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCER\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NANR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMCR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NMAR\"\n" )
    outFile.write( "\"data/pbwb_" + noStr + formStr + ".NCEC\"\n" )
    outFile.write( "\"data/pmwb_" + noStr + formStr + ".NCLR\"\n" )

    #アナザーフォルム(パレットのみ)
    data_idx = 0
    while POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx] != "end"
      len = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx].length
      noStr = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx][0..2]
      formStr = ""
      if( len > 3 )
        formStr = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx][3..(len-1)]
      end
      #print(noStr + " " + formStr + "\n")

      outFile.write( "\"data/pmwb_" + noStr + formStr + "_n.NCLR\"\n" )
      outFile.write( "\"data/pmwb_" + noStr + formStr + "_r.NCLR\"\n" )
      data_idx = data_idx+1
    end #while

    outFile.close
    
    
    system( NARC_CONVERT + " " + NARC_NAME + " " + NARC_CONVERT_OPT )
  end
end
#---------------------------------------------------------
#デバッグ用スクリプト出力
#---------------------------------------------------------
def convDebStrFuncOne( file , path )
  file.write( "  \"" + path + "\",\n")
end

def convDebStrFunc( file , main , sub )
    convDebStrFuncOne( file , "pfwb_" + main + sub + "_m.NCGR" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + "_f.NCGR" )
    convDebStrFuncOne( file , "pfwb_" + main + "c" + sub + "_m.NCBR" )
    convDebStrFuncOne( file , "pfwb_" + main + "c" + sub + "_f.NCBR" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + ".NCER" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + ".NANR" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + ".NMCR" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + ".NMAR" )
    convDebStrFuncOne( file , "pfwb_" + main + sub + ".NCEC" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + "_m.NCGR" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + "_f.NCGR" )
    convDebStrFuncOne( file , "pbwb_" + main + "c" + sub + "_m.NCBR" )
    convDebStrFuncOne( file , "pbwb_" + main + "c" + sub + "_f.NCBR" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + ".NCER" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + ".NANR" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + ".NMCR" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + ".NMAR" )
    convDebStrFuncOne( file , "pbwb_" + main + sub + ".NCEC" )
    convDebStrFuncOne( file , "pmwb_" + main + sub + "_n.NCLR" )
    convDebStrFuncOne( file , "pmwb_" + main + sub + "_r.NCLR" )
end

def convDebStr()
  outFile = File.open( DEBUG_STR_FILE, "w" );
  
  print( "--------------\n" )
  print( File.dirname(File.expand_path(__FILE__)).to_s + "\n" )
  print( "--------------\n" )
  
  base_dir = File.dirname(File.expand_path(__FILE__)).sub(/^\/cygdrive\/c\//, "c:\/")

  outFile.write( "//このファイルはpokegra_conv.rbから生成されています。\n" )
  outFile.write( "static const char pogeguraMusPathName[] = \"" + base_dir + "/data/\";\n")
  outFile.write( "\n")
  outFile.write( "\n")
  outFile.write( "static const char *pogeguraMusFileName[] = \n{\n")
  #子ファイル名列挙
  data_idx = 0
  while POKEGURA_CONV_LIST[data_idx] != "end"
    len = POKEGURA_CONV_LIST[data_idx].length
    noStr = POKEGURA_CONV_LIST[data_idx][0..2]
    formStr = ""
    if( len > 3 )
      formStr = POKEGURA_CONV_LIST[data_idx][3..(len-1)]
    end
    convDebStrFunc( outFile , noStr , formStr )
    data_idx = data_idx+1
  end #while
  #タマゴ追加
  convDebStrFunc( outFile , "egg" , "_normal" )
  convDebStrFunc( outFile , "egg" , "_manafi" )
  #アナザーフォルム
  data_idx = 0
  while POKEGURA_CONV_LIST_ANOTHER[data_idx] != "end"
    len = POKEGURA_CONV_LIST_ANOTHER[data_idx].length
    noStr = POKEGURA_CONV_LIST_ANOTHER[data_idx][0..2]
    formStr = ""
    if( len > 3 )
      formStr = POKEGURA_CONV_LIST_ANOTHER[data_idx][3..(len-1)]
    end
    #print(noStr + " " + formStr + "\n")
    convDebStrFunc( outFile , noStr , formStr )
    data_idx = data_idx+1
  end #while
  #身代わり
  main = "migawari"
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NCGR" )
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NCER" )
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NANR" )
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NMCR" )
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NMAR" )
  convDebStrFuncOne( outFile , "pfwb_" + main + ".NCEC" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NCGR" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NCER" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NANR" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NMCR" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NMAR" )
  convDebStrFuncOne( outFile , "pbwb_" + main + ".NCEC" )
  convDebStrFuncOne( outFile , "pmwb_" + main + ".NCLR" )

  #アナザーフォルム(パレットのみ)
  data_idx = 0
  while POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx] != "end"
    len = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx].length
    noStr = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx][0..2]
    formStr = ""
    if( len > 3 )
      formStr = POKEGURA_CONV_LIST_ANOTHER_PLTT[data_idx][3..(len-1)]
    end
    #print(noStr + " " + formStr + "\n")
    convDebStrFuncOne( outFile , "pmwb_" + noStr + formStr + "_n.NCLR" )
    convDebStrFuncOne( outFile , "pmwb_" + noStr + formStr + "_r.NCLR" )
    data_idx = data_idx+1
  end #while    

  outFile.write( "};\n")
  outFile.close

end
#---------------------------------------------------------
#メイン
#---------------------------------------------------------
unless FileTest.exist?(OUTPUT_DIR)
  FileUtils.mkdir_p(OUTPUT_DIR)
end
unless FileTest.exist?(TEMP_DIR)
  FileUtils.mkdir_p(TEMP_DIR)
end

  $isRefresh = FALSE
  
if( ARGV[0].to_s == "-s" )
  print("ミュージカル用ポケモングラフィック変換(単発[" + ARGV[1].to_s + "])\n")
  
  if( ARGV[1].to_s == "" )
    print("番号がありません\n")
    exit
  end
  
  if( ARGV[1].to_i > 657   )
    print("番号が大きいです\n")
    exit
  end

  if( ARGV[1].to_i == 0   )
    print("番号が不正です\n")
    exit
  end
  
  dataName = sprintf("%03d" , ARGV[1].to_s)
  
  convGraFile( dataName )
  
  if( $isRefresh == TRUE )
    print("変換完了\n")
  else
    print("更新がありませんでした。\n")
  end
  
elsif( ARGV[0].to_s == "-d" )
  convDebStr()
else
  print("ミュージカル用ポケモングラフィック変換\n")
  convAll()
end
