$KCODE = "SJIS"

def GetIdxByStr(vec, str)
  if str == "NONE" then
    ret_idx = 0xffff
  elsif str == nil then
    #エラー
    p "ERROR::指定文字列がありません"
    exit -1
  else
    ret_idx = vec.index(str)
    if ret_idx == nil then
      #エラー
      printf("ERROR::指定文字列が見つからない %s\n",str)
      exit -1
    end
  end
  return ret_idx
end

cutin_no = 0

csv_file = open(ARGV.shift,"r")
res_file = open(ARGV.shift,"r")
list_file = open("ci_list","w")

#リソースファイルベクタ作成
vec = Array.new
while line = res_file.gets
  file_name = line.delete("\"")
  tmp1 = file_name.sub(/.nsbmd/,".imd")
  tmp2 = tmp1.sub(/\.nsbca/,".ica")
  tmp3 = tmp2.sub(/\.nsbta/,".ita")
  tmp4 = tmp3.sub(/\.nsbtp/,".itp")
  tmp5 = tmp4.sub(/\.nsbma/,".ima")
  tmp = tmp5.sub(/\.nsbva/,".iva")
  name = tmp.chomp("\n").chomp("\r")
#  tmp = /\./.match(file_name)
#  name = tmp.pre_match
  p name
  vec << name
end


ary = Array.new
#1行読み飛ばし
csv_file.gets
while line = csv_file.gets
  ary.clear
  #END検出
  if line =~/^#END/ then
		break
	end
  column = line.split ","

  p column

  #データ収集

  bin_name = "ci_" + cutin_no.to_s

  p bin_name

  #↓プログラムで使用する構造体に合わせます
  ary << GetIdxByStr(vec, column[1])  #SPA
  ary << column[2].to_i               #SPAWAIT
  ary << column[3].to_i               #SPAWAIT
  ary << GetIdxByStr(vec, column[4])  #MDL_1
  ary << GetIdxByStr(vec, column[9])  #MDL_2

  ary << column[8].to_i               #MDL_1ANMWAIT
  ary << column[13].to_i              #MDL_2ANMWAIT
  
  ary << GetIdxByStr(vec, column[5])  #MDL_1ANM_1
  ary << GetIdxByStr(vec, column[6])  #MDL_1ANM_2
  ary << GetIdxByStr(vec, column[7])  #MDL_1ANM_3
  ary << 0xffff  #NONE
  
  ary << GetIdxByStr(vec, column[10])   #MDL_2ANM_1
  ary << GetIdxByStr(vec, column[11])  #MDL_2ANM_2
  ary << GetIdxByStr(vec, column[12])  #MDL_2ANM_3
  ary << 0xffff  #NONE
  
  p ary

  #バイナリ作成
  dst_file = open("bin/#{bin_name}.bin","wb")
  pack_str = ary.pack("LS14")
  dst_file.write(pack_str)

  list_file.write("\"bin/#{bin_name}.bin\"\n")

  cutin_no += 1
end
