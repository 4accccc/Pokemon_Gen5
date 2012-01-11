#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		todo_search.rb
#*	@brief  仮検索
#*	@author	Masayuki Onoue
#*	@date		2011.12.06
#*
#*/
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

require 'csv'

# 記入データ
class WRITE_DATA
  attr_accessor :filename, :revision ,:str ,:user_name ,:line_index
  def initialize( filename, revision ,user_name ,line_index ,str  )
    @filename   = filename
    @revision   = revision
    @str        = str
    @user_name  = user_name
    @line_index = line_index
  end
end

# 指定ラインをはじめに書いた人を取得する
def get_first_write_data( filename ,line_index ,line )
  # 改行文字は切り取っておく
  chomp_search_line = line.chomp
  # サブバージョンのログコマンドを発行
  command = "svn blame " + filename
  puts command
  # 記入データリスト
  write_data_array = Array.new
  res = `#{ command }`
  res.each{|blame_line|
    if blame_line =~ /\s+(\w+)\s+(\w+)\s(.*$)/ then
      # 改行文字を切り取った文字列をと比較する
      chomp_sauce_line  = $3.chomp
      if chomp_sauce_line == chomp_search_line then
        # 記入データをリスト化する
        write_data = WRITE_DATA.new( filename, $1.to_i ,$2 ,line_index.to_i , chomp_search_line )
        write_data_array << write_data
        #puts "%sさんがリビジョン%sで%sと書きました"%[$2,$1,chomp_search_line]
      end
    end
  }
  
  # 記入者チェック
  if write_data_array.size <= 0 then
    write_data = WRITE_DATA.new( filename, 0 ,"バージョン管理外ファイル" ,line_index.to_i , line.chomp )
    write_data_array << write_data
  end

  # 記入の古い順にソート
  write_data_array.sort{|a,b| a.revision <=> b.revision }

=begin
  # デバッグ出力
  write_data_array.each{ |data|
    puts "%5s %10s %s"%[data.revision ,data.user_name ,data.str]
  }
  data = write_data_array[0]
  puts "結果=>%5s %10s %s"%[data.revision ,data.user_name ,data.str]
=end

  # 一番古い記入者を返す
  return write_data_array[0]

end

# ================================
# メイン
# ================================

# 検索する拡張子配列
extension_list = ARGV[1]

# 検索トップディレクトリ
top_directory = ARGV[0] + '/**/*.'+extension_list

# 正規表現文字列
pattern_str = ARGV[2]

# 出力ファイル名
out_filename = ARGV[3]

# globワード
glob_word = top_directory

# TODOリストデータ
todo_list = Array.new
 
  
puts "todo検索開始"

# 検索対象ファイルを全て調べる
Dir.glob( glob_word ).each{|filename|
  #puts filename
  # 全行検索
  File.open( filename , "r" ){|f|
    f.each_with_index{|line,line_index|
      #if line =~ /TODO|todo|仮[^想]|後で|あとで|とりあえず|取りあえず|取り合えず|取り敢えず|取敢えず|とり合えず/ then
      if line =~ /#{pattern_str}/ then
        todo_list << get_first_write_data( filename ,line_index.to_i+1 ,line  )
      end
    }
  }

# 一件だけチェック
=begin
  if todo_list.size > 0 then
    break
  end
=end

}

puts "結果を書き込んでいます。"

CSV.open(out_filename, 'w') do |writer|
  writer << [ "編集ユーザー名", "ファイル名" ,"リビジョン", "行番号", "文字列"]
  # 検索結果出力
  todo_list.each{|data|
    writer << [ data.user_name, data.filename, data.revision, data.line_index, data.str ]
  }
end

puts "終了しました"

