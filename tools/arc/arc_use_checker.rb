###################################################################################
# naixファイルで定義されているIDから全ソースコード中で定義されていないものを抽出
# ・naix内IDが150行以内のファイルに限る
#
# 作成日：2010/6/4 作成者：保坂 元八
###################################################################################

ARC_PATH = "c:/home/pokemon_wb/prog/arc"
SRC_PATH = "c:/home/pokemon_wb/prog/src"
LINE_MAX = 150

#構造体定義
SData = Struct.new('Data', :filename, :tblname)

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
    if ext == ".c" || ext == ".h" || ext == ".cdat"
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

# 全てのnaixに対して処理
Dir::foreach(ARC_PATH) {|filename|
  if File::extname(filename) == ".naix"

    lineno = 0
    path =  ARC_PATH + "/" + filename;

    #行数を調べる
    File.open( path, "r" ){|f|
      while line = f.gets
        if f.eof?
          lineno = f.lineno
        end
      end
    }
    
    #特定行以上ならスキップ
    next if lineno >= LINE_MAX
#    puts lineno

    #PUSH
    File.open( path, "r" ){|f|
      f.each{|i|
        if i.include?(" = ") == true
          i = i.strip
          i = i.sub(/ = \d*,/, "")
          i = i.sub(/ = \d*/, "")
          list.push( SData.new(filename,i) );
        end
      }
    }
  end
}

#list.push( SData.new( "test.naix" , "NARC_guru2_2d_cook_s_obj0_NCLR" ) )
#list.push( SData.new( "test.naix" , "NARC_guru2_2d_record_s_NCLR" ) )
#list.push( SData.new( "test.naix" , "NARC_guru2_2d_record_s2_NCLR" ) )

#puts list

puts "全要素数=" + list.size.to_s

$hitflag = false

puts "使われていない↓"

list.each{|i|
  $hitflag = false
  recursive_search( SRC_PATH, i.tblname )
  #なかった出力
  if $hitflag == false
    #ファイル名も出力
    puts i.filename + " >> " + i.tblname
  end
}


=begin
=end
