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

load 'area_common.def'	#COL_～の定義など

#NAIXReaderの読み込み
lib_path_naixread = ENV["PROJECT_ROOT"] + "tools/naixread"
require lib_path_naixread


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

begin
  area_tbl_file = File.open(ARGV[0],"r")
  id_file = AreaIDMaker.new(TARGET_HEADER_FILENAME)

  build_vec = []
  tex_vec = []
  anm_ita_vec = []

  total_bin_file = File.open(TARGET_BIN_FILENAME, "wb")
  total_txt_file = File.open("area_data.txt", "w")

  #1行読み飛ばし
  #line = area_tbl_file.gets
	READTHROUGH_LINES.times{|| area_tbl_file.gets}

  area_count = 0
  while line = area_tbl_file.gets
    column = line.split "\t"

    #area_name = getAreaName(column)
    #エリアＩＤ列挙
    id_file.putAreaName(column)

    #binファイル作成
    #データ書き込み
    bm_id = EntryVec(build_vec,column[COL_BMNAME])	#モデル
    FileWrite(total_bin_file,bm_id, "S")
    tex_id = EntryVec(tex_vec,column[COL_TEXNAME])		#テクスチャセット
    FileWrite(total_bin_file,tex_id, "S")
    anm_ita_id = EntryVec2(anm_ita_vec,column[COL_ITA_NAME],"none")	#地形アニメファイル
    FileWrite(total_bin_file,anm_ita_id, "S")
    inout = GetInnerOuter(column[COL_INOUT])			#INNER/OUTER
    FileWrite(total_bin_file,inout, "C")
    light = Getlight(column[COL_LIGHTTYPE])				#ライト
    FileWrite(total_bin_file,light, "C")
    total_txt_file.printf("AREA:%3d BM:%2d TEX:%2d ANM:%2d IO:%d LIGHT:%d\n",
                area_count, bm_id, tex_id, anm_ita_id, inout, light);

    area_count += 1
    
  end

  total_bin_file.close
  total_txt_file.close
  #テイル作成
  id_file.close

end



