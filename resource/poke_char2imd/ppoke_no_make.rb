#連れ歩きポケモンのグラフィックを特定する情報をdatファイルとして出力する

$KCODE = "SJIS"

csv_file = open(ARGV.shift,"r")
dat_file = open("pair_poke_idx.dat","w")

line = csv_file.gets
line = csv_file.gets

val = 0;
#ハッシュ作成
hash = Hash.new
form_hash = Hash.new
sexdif_hash = Hash.new
while line = csv_file.gets
	form_flg = 0;
	column = line.split ","
	name = column[1]
	#ポケモン名の「・」までを取得
	m = /・/.match("#{name}")
	if m != nil then
		poke_name = m.pre_match
		form_flg = 1;
	else
		#ポケモン名の「／」までを取得
		m = /／/.match("#{name}")
		if m != nil then
			poke_name = m.pre_match
		else
			poke_name = name
		end
	end
	
	#ハッシュに追加
	if hash.key?(poke_name) == false then
		#追加
		hash[poke_name] = val
		form_hash[poke_name] = 0;
		sexdif_hash[poke_name] = 0;
	else
		if form_flg == 1 then
			form_hash[poke_name] = form_hash[poke_name] + 1;
		else
			sexdif_hash[poke_name] = 1;
		end
	end
	
	val+=1
end

sorted = hash.sort_by{|key,val| val}

dat_file.printf("//このファイルはコンバーターにより出力されます\n\n")
dat_file.printf("static const u16 PairPokeIdxTbl[] = {\n")

dat_file.printf( "\t0,\t\t//ダミー\n" )

sorted.each_with_index{ | i,idx |
	no = idx+1
	dat_file.printf( "\t#{i[1]},\t\t//No.#{no} #{i[0]}\n" )
}

dat_file.printf("};")


dat_file.printf("\n\n")
#フォルム数を配列化する

dat_file.printf("static const u16 PairPokeFormNum[] = {\n")

sorted.each_with_index{ | i,idx |
	no = idx+1
	poke_name = i[0]
	dat_file.printf( "\t#{form_hash[poke_name]},\t\t//No.#{no} #{i[0]}\n" )
}

dat_file.printf("};")

dat_file.printf("\n\n")
#性別の有無を配列化する

dat_file.printf("static const u16 PairPokeSexDif[] = {\n")

sorted.each_with_index{ | i,idx |
	no = idx+1
	poke_name = i[0]
	dat_file.printf( "\t#{sexdif_hash[poke_name]},\t\t//No.#{no} #{i[0]}\n" )
}

dat_file.printf("};")
