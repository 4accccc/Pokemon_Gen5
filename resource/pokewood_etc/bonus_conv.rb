#!ruby -Ks
$KCODE = "SJIS"

##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
## @file		\home\swan\resource\pokewood_bonus\bonus_conv.rb
## @brief   ポケウッドご褒美コンバータ
## @author	Katsuhiko=Ichiraku
## @date		2011.09.21
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require 'csv'

# パス要変更
HASHPATH = "../../../swan/tools/hash/"

# アイテムハッシュ
require HASHPATH + "item_hash.rb"	# 道具

# 各種定義

# ランク
$rank_hash      = { "S"=>0, "A"=>1, "B"=>2, "C"=>3 }


#-------------
# ハッシュから値取得
#-------------
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

#ここから main

csv_file = open(ARGV.shift,"r")

dat_file_name = ARGV.shift
dat_file = open(dat_file_name,"w")

dat_file.printf("//======================================================================\n")
dat_file.printf("/**\n")
dat_file.printf(" * @file	%s\n",dat_file_name)
dat_file.printf(" * @brief	ポケウッドご褒美データファイル\n")
dat_file.printf(" * @note	このファイルはコンバーターから出力されます\n")
dat_file.printf(" *\n")
dat_file.printf(" */\n")
dat_file.printf("//======================================================================\n")
dat_file.printf("\n")

#4行飛ばし
csv_file.gets
csv_file.gets
csv_file.gets


# マクロ定義
line = csv_file.gets
column = line.split ","
dat_file.printf("#define %s (%s)\t//%s\n", column[0], column[1], column[2])

csv_file.gets

#==========================================
#ファンの種類・道具をくれるタイプ
#==========================================
dat_file.printf("#define\tNOT_GIVE\t(0xff)\n")

dat_file.printf("// ファンの種類・道具を上げるタイプ\n")
dat_file.printf("static const u16 pkwd_bonus_fan[] = {\n")
while line = csv_file.gets
  #FAN_END検出
  if line =~/^#FAN_END/ then
		break
	end
  
  column = line.split ","

  # あげるタイプ
  if column[3] =~/^-/ then
    column[3] = "NOT_GIVE"
  end

  dat_file.printf("\t%s,  // %s\n", column[3], column[1].chomp("\r").chomp("\n"))

end
dat_file.printf("};\n")
dat_file.printf("\n")

#==========================================
#ご褒美のランク
#==========================================
#1行飛ばし
csv_file.gets

#マクロ定義
line = csv_file.gets
column = line.split ","
dat_file.printf("#define %s (%s)\t//%s\n", column[0], column[1], column[2])
line = csv_file.gets
column = line.split ","
dat_file.printf("#define %s (%s)\t//%s\n", column[0], column[1], column[2])

#2行飛ばし
csv_file.gets
csv_file.gets

dat_file.printf("// ご褒美のランク\n")
dat_file.printf("static const u16 pkwd_bonus_rank[4][3] = {\n")
dat_file.printf("\t//BAD_END GOOD_END STRANGE_END \n")

while line = csv_file.gets
  #FAN_END検出
  if line =~/^#RANK_END/ then
		break
	end
  
  column = line.split ","

  dat_file.printf("\t{%d, ", hash2no($rank_hash, column[1]) )
  dat_file.printf("%d, ", hash2no($rank_hash, column[3]) )
  dat_file.printf("%d },", hash2no($rank_hash, column[5]) )
  dat_file.printf("\t// %s\n", column[0].chomp("\r").chomp("\n") )


end

dat_file.printf("};\n")
dat_file.printf("\n")

#==========================================
#ご褒美リスト
#==========================================
#3行飛ばし
csv_file.gets
csv_file.gets
csv_file.gets

dat_file.printf("// ご褒美アイテムリスト\n")
dat_file.printf("static const u16 pkwd_bonus_item[5][4] = {\n")
dat_file.printf("\t//A B C D\n")

item_num = Array::new

while line = csv_file.gets
  #FAN_END検出
  if line =~/^#END/ then
		break
	end
  
  column = line.split ","

  dat_file.printf("\t{%d, ", hash2no($item_hash, column[1]) )
  dat_file.printf("%d, ", hash2no($item_hash, column[3]) )
  dat_file.printf("%d, ", hash2no($item_hash, column[5]) )
  dat_file.printf("%d },", hash2no($item_hash, column[7]) )
  dat_file.printf("\t// あげるタイプ %d\n", column[0].chomp("\r").chomp("\n") )

  # アイテムの個数を保存
  array = Array[ column[2], column[4], column[6], column[8] ]
  item_num << array

end

dat_file.printf("};\n")
dat_file.printf("\n")

dat_file.printf("// ご褒美アイテムの個数リスト\n")
dat_file.printf("static const u16 pkwd_bonus_item_num[5][4] = {\n")
dat_file.printf("\t//A B C D\n")

i=0
item_num.each{ | num1, num2, num3, num4 |
  dat_file.printf("\t{ %d, %d, %d, %d },", num1, num2, num3, num4 )
  dat_file.printf("\t// あげるタイプ %d\n", i)
  i += 1
}

dat_file.printf("};\n")
dat_file.printf("\n")

