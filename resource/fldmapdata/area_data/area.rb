#------------------------------------------------------------------------------
#	//エリア管理表を開く
#	//ヘッダ行読み飛ばし
#	//エリアコード、配置モデルデータ、テクスチャセット、人物データ、ライトフラグをリード
#	//それぞれをベクタ登録
#	//登録の際、登録インデックスを返すようにする
#	//返ってきたインデックスをエリアデータとして保存
#	//ライトフラグ保存
#	//エリアID分繰り返し、終了したら、各ベクタをアーカイブ用スクリプトファイルとして出力
#
#
#	2008.12.18	tamada	wb環境に移植開始
#
#------------------------------------------------------------------------------

load 'area_common.def'
require 'area_common.rb'	#COL_～の定義など

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
def MakeDependFile(vec, inFileName, inExt, symbolName)
	dep_file = File.open(inFileName, "w")
	dep_file.printf("%s = \\\n", symbolName)
	vec.each do |item|
		file_name = ChangeExtend(item, inExt)
		dep_file.printf("\t%s \\\n", file_name)
	end
end

#ファイルパックライト
def FileWrite(bin_file, data, code)
	ary = Array(data)
	pack_str = ary.pack(code)
	bin_file.write(pack_str)
end


##	エラーを定義
class DataTypeError < Exception; end

##	エリアID生成用
class AreaIDMaker
	def initialize(filename)
		@area_count = 0
		@area_names = {}
		@before_name = ""
		@season_count = 0

		@wf = File.open(filename, "w")
		@wf.printf("//このファイルはコンバータにより生成されます\n")
		@wf.printf("enum {\n")
	end

	def close
		@wf.printf("\tAREA_ID_MAX\n");
		@wf.printf("};")
		@wf.close
	end

	def putLine(name)
		raise DataTypeError, "#{name}が多重定義されています" if @area_names.has_key?(name)
		@area_names[name] = @area_count
		@wf.printf("\t%-24s = %3d,\n",name.upcase, @area_count)
	end

	def putAreaName(column)
		area_id = column[COL_AREANAME]
		season_id = column[COL_SEASONID]
		case season_id
		when "SPRING"
			raise DataTypeError, "#{area_id}：#{season_id}の直前がおかしい" if @season_count != 0
			putLine(area_id)
			putLine(area_id + "_" + season_id)
			@season_count += 1
			@before_name = area_id
		when "SUMMER"
			raise DataTypeError, "#{area_id}：#{season_id}の直前がおかしい" if @season_count != 1
			raise DataTypeError, "#{area_id}指定は#{season_id}に不要です" if area_id != ""
			putLine(@before_name + "_" + season_id)
			@season_count += 1
		when "AUTUMN"
			raise DataTypeError, "#{area_id}：#{season_id}の直前がおかしい" if @season_count != 2
			raise DataTypeError, "#{area_id}指定は#{season_id}に不要です" if area_id != ""
			putLine(@before_name + "_" + season_id)
			@season_count += 1
		when "WINTER"
			raise DataTypeError, "#{area_id}：#{season_id}の直前がおかしい" if @season_count != 3
			raise DataTypeError, "#{area_id}指定は#{season_id}に不要です" if area_id != ""
			putLine(@before_name + "_" + season_id)
			@season_count = 0
			@before_name = ""
		when "NO"
			raise DataTypeError, "#{area_id}の直前がおかしい" if @season_count != 0
			putLine(area_id)
			@season_count = 0
			@before_name = ""
		else
			raise DataTypeError, "季節指定に使えない'#{season_id}'が#{area_id}にありました"
		end
		@area_count += 1
	end

end


#------------------------------------------------------------------------------
#	コンバート本体
#------------------------------------------------------------------------------

area_tbl_file = File.open(ARGV[0],"r")
#area_id_h = File.open(TARGET_HEADER_FILENAME, "w")
id_file = AreaIDMaker.new(TARGET_HEADER_FILENAME)
#area_id_h.printf("//このファイルはコンバータにより生成されます\n")
#area_id_h.printf("enum {\n")

build_vec = []
tex_vec = []
g_anm_vec = []
marge_tex_before = []

total_bin_file = File.open(TARGET_BIN_FILENAME, "wb")
total_txt_file = File.open("area_data.txt", "w")

#1行読み飛ばし
#line = area_tbl_file.gets
read_through area_tbl_file

area_count = 0
while line = area_tbl_file.gets
	column = line.split "\t"

	#area_name = getAreaName(column)
	#エリアＩＤ列挙
	#area_id_h.printf("\t%-24s = %3d,\n",area_name.upcase, area_count)
	id_file.putAreaName(column)

	#binファイル作成
	#データ書き込み
	bm_id = EntryVec(build_vec,column[COL_BMNAME])	#モデル
	FileWrite(total_bin_file,bm_id, "S")
	tex_id = EntryVec(tex_vec,column[COL_TEXNAME])		#テクスチャセット
	FileWrite(total_bin_file,tex_id, "S")
	anm_id = EntryVec2(g_anm_vec,column[COL_ANMNAME],"none")	#地形アニメファイル
	FileWrite(total_bin_file,anm_id, "S")
	inout = GetInnerOuter(column[COL_INOUT])			#INNER/OUTER
	FileWrite(total_bin_file,inout, "C")
	light = Getlight(column[COL_LIGHTTYPE])				#ライト
	FileWrite(total_bin_file,light, "C")
	total_txt_file.printf("AREA:%3d BM:%2d TEX:%2d ANM:%2d IO:%d LIGHT:%d\n",
						  area_count, bm_id, tex_id, anm_id, inout, light);

	#マージ前テクスチャを収集
	EntryVec(marge_tex_before,column[COL_TEXPART1])	#マージ前テクスチャ1
	EntryVec(marge_tex_before,column[COL_TEXPART2])	#マージ前テクスチャ2
	#"dummy"は省く
	marge_tex_before.delete("dummy")
	
	area_count += 1
	
end

total_bin_file.close
total_txt_file.close
#テイル作成
#area_id_h.printf("\tAREA_ID_MAX\n");
#area_id_h.printf("};")
id_file.close

#	//配置
MakeScript(build_vec, "build_list.txt", "dat" );
MakeScript(build_vec, "build_xls_list.txt", "xls" );
#	//地形アニメ
MakeScript(g_anm_vec, ARC_ANM_LIST_FILENAME, "nsbta" );



