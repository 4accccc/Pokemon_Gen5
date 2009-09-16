#============================================================================
#
# 依存定義ファイルを削除してmakeをかけるためのツール
#
# @author tamada GAMEFREAK inc.
# @date 2009.09.16  renew
#
#
#============================================================================
require "fileutils"

PROGDIR = ENV["PROJECT_PROGDIR"].sub(/C:/,"/cygdrive/c").chomp

#指定ファイル名を含む依存ファイルをリストアップする
def search_depend_files( search_name )
  build_type = `cd #{PROGDIR}; make echo_buildtype`.chomp
  #puts "result:#{build_type}"
  depend_path = "#{PROGDIR}/depend/#{build_type}/*.d"
  command = "grep #{search_name} #{depend_path}"
  #puts "command:#{command}"
  dependfiles = `#{command}`
  #puts "\"#{dependfiles}\""
  return dependfiles
end

#依存ファイル名リストから依存ファイルとオブジェクトファイルを削除する
def remove_depfile_and_objfile( dependfiles )
  dependfiles.each{|line|
    file = line.sub(/:.*$/,"").chomp
    puts "rm #{file}"
    FileUtils.rm( file )
    FileUtils.rm( file.sub(/\/depend\//,"\/obj\/").sub(/d$/,"o") )
  }
end

#ここから実行本体
raise Exception, "ヘッダファイル名を指定してください" if ARGV.size == 0 

search_name = ARGV.shift
dependfiles = search_depend_files( search_name )
raise Exception, "依存しているファイルが見つかりません" if dependfiles == ""
remove_depfile_and_objfile( dependfiles )

