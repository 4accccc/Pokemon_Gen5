#CSVファイルから配列を作成

$KCODE = "SJIS"

def MakeList(file, val, path_code, path, array)
list = open(file,"w")
list.printf(path_code + " = " + path)
list.printf("#{val} = \\\n")
yen_flg = 0
array.each{ |i|
	if yen_flg != 0 then
		list.printf(" \\\n")
	else
		yen_flg = 1
	end
	name = i
	list.printf("\t$(#{path_code})#{name}")
}
end

mons_array = []
size_32_array = []
size_64_array = []
imd_array = []

csv_file = open(ARGV.shift,"r")
line = csv_file.gets
line = csv_file.gets
while line = csv_file.gets
#	if line =~/^#END/ then
#		break
#	end

	column = line.split ","

	mons_array << column
	
end

mons_array.each{ |i|
#	if i[6].chomp("\r").chomp("\n") == "○" then
		if i[4] == "4" then
			size_32_array << i[3]
		elsif i[4] == "8" then
			size_64_array << i[3]
		end
		str = i[3].dup
		p str
		imd_array << str.sub(".ncg",".imd")
#	end
}

MakeList("tsure_32x32/pair_poke32x32", "32X32_DATA", "NCG_PATH", "../tsure_data/\n", size_32_array)
MakeList("tsure_64x64/pair_poke64x64", "64X64_DATA", "NCG_PATH", "../tsure_data/\n", size_64_array)
MakeList("imd_list", "IMD_DATA", "IMD_PATH", "tex_imd/\n",imd_array)
