###################################################################################
# ARCフォルダ内のnaixで全ソースコードをGREPし、引っかからなかったものを列挙
# 作成日：2010/6/4 作成者：保坂 元八
###################################################################################

ARC_PATH = "c:/home/pokemon_wb/prog/arc"
SRC_PATH = "c:/home/pokemon_wb/prog/src"
SCRIPT_PATH = "c:/home/pokemon_wb/resource/fldmapdata/script"

# 再帰的にCファイルを検出
def recursive_search(path, word)
  Dir::foreach(path) do |v|
    next if v == "." or v == ".." or v == ".svn"
    if path =~ /\/$/
      v = path + v
    else
      v = path + "/" + v
    end

    ext = File::extname(v)
    if ext == ".c" || ext == ".h" || ext == ".cdat" || ext == ".ev"
      f = open(v)
      tbl = f.read
      if tbl.include?( word ) == true
        $hitflag = true;
        return;
      end
      f.close
    end

    #ディレクトリの場合は再帰
    if FileTest::directory?(v)
      recursive_search(v,word) #再帰呼び出し リカーシブ　コール/Recursive Call
    end
  end
end

#======================================================

list = Array.new

# naixファイル名をリストアップ
Dir::foreach(ARC_PATH) {|filename|
  if File::extname(filename) == ".naix"
    list.push( filename )
  end
}

#puts list

puts "全要素数=" + list.size.to_s


$hitflag = false

puts "使われていない↓"

list.each{|i|
  $hitflag = false
  recursive_search( SRC_PATH, i )
  recursive_search( SCRIPT_PATH, i )
  #なかったら出力
  if $hitflag == false
    puts i
  end
}

=begin
=end
