###################################################################################
#
# 作成日：2010/7/2 作成者：保坂 元八
###################################################################################
#! ruby -Ks

require 'kconv'

#文字コード指定
$KCODE = 'SJIS'

SRC_MESSAAGE = "../../resource/message/src"

# ひらがなの中に漢字がないか 一項目をチェック
def check_elem( str )
  #タグを外す
  check = str.gsub(/\[.*\]/, '')
  #漢字判定
  if /[亜-煕]/ =~ check
    #結果出力
    res = str.gsub(/<lang.*">/, '')
    res = res.gsub(/<\/lan.*>/, '')
    puts res
  end
end   

# gmm内のひらがなの中に漢字がないかチェック
def check_kanji( gmm_file )
  puts "====================================================================="
  puts gmm_file + " > "
  File.open( gmm_file, "r" ){|f|
    str = f.read
    str = Kconv.tosjis(str)
    
    check = ""
    b_head = true
    str.each_line{|l|
    
      if b_head == true
        if l.include?("\"JPN\"")
          check = l
          b_head = false
        end
      end
      
      if b_head == false
        # 初回以外行追加
        if check != l
          check += l
        end
        
        if l.include?("/language")
          # チェック
          check_elem( check )
          # 終了処理
          check = ""
          b_head = true
        end
      end
     
    }
  }
end

# 再帰的にGMMを開く
def recursive_search(path)
  Dir::foreach(path) do |v|
    next if v == "." or v == ".." or v == ".svn"
    if path =~ /\/$/
      v = path + v
    else
      v = path + "/" + v
    end

    ext = File::extname(v)
    if ext == ".gmm"
      check_kanji( v )
    end

    #ディレクトリの場合は再帰
    if FileTest::directory?(v)
      recursive_search(v) #再帰呼び出し リカーシブ　コール/Recursive Call
    end
  end
end


#check_kanji( "bag.gmm" )

recursive_search( SRC_MESSAAGE ) 
