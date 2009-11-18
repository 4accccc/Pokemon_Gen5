#フィールドカメラデータ生成

$KCODE = "SJIS"

COL_NUMBER  = 0
COL_LENGTH  = 1
COL_PITCH   = 2
COL_YAW     = 3
COL_ROLL    = 4
COL_VIEWTYPE = 5
COL_DEPTH   = 6
COL_FOVY    = 7
COL_NEAR    = 8
COL_FAR     = 9
COL_PLLINK  = 10
COL_OFSX    = 11
COL_OFSY    = 12
COL_OFSZ    = 13
COL_COMMENT = 14

class VEC_DATA
	#外部参照できるようにする
	attr_accessor :X, :Y, :Z
	def initialize()
		@X = ""
		@Y = ""
		@Z = ""
	end
end

class CLIP_DATA
	#外部参照できるようにする
	attr_accessor :Near, :Far
	def initialize()
		@Near = ""
		@Far = ""
	end
end

class CAMERA_DATA
	#外部参照できるようにする
	attr_accessor :Distance, :Angle, :ViewType, :DepthType, :PerspWay, :Clip, :Shift, :Comment, :PlayerLink
	def initialize()
		@Distance = ""
		@Angle = VEC_DATA.new
		@ViewType = ""
    @DepthType = ""
		@PerspWay = ""
		@Clip = CLIP_DATA.new
		@Shift = VEC_DATA.new
		@Comment = ""
    @PlayerLink = ""
	end
end

#マイナスチェック
def CheckMinus(integer, bit)
	#15ビット目をチェックし負かどうか判定
	return integer[bit]
end

#シフト値文字列作成
def MakeShiftStr(hex_str)
	#16進数文字列を整数化
	integer = hex_str.hex
	#マイナスチェック
	if CheckMinus(integer,31) == 1 then
		pre_str = "-0x"
		#限界値から差し引く
		val = 0x100000000 - integer
	else
		pre_str = "0x"
		val = integer
	end
	#文字列化
	str = format("%x", val)
	return pre_str + str
end

#アングル値文字列作成
def MakeAngleStr(hex_str)
	#16進数文字列を整数化
	integer = hex_str.hex
	#限界値から差し引く
	val = (0x10000 - integer) % 0x10000

	#値は0か?
	if val == 0 then
		pre_str = "0x"
	else
		#マイナスチェック
		if CheckMinus(val,15) == 1 then
			#マイナスならプラスに
			pre_str = "0x"
		else
			#プラスならマイナスに
			pre_str = "-0x"
		end
	end
	#文字列化
	str = format("%x", val)
	return pre_str + str
end

csv_file = open(ARGV.shift,"r")

data_array = Array.new

#一行読み飛ばし
line = csv_file.gets

while line = csv_file.gets
	column = line.split ","

  if column.size < 2 then break end
	data = CAMERA_DATA.new

	#距離
	data.Distance = "0x" + column[COL_LENGTH]
	#アングル
	data.Angle.X = MakeShiftStr(column[COL_PITCH])
	data.Angle.Y = MakeShiftStr(column[COL_YAW])
	data.Angle.Z = MakeShiftStr(column[COL_ROLL])
	#data.Angle.X = MakeAngleStr(column[2])
	#data.Angle.Y = MakeAngleStr(column[3])
	#data.Angle.Z = MakeAngleStr(column[4])

	#ビュータイプ
	if column[COL_VIEWTYPE] == "PER" then
		data.ViewType = "GF_CAMERA_PERSPECTIV"
	elsif column[COL_VIEWTYPE] == "ORTHO" then
		data.ViewType = "GF_CAMERA_ORTHO"
	else
		data.ViewType = "GF_CAMERA_PERSPECTIV"
	end
  if column[COL_DEPTH] == "ZBUF" then
    data.DepthType = "DEPTH_TYPE_ZBUF"
  elsif column[COL_DEPTH] == "WBUF" then
    data.DepthType = "DEPTH_TYPE_WBUF"
  else
    raise Exception
  end
	#パース
	data.PerspWay = "0x" + column[COL_FOVY]
	#クリップ
	data.Clip.Near = "FX32_ONE * 0x" + column[COL_NEAR]
	data.Clip.Far = "FX32_ONE * 0x" + column[COL_FAR]
	#data.Clip.Near = "FX32_ONE * " + column[7]
	#data.Clip.Far = "FX32_ONE * " + column[8]
  #プレイヤーのリンク
  if column[COL_PLLINK] == "○" then
    data.PlayerLink = "1"
  else
    data.PlayerLink = "0"
  end
	#シフト
	data.Shift.X = MakeShiftStr(column[COL_OFSX])
	data.Shift.Y = MakeShiftStr(column[COL_OFSY])
	data.Shift.Z = MakeShiftStr(column[COL_OFSZ])
	#コメント
	data.Comment = column[COL_COMMENT].chomp
	#配列に格納
	data_array << data
end

#ファイルに書き込む
dat_file_content = ""
data_array.each_with_index{|data, i|
	dat_file_content += sprintf("\t{\t\t\t\t\t\t\t// #{i} #{data.Comment}\n")
	dat_file_content += sprintf("\t\t#{data.Distance},\n")
	dat_file_content += sprintf("\t\t{\n")
	dat_file_content += sprintf("\t\t\t#{data.Angle.X},#{data.Angle.Y},#{data.Angle.Z}\n")
	dat_file_content += sprintf("\t\t},\n")
	dat_file_content += sprintf("\t\t#{data.ViewType},\n")
  dat_file_content += sprintf("\t\t#{data.DepthType},\n")
	dat_file_content += sprintf("\t\t#{data.PerspWay},\n")
	dat_file_content += sprintf("\t\t#{data.Clip.Near},\n")
	dat_file_content += sprintf("\t\t#{data.Clip.Far},\n")
	dat_file_content += sprintf("\t\t#{data.PlayerLink},\n")
	dat_file_content += sprintf("\t\t{#{data.Shift.X},#{data.Shift.Y},#{data.Shift.Z}}\n")
	dat_file_content += sprintf("\t},\n")
}

dat_file = open("field_camera.dat","w")
dat_file.printf("//このファイルはコンバータにより出力されます\n")

dat_file.printf("static const FLD_CAMERA_PARAM FieldCameraData[] = {\n")
dat_file.puts "#{dat_file_content}"

dat_file.printf("};\n")



