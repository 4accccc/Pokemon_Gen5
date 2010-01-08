#==========================================================
#	ミュージカル・演目変換Ruby
#	このRubyが各Rubyとかexeとかを呼び、色々コンバートしてくれます。
#===========================================================
require 'fileutils'
require 'program_conv'

#出力個数
PROGRAM_NUM = 20
#PROGRAM_NUM = 1
#演目データエクセル
PRAGRAM_DATA_EXCEL = "program_data.xls"
#演目データ出力名
PRAGRAM_BIN_NAME = "program_data.bin"
#演目アーカイブ名
NARC_NAME_BASE = "mus_prog_data"

#ストリーミングコンバータ
STREAMING_CONVERT = "waveconv -c "
#gmmコンバータ
MESSAGE_CONVERT = "perl -I ../../message ../../message/msgconv.pl "
MESSAGE_CONVERT_LANG = "JPN JPN_KANJI"
#スクリプトコンバータ
SCRIPT_CONVERT = "ruby ../eescmk.rb " + ENV['PROJECT_ROOT'] + "/prog/src/musical/stage/script/musical_script_command.h"
SCRIPT_CONVERT_OPT = ENV['PROJECT_ROOT'] + "/ on"
#スクリプトバイナリ用
CW_BIN_TOOLS = ENV['CWFOLDER'] + "/ARM_Tools/Command_Line_Tools/"
SCRIPT_MWAS = CW_BIN_TOOLS + "mwasmarm.exe"
SCRIPT_MWLD = CW_BIN_TOOLS + "mwldarm.exe"
SCRIPT_ELF2BIN = ENV['PROJECT_ROOT'] + "/tools/elf2bin.exe"

#アーカイバー
NARC_CONVERT = "nnsarc -c -l -n "

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


index = 1
progBinName = ""
strmDataName = ""
gmmDataName = ""
scriptDataName = ""

while index <= PROGRAM_NUM

  isRefreshFile = 0
  pathName = sprintf("p%02d",index)

  #フォルダが無い場合フォルダを作る
  unless FileTest.exist?(pathName)
    FileUtils.mkdir_p(pathName) 
  end

  progBinName = pathName + "/" + PRAGRAM_BIN_NAME
  
  #演目データ変換
  if( CompFileDate( PRAGRAM_DATA_EXCEL , progBinName ) == 1 )
    ProgramExcelConv.conv(index,PRAGRAM_DATA_EXCEL,progBinName)
    isRefreshFile = 1
  end
  
  #ストリーミング変換
  cnt = 0
  searchName = pathName + "/*.wav"
  Dir::glob(searchName).each{|fileName|
    cnt = cnt + 1
    strmDataName = fileName.sub(/.wav/,".swav")
    if( CompFileDate( fileName , strmDataName ) == 1 )
      system(STREAMING_CONVERT + fileName )
      isRefreshFile = 1
    end
  }

  if( cnt != 1 )
    print("ストリーミングデータのコンバートでエラーが発生しました。\n")
    print("wavが無いか、wavが2個以上あります。\n")
    print("ディレクトリ[" + pathName + "]\n")
    exit
  end

  #gmm変換
  cnt = 0
  searchName = pathName + "/*.gmm"
  Dir::glob(searchName).each{|fileName|
    cnt = cnt + 1
    gmmDataName = fileName.sub(/.gmm/,".dat")
    if( CompFileDate( fileName , gmmDataName ) == 1 )
      system(MESSAGE_CONVERT + fileName + " " + pathName + "/ " + MESSAGE_CONVERT_LANG )
      isRefreshFile = 1
    end
  }

  if( cnt != 1 )
    print("メッセージデータのコンバートでエラーが発生しました。\n")
    print("gmmが無いか、gmmが2個以上あります。\n")
    print("ディレクトリ[" + pathName + "]\n")
    exit
  end

  #スクリプト変換
  cnt = 0
  searchName = pathName + "/*.esf"
  Dir::glob(searchName).each{|fileName|
    cnt = cnt + 1
    scriptAssemblerName = fileName.sub(/.esf/,".s")
    scriptObjectName = fileName.sub(/.esf/,".o")
    scriptElfName = fileName.sub(/.esf/,".elf")
    scriptDataName = fileName.sub(/.esf/,".bin")
    if( CompFileDate( fileName , scriptDataName ) == 1 )
      print( "conv[" + scriptDataName + "]\n" )
      #エディタ→アセンブラソース
      #system(SCRIPT_CONVERT + " " + fileName + " " + scriptAssemblerName + " " + SCRIPT_CONVERT_OPT )
      print(SCRIPT_CONVERT + " " + fileName + " " + SCRIPT_CONVERT_OPT + "\n" )
      system(SCRIPT_CONVERT + " " + fileName + " " + SCRIPT_CONVERT_OPT )
      system("cp *.s " + scriptAssemblerName )
      system(sfn(SCRIPT_MWAS) + " " + scriptAssemblerName + " -o " + scriptObjectName )
      system(sfn(SCRIPT_MWLD) + " -dis -o " + scriptElfName + " " + scriptObjectName )
      system(SCRIPT_ELF2BIN + " " + scriptElfName )
      isRefreshFile = 1
    end
  }
  
  if( cnt != 1 )
    print("スクリプトデータのコンバートでエラーが発生しました。\n")
    print("esfが無いか、esfが2個以上あります。\n")
    print("ディレクトリ[" + pathName + "]\n")
    exit
  end

  #narc作成
  if( isRefreshFile == 1 )
    system("pause")
    narcFileName = pathName + "/" + NARC_NAME_BASE
    system(NARC_CONVERT + narcFileName + ".narc " + progBinName + " " + gmmDataName + " " + scriptDataName + " " + strmDataName )
    #narcサイズチェック
    state = File::stat( narcFileName + ".narc")
    print( "アーカイブを作成しました。\n" )
    print( "Name[" + narcFileName + "]\n" )
    print( "Size[" + state.size.to_s + "byte(" + ((state.size/1024).to_i).to_s + "kb)]\n" )
    if( state.size > 512*1024 )
      print( "------------------------\n" )
      print( "サイズオーバーです！！！\n" )
      print( "------------------------\n" )
    end

  end

  index = index+1
end



