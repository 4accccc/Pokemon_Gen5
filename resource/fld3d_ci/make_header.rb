$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")

h_file_name = ARGV.shift
h_file = open(h_file_name,"w")

h_file.printf("//======================================================================\n")
h_file.printf("/**\n")
h_file.printf(" * @file	%s\n",h_file_name)
h_file.printf(" * @brief	数値定義ファイル\n")
h_file.printf(" * @note	このファイルはコンバーターから出力されます\n")
h_file.printf(" * @note	スクリプトが参照します\n")
h_file.printf(" *\n")
h_file.printf(" */\n")
h_file.printf("//======================================================================\n")
h_file.printf("\n")

h_file.printf("#pragma once\n")
h_file.printf("\n")

#1行飛ばし
csv_file.gets
index = 0
while line = csv_file.gets
  #END検出
  if line =~/^#END/ then
		break
	end
  
  column = line.split ","

  h_file.printf("#define %s (%d)\n",column[0].upcase, index)

  index += 1
end

