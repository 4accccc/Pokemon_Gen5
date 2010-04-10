#==========================================================
#	ミュージカル用ポケモン画像変換
#	このRubyが各Rubyとかexeとかを呼び、色々コンバートしてくれます。
#===========================================================

require 'fileutils'
load 'pokegra_conv.lst'

#変換perlのディレクトリ
PERL_TOOL_DIR = ENV['PROJECT_ROOT'] + "tools/pokegra/"
#変換元データのディレクトリ
POKEGURA_DATA_DIR = ENV['PROJECT_ROOT'] + "../pokemon_wb_doc/pokegra_mus/"
#変換後データのディレクトリ
OUTPUT_DIR = "./data/"
TEMP_DIR = "./temp/"

#アーカイバー
NARC_CONVERT = "nnsarc -r -l -n -i"
NARC_CONVERT_OPT = "-S pokegra_wb.scr"
NARC_NAME = "pokegra_mus.narc"

#リストファイル
NARC_LIST = "pokegra_wb.scr"

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

#"program files"等の空白が入るファイル名を使えるように変換する
#http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-list/32740
MAX_PATH_SIZE = 513
def get_short_path_name lfn
  old_name = File.basename(lfn)
  require 'Win32API'
  buff = 0.chr * MAX_PATH_SIZE
  len = Win32API.new( 'kernel32' , 'GetShortPathName' , %w(P P N) , "N" ).
        Call( lfn , buff, buff.size)
  return nil if len==0
  ret = buff.split(0.chr)[0].tr('\\','/')[0,len]
  name = File.basename(ret)
  dir  = ret[0,ret.size - name.size]
  if name.size > old_name.size
    name = name[0,old_name.size]
  end
  return dir+name
end
alias sfn get_short_path_name


def convGraFile( mainFolder , subFolder )
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
        system( "perl " + PERL_TOOL_DIR + "ncl.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NCLR" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        isRefresh = TRUE
      end

    #ncg
    when fileType == '.ncg'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCGR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "comp/ncg.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NCGR" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        isRefresh = TRUE
      end
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCBR'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "comp/ncgc.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NCBR" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        isRefresh = TRUE
      end

    #nce
    when fileType == '.nce'
      convFile = OUTPUT_DIR + File::basename( fileName , '.*' ) + '.NCEC'
      if( CompFileDate( fileName , convFile ) == 1 )
        system( "perl " + PERL_TOOL_DIR + "nce_mus.pl " + fileName + " " + TEMP_DIR )
        system( "cp " + TEMP_DIR + "*.NCEC" + " " + OUTPUT_DIR )
        system( "rm " + TEMP_DIR + "*.*" )
        isRefresh = TRUE
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
        isRefresh = TRUE
      end

    end
  }
end
#---------------------------------------------------------
#メイン
#---------------------------------------------------------

print("ミュージカル用ポケモングラフィック変換\n")
unless FileTest.exist?(OUTPUT_DIR)
  FileUtils.mkdir_p(OUTPUT_DIR)
end
unless FileTest.exist?(TEMP_DIR)
  FileUtils.mkdir_p(TEMP_DIR)
end

data_idx = 0

isRefresh = FALSE

while POKEGURA_CONV_LIST[data_idx] != "end"
  noStr = POKEGURA_CONV_LIST[data_idx][0..2]
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
  
  convGraFile( countryName , noStr )

  data_idx = data_idx+1

  #isRefresh = TRUE
end #while

convGraFile( "" , "egg" )
convGraFile( "" , "migawari" )


if( isRefresh == TRUE || FileTest.exist?(NARC_NAME) == FALSE )
  
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


