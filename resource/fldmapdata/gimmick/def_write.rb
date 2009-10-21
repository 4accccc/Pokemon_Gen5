$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")

cdat_file_name = ARGV.shift
cdat_file = open(cdat_file_name,"w")


cdat_file.printf("//======================================================================\n")
cdat_file.printf("/**\n")
cdat_file.printf(" * @file	%s\n",cdat_file_name)
cdat_file.printf(" * @brief	数値定義ファイル\n")
cdat_file.printf(" * @note	このファイルはコンバーターから出力されます\n")
cdat_file.printf(" *\n")
cdat_file.printf(" */\n")
cdat_file.printf("//======================================================================\n")
cdat_file.printf("\n")

while line = csv_file.gets
  #END検出
  if line =~/^#END/ then
		break
	end
  column = line.split ","
  if column[0].index("//") == 0 then
    cdat_file.printf("%s\n", column[0])
    next
  end

  cdat_file.printf("#define %s (%d)", column[0], column[1].to_i)
  if column[2].index("//") == 0 then
    cdat_file.printf("\t%s\n", column[2])
  else
    cdat_file.printf("\n")
  end
end

