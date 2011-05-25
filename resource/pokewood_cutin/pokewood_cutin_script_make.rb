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
#=============================================================================
#
#   メイン処理
#
#=============================================================================
CONVNAME  = ARGV[0]
NAIXNAME  = ARGV[1]
SCRNAME   = ARGV[2]
begin

  #カレントディレクトリの中にあるファイルを検索
  list = Dir::entries(Dir::pwd)

  list.each{ |file|
    r = Regexp.new(SCRNAME+'.\.csv') 
    if r =~ file
      target = file.sub( "\.csv", "" )
      system( "ruby #{CONVNAME} #{file} #{NAIXNAME} #{target}" ) 
    end
  }

end
