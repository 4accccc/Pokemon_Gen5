#	//エリア管理表を開く
#	//2行読み飛ばし
#	//エリアコード、配置モデルデータ、テクスチャセット、人物データ、ライトフラグをリード
#	//それぞれをベクタ登録
#	//登録の際、登録インデックスを返すようにする
#	//返ってきたインデックスをエリアデータとして保存
#	//ライトフラグ保存
#	//1行ごとにバイナリを作成
#	//エリアID分繰り返し、終了したら、各ベクタをアーカイブ用スクリプトファイルとして出力

COL_AREANAME	=	0
COL_BMNAME		=	1
COL_TEXNAME		=	2
COL_TEXPART1	=	3
COL_TEXPART2	=	4
COL_ANMNAME		=	5
COL_INOUT		=	6
COL_LIGHTTYPE	=	7
#配列へのエントリ
def EntryVec(vec, item)
	#すでにあるか調べる
	index = vec.index(item)
	if index == nil then
		#登録
		vec << item
		return (vec.length-1)
	else
		return index
	end
end
#配列へのエントリ(指定文字列は省く)
def EntryVec2(vec, item, omit)
	if omit == item then
		return 0xffff	#登録なしコード
	end
	#すでにあるか調べる
	index = vec.index(item)
	if index == nil then
		#登録
		vec << item
		return (vec.length-1)
	else
		return index
	end
end

#屋内外設定取得
def GetInnerOuter(str)
	fix_str = str.chomp("\n").chomp("\r")
	if "OUT" == fix_str then
		return 1;
	elsif "IN" == fix_str then
		return 0;
	end
	#指定不明の場合は0をセット
	return 0;
end

#ライト設定取得
def Getlight(str)
	fix_str = str.chomp("\n").chomp("\r")
	if "ON" == fix_str then
		return 1;
	elsif "OFF" == fix_str then
		return 0;
	elsif "TOUDAI" == fix_str then
		return 2;
	end
	#指定不明の場合は0をセット
	return 0;
end

#拡張子変更
def ChangeExtend(inStr, inExtStr)
	idx = inStr.index(".")+1
	last = inStr.length-1

	str = inStr.dup
	str[idx..last] = inExtStr
	return str
end

#スクリプトファイル作成
def MakeScript(vec, inFileName, inExt)
	script_file = File.open(inFileName,"w")
	vec.each do | item |
		file_name = ChangeExtend(item, inExt);
		script_file.printf("\"%s\"\n",file_name)
	end
end

#ファイルパックライト
def FileWrite(bin_file, data, code)
	ary = Array(data)
	pack_str = ary.pack(code)
	bin_file.write(pack_str)
end

#area_tbl_file = File.open("area_table.txt","r")
area_tbl_file = File.open(ARGV[0],"r")
area_id_h = File.open("area_id.h","w")

area_id_h.printf("//このファイルはコンバータにより生成されます\n")
area_id_h.printf("enum {\n")

area_vec =[]
build_vec = []
tex_vec = []
g_anm_vec = []
marge_tex_before = []

total_bin_file = File.open("area_data.bin","wb")

#1行読み飛ばし
line = area_tbl_file.gets
while line = area_tbl_file.gets
	column = line.split "\t"

	#エリアＩＤ列挙
	area_id_h.printf("\t%s,\n",column[COL_AREANAME])

	#エリアバイナリ作成
	bin_file_name = "#{column[COL_AREANAME]}.bin"
	#小文字化
	bin_file_name.downcase!
	#エントリ
	EntryVec(area_vec, bin_file_name)
	#binファイル作成
#	bin_file = File.open(bin_file_name,"wb")
	#データ書き込み
	data = EntryVec(build_vec,column[COL_BMNAME])	#モデル
#	FileWrite(bin_file,data, "S")
	FileWrite(total_bin_file,data, "S")
	data = EntryVec(tex_vec,column[COL_TEXNAME])		#テクスチャセット
#	FileWrite(bin_file,data, "S")
	FileWrite(total_bin_file,data, "S")
	data = EntryVec2(g_anm_vec,column[COL_ANMNAME],"none")	#地形アニメファイル
#	FileWrite(bin_file,data, "S")
	FileWrite(total_bin_file,data, "S")
	data = GetInnerOuter(column[COL_INOUT])			#INNER/OUTER
#	FileWrite(bin_file,data, "C")
	FileWrite(total_bin_file,data, "C")
	data = Getlight(column[COL_LIGHTTYPE])				#ライト
#	FileWrite(bin_file,data, "C")
	FileWrite(total_bin_file,data, "C")

	#マージ前テクスチャを収集
	EntryVec(marge_tex_before,column[COL_TEXPART2])	#マージ前テクスチャ1
	EntryVec(marge_tex_before,column[COL_TEXPART2])	#マージ前テクスチャ2
	#"dummy"は省く
	marge_tex_before.delete("dummy")
	
	
end

total_bin_file.close
#テイル作成
area_id_h.printf("};")

#ベクタを使用して、アーカイブ用スクリプトファイルを作成
#	//エリア
MakeScript(area_vec, "area_list.txt", "bin" );
#	//配置
MakeScript(build_vec, "build_list.txt", "dat" );
MakeScript(build_vec, "build_xls_list.txt", "xls" );
#	//テクスチャ
MakeScript(tex_vec, "tex_list.txt", "nsbtx" );
#	//マージ前テクスチャ
MakeScript(marge_tex_before, "tex_imd_list.txt", "imd" );
#	//地形アニメ
MakeScript(g_anm_vec, "g_anm_list.txt", "nsbta" );



