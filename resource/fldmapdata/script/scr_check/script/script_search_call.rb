require "script_search.rb"

#EVファイル列挙
all_ary = Dir.glob("../../*.ev")
ary = Array.new
all_ary.each{|i|
  i.sub!(/..\/..\//,"")
  ary << i
}

# 検索条件ファイル解析
data = SearchData.new(ARGV[0])

put_file = File.open( "../result/script_search/"+data.put_file, "w" )

ary.each{|i|
  # ARGV[0]=検索条件ファイル 、i=検索対象のevファイル名
  #system("ruby script_search.rb #{ARGV[0]} #{i}")
  src = "../ev_tmp/"+i
  search = ScriptSearch.new( src , data ,put_file )
  search.run
}

put_file.close
