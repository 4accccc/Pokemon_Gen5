csv_file = open(ARGV.shift,"r")
line = csv_file.gets
line = csv_file.gets
while line = csv_file.gets
	column = line.split ","

	filename = "rev_info/" + column[3].sub(/.ncg/,".txt")
	if column[5] == "1" then
		data = "3,5"
	else
		data = "4,0"
	end

	txt_data = open("#{filename}","w")
	txt_data.print("#{data}")
end

