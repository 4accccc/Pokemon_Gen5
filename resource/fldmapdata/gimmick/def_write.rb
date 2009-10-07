$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")
cdat_file = open(ARGV.shift,"w")

cdat_file.printf("//このファイルはコンバーターから出力されます\n");
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

