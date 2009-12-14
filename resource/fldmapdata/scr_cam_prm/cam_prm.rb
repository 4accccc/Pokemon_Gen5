$KCODE = "SJIS"

def SetValid(ary, str)
  if str == "ON" then
    ary << 1
  else
    ary << 0
  end
end

csv_file = open(ARGV.shift,"r")

h_file_name = ARGV.shift
h_file = open(h_file_name,"w")

list_file_name = "arc_list"
list_file = open(list_file_name,"w")

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

ary = Array.new

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

  #バイナリデータ作成
  bin_name = "bin/" + column[0].downcase + ".bin"
  bin_file = open(bin_name,"wb")

  list_file.printf("\"%s\"\n",bin_name)

  ary.clear
  ary << column[1].to_i   #pitch
  ary << column[2].to_i   #yaw
  ary << column[3].to_i   #distance
  ary << column[4].to_i   #pos x
  ary << column[5].to_i   #pos y
  ary << column[6].to_i   #pos z

  ary << 0   #shift check
  SetValid(ary, column[7].chomp("\n").chomp("\r")) #pitch check
  SetValid(ary, column[8].chomp("\n").chomp("\r")) #yaw check
  SetValid(ary, column[9].chomp("\n").chomp("\r")) #distance check
  ary << 0   #Fovy check
  SetValid(ary, column[10].chomp("\n").chomp("\r")) #pos check

  pack_str = ary.pack("S2L4L6")	#44byte
  bin_file.write(pack_str)
  
  index += 1
end


