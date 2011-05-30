#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		pokewood_cutin_script_make.rb
#	@brief  ポケウッドカットインスクリプトのmake部分
#	@author	Toru=Nagihashi
#	@data		2011.05.24
#
#	依存関係的にmakefileで書きづらいのでrubyでmakeしています
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../resonance_resort/data/conv_tool.rb" #モジュール使い回しのためリゾートのを使っています。
#=============================================================================
#
#   メイン処理
#
#=============================================================================
CONVNAME  = ARGV[0]
NAIXNAME  = ARGV[1]
SCRNAME   = ARGV[2]
CDATNAME  = ARGV[3]
SADLNAME  = ARGV[4]

H_ROW_SENARIO     = 1 
H_ROW_EVENT       = 2 
H_COL_VALUE       = 1
begin

  #カレントディレクトリの中にあるファイルを検索
  list = Dir::entries(Dir::pwd)

  #各csvのシナリオとイベント番号を書きこむためのcdat
  cdat    = TextBuilder.new
  cdat.write( "static const struct{\n")
  cdat.write("\tu16 senario;\n")
  cdat.write("\tu16 event;\n")
  cdat.write("} sc_pw_cutin_dat_search[] ={\n" )

  list.each{ |file|
    r = Regexp.new(SCRNAME+'.\.csv') 
    if r =~ file

      #datをコンバート
      target = file.sub( "\.csv", "" )
      system( "ruby #{CONVNAME} #{file} #{NAIXNAME} #{SADLNAME} #{target}" ) 

      #cdat作成のために
      csv = CsvLoader.new
      csv.setPrefix( '#header_start' )
      csv.setSuffix( '#header_end' )
      csv.load( file )

      senario = csv.get( H_ROW_SENARIO, H_COL_VALUE )
      event   = csv.get( H_ROW_EVENT,   H_COL_VALUE )

      event   = event.to_i - 1

      cdat.write( "\t{\n" )
      cdat.write( "\t\t#{senario.to_i},#{event.to_i}\n" )
      cdat.write( "\t},\n" )
      
    end
  }

  cdat.write( "};\n" )
  cdat.genelate( "#{CDATNAME}.cdat" )

end
