#=====================================================
#
# バトルサブウェイ用トレーナー名配列の生成
# 2010.03.29  tamada
# 
# tools/btl_subway/trainer.txtのトレーナー名をそのままの
# 順序にした配列を生成する
#
#
# バトルタワーメーカーの出力において、トレーナー種別は
# tools/btl_subwa/trainer.txt内のトレーナー名へのインデックス数値が
# 出力されている。
# これをtools/hash/trtype_hash.rbを引いて出力できるように
# 変換する必要があり、暫定で作成した。
#
#
#=====================================================
puts "\$bts_trainername_array = [\n"

count = 0
File.open(ARGV[0]){|file|
  file.each{|line|
    printf("\"%s\", # %d\n", line.chomp, count )
    count += 1
  }
}

puts "]\n"
