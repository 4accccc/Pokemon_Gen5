$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")
list_file = open("pair_poke_info_list","w")

line = csv_file.gets
line = csv_file.gets
while line = csv_file.gets
	column = line.split ","
	file_name = column[2].downcase
	dst_file = open("info_bin/#{file_name}.bin","w")
	list_file.printf("\"info_bin/#{file_name}.bin\"\n")

	ary = Array.new
	#足跡
	ary << column[7].to_i
	#大きいサイズ
	if column[4] == "8" then
		ary << 1
	else
		ary << 0
	end
	data = 0
	#浮遊
	if column[8].chomp("\r").chomp("\n") == "○" then
		data = (1<<4)
	end
	#アニメ再生速度1/2
	if column[10].chomp("\r").chomp("\n") == "○" then
		data += 1
	end

	ary << data

	#屋内
	if column[9].chomp("\r").chomp("\n") == "○" then
		ary << 1
	else
		ary << 0
	end	
	
	pack_str = ary.pack("CCCC")
	dst_file.write(pack_str)
	
end

