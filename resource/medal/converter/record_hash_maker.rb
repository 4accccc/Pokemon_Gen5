##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		record_hash_maker.rb
#*	@brief  レコードIDのハッシュを作成する
#*	@author		Masayuki Onoue
#*	@data		2011.01.07
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require File.dirname(__FILE__) + '/rbdef.rb'

record_h_name = ARGV[0]
out_hash_name = ARGV[1]
# ========================
# レコードデータの読み込み
record_h = open( record_h_name , "r")

def_name_list = Array.new # 定義順番保存変数
def_hash = Hash.new
record_h.each{ |line|
  if line.include?("#define") then

    # 括弧に囲まれたものをのデータを取得
    /#define\s+([^\s]+)\s+\((.*)\)/ =~ line
    hash_name = $1
    hash_data = $2

    if (hash_name != nil) && (hash_data != nil)
      str = hash_data.gsub(" ","") # スペース削除
      hash_name.strip! # 前後の空白文字削除
      hash_data.strip! # 前後空白文字削除
      def_hash[hash_name] = str
      def_name_list << hash_name
    else
      # 括弧に囲まれていないもののデータを取得
      /#define\s+([^\s]+)\s+(.*)/ =~ line
      hash_name = $1
      hash_data = $2
      if hash_data.size != 0 then
        hash_name.strip!
        hash_data.strip!
        def_hash[hash_name] = hash_data
        def_name_list << hash_name
      else
        # 値が定義されていないものはデータに含まない(インクルードガードなど)
      end
    end
  end
}

# ===========================
# データを数値に変換する
def_name_list.each{ |key|
  split_data = def_hash[key].split("+")

  # 分割したデータの定義部分は数値に置き換える
  data = 0
  split_data.each{|sp_data|
    if sp_data =~ /\d+/
      data += sp_data.to_i
    else
      # 文字列はハッシュから検索して数値に置き換える
      if def_hash.key?(sp_data) then
        data += def_hash[ sp_data ].to_i
      else
        puts key+"->"+sp_data+"a"
        raise "define定義を数値に書き換えることが出来ません->"+key
      end
    end
  }

  # 数値データをハッシュに登録する
  def_hash[key] = data.to_s
}

# =========================
# データの書き込み
out_hash = open( out_hash_name , "w")

out_hash.puts RECRBDEF_CMT % out_hash_name

def_name_list.each{ |key|
  out_hash.puts "\t" + '"' + key + '" => ' + def_hash[ key ] + ","
}
out_hash.puts "}"

out_hash.close

