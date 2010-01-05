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
#	2009.05.18  tamada  light/itp/itaの外部データ(*.naix)参照に対応
#	2009.05.19  tamada  エラーになっても出力ファイルが生成されているために
#	                    再度makeすると問題なく通ってしまう対処
#
#------------------------------------------------------------------------------

load 'area_common.def'	#COL_～の定義など

#NAIXReaderの読み込み
lib_path_naixread = ENV["PROJECT_ROOT"] + "tools/naixread"
require lib_path_naixread

NO_ANIME_ID = 0xff

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


def enum_search naix, name
  if name == "none" then return NO_ANIME_ID end
  index = naix.getIndex(name.sub(/\./,"_"))
  if index == nil then
    puts "#{naix.getArchiveName}に#{name}がみつからない！"
    return NO_ANIME_ID
  end
  return index
end

def edge_enum_search naix, name
  if name == "none" then return NO_ANIME_ID end
  name += ".bin"
  index = naix.getIndex(name.sub(/\./,"_"))
  if index == nil then
    puts "#{naix.getArchiveName}に#{name}がみつからない！"
    return NO_ANIME_ID
  end
  return index
end

def roundup(num, div)
  r = num % div
  if r == 0 then
    return num
  else
    return num + div - num % div
  end
end

#------------------------------------------------------------------------------
#	コンバート本体
#------------------------------------------------------------------------------

begin
  ita_enum = NAIXReader.read("area_map_ita/area_map_ita.naix")
  itp_enum = NAIXReader.read("area_map_itp/area_map_itp_tbl.naix")
  light_enum = NAIXReader.read("../../field_light/field_light.naix")

  edge_enum = NAIXReader.read("edgemarking/edgemarking.naix")
  mmdl_color_enum = NAIXReader.read("area_mmdl_color/area_map_mmdl_color.naix")

  area_tbl_file = File.open(ARGV[0],"r")
  id_file = AreaIDMaker.new(TARGET_HEADER_FILENAME)

  build_vec = []
  tex_vec = []

  total_txt_file = File.open("area_data.txt", "w")

  #行読み飛ばし
	READTHROUGH_LINES.times{|| area_tbl_file.gets }

  area_count = 0
  bin_out_total = ""
  area_tbl_file.each{|line|
    column = line.split "\t"
    bin_out = ""

    #エリアＩＤ列挙
    id_file.putAreaName(column)

    #binファイル作成
    #データ書き込み
    bm_id = EntryVec(build_vec, column[COL_BMNAME])	#モデル
    bin_out += [bm_id].pack("S") 

    tex_id = EntryVec(tex_vec,column[COL_TEXNAME])		#テクスチャセット
    bin_out += [tex_id].pack("S") 

    #地形ITAアニメファイル指定
    anm_ita_id = enum_search(ita_enum, column[COL_ITA_NAME].sub(/ita$/,"nsbta"))
    #地形ITPアニメファイル指定
    anm_itp_id = enum_search(itp_enum, column[COL_ITP_NAME].sub(/xls$/,"itptbl"))
    bin_out +=[anm_ita_id,anm_itp_id].pack("CC")

    inout = GetInnerOuter(column[COL_INOUT])			#INNER/OUTER
    bin_out += [inout].pack("C") 

    light_idx = enum_search( light_enum, column[COL_LIGHTTYPE] ) #ライト
    bin_out += [light_idx].pack("C") 

    edge_idx = edge_enum_search( edge_enum, column[COL_EDGEMARK] ) #エッジマーキング
    bin_out += [edge_idx].pack("C")

    mmdl_idx = enum_search( mmdl_color_enum, column[COL_MMDL_COLOR] )
    bin_out += [mmdl_idx].pack("C")

    total_txt_file.printf("AREA:%3d BM:%2d TEX:%2d ANM:%2d IO:%d LIGHT:%d EDGE:%d MDLCOL:%d\n",
                area_count, bm_id, tex_id, anm_ita_id, inout, light_idx, edge_idx, mmdl_idx);

    #bin_out += '0'    #padding
    #bin_out +=  [0xaa].pack("C") * (roundup(bin_out.length, 4) - bin_out.length)
    bin_out_total += bin_out
    area_count += 1
  }

  File.open(TARGET_BIN_FILENAME, "wb"){|file|
    file.write(bin_out_total)
  }

  total_txt_file.close
  #テイル作成
  id_file.close

end



