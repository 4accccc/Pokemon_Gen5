#============================================================================
#
# 依存定義ファイルを削除してmakeをかけるためのツール
#
# @author tamada GAMEFREAK inc.
# @date 2009.09.16  renew
#
# @note
#   prog/makefileの"echo_buildtype"ターゲットに依存している。
#   "echo_buildtype"はNITRO_BUILDTYPEを取得するためだけに追加した。
#
#============================================================================
require "fileutils"

PROGDIR = ENV["PROJECT_PROGDIR"].sub(/C:/,"/cygdrive/c").chomp

def DebugPut arg
  puts "DEBUG:" + arg
end

#指定ファイル名を含む依存ファイルをリストアップする
def search_depend_files( search_name )
  build_type = `cd #{PROGDIR}; make -f makefile_main echo_buildtype`.chomp
  DebugPut "result:#{build_type}"
  depend_path = "#{PROGDIR}depend/#{build_type}/*.d"
  command = "grep #{search_name} #{depend_path}"
  DebugPut "command:#{command}"
  dependfiles = `#{command}`
  DebugPut "\"#{dependfiles}\""
  return dependfiles
end

#依存ファイル名リストから依存ファイルとオブジェクトファイルを削除する
def remove_depfile_and_objfile( dependfiles )
  removed = Hash.new
  dependfiles.each{|line|
    file = line.sub(/:.*$/,"").chomp
    if removed.has_key?(file) then next end
    removed[file] = true
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

