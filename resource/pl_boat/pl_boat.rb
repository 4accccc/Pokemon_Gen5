$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")

dat_file_name = ARGV.shift
dat_file = open(dat_file_name,"w")

dat_file.printf("//======================================================================\n")
dat_file.printf("/**\n")
dat_file.printf(" * @file	%s\n",dat_file_name)
dat_file.printf(" * @brief	船内部屋ＯＢＪ定義ファイル\n")
dat_file.printf(" * @note	このファイルはコンバーターから出力されます\n")
dat_file.printf(" *\n")
dat_file.printf(" */\n")
dat_file.printf("//======================================================================\n")
dat_file.printf("\n")

#1行飛ばし
csv_file.gets

#トレーナー
dat_file.printf("static const TR_DATA TrData[TRAINER_MAX] = {\n")
while line = csv_file.gets
  #TR_END検出
  if line =~/^#TR_END/ then
		break
	end
  
  column = line.split ","

  dat_file.printf("\t{%s, %s, {%s, %s}},\n",column[0].upcase, column[1].upcase, column[2] ,column[3].chomp("\r").chomp("\n"))

end
dat_file.printf("};\n")
dat_file.printf("\n")

#長話
dat_file.printf("static const NONE_TR_DATA LongTalkerData[LONG_TALKER_MAX] = {\n")
while line = csv_file.gets
  #TR_END検出
  if line =~/^#LONG_END/ then
		break
	end
  
  column = line.split ","

  dat_file.printf("\t{%s, {%s, %s}},\n",column[0].upcase, column[1], column[2].chomp("\r").chomp("\n"))

end
dat_file.printf("};\n")
dat_file.printf("\n")

#普通話
dat_file.printf("static const NONE_TR_DATA ShortTalkerData[SHORT_TALKER_MAX] = {\n")
while line = csv_file.gets
  #TR_END検出
  if line =~/^#SHORT_END/ then
		break
	end
  
  column = line.split ","

  dat_file.printf("\t{%s, {%s, %s}},\n",column[0].upcase, column[1], column[2].chomp("\r").chomp("\n"))

end
dat_file.printf("};\n")
