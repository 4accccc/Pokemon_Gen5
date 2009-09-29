#==========================================================
#	ミュージカル・演目変換Ruby
#	このRubyが各Rubyを呼び、色々コンバートしてくれます。
#===========================================================
require 'fileutils'
require 'program_conv'

#出力個数
PROGRAM_NUM = 5
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


index = 1
progBinName = ""
strmDataName = ""
gmmDataName = ""

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

  #narc作成
  if( isRefreshFile == 1 )
    narcFileName = pathName + "/" + NARC_NAME_BASE
    system(NARC_CONVERT + narcFileName + " " + progBinName + " " + gmmDataName + " " + strmDataName )

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



