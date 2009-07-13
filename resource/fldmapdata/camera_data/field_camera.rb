#フィールドカメラデータ生成

$KCODE = "SJIS"

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
	attr_accessor :Distance, :Angle, :ViewType, :PerspWay, :Clip, :Shift, :Comment
	def initialize()
		@Distance = ""
		@Angle = VEC_DATA.new
		@ViewType = ""
		@PerspWay = ""
		@Clip = CLIP_DATA.new
		@Shift = VEC_DATA.new
		@Comment = ""
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
dat_file = open("field_camera.dat","w")

data_array = Array.new

#一行読み飛ばし
line = csv_file.gets

while line = csv_file.gets
	column = line.split ","

	data = CAMERA_DATA.new

	#距離
	data.Distance = "0x" + column[1]
	#アングル
	data.Angle.X = MakeAngleStr(column[2])
	data.Angle.Y = MakeAngleStr(column[3])
	data.Angle.Z = MakeAngleStr(column[4])

	#ビュータイプ
	if column[5] == "PER" then
		data.ViewType = "GF_CAMERA_PERSPECTIV"
	elsif column[5] == "ORTHO" then
		data.ViewType = "GF_CAMERA_ORTHO"
	else
		data.ViewType = "GF_CAMERA_PERSPECTIV"
	end
	#パース
	data.PerspWay = "0x" + column[6]
	#クリップ
	data.Clip.Near = "FX32_ONE * " + column[7]
	data.Clip.Far = "FX32_ONE * " + column[8]
	#シフト
	data.Shift.X = MakeShiftStr(column[9])
	data.Shift.Y = MakeShiftStr(column[10])
	data.Shift.Z = MakeShiftStr(column[11])
	#コメント
	data.Comment = column[12].chomp
	#配列に格納
	data_array << data
end

#ファイルに書き込む
dat_file.printf("//このファイルはコンバータにより出力されます\n")

dat_file.printf("static const FLD_CAMERA_PARAM FieldCameraData[] = {\n")

data_array.each_with_index{|data, i|
	dat_file.printf("\t{\t\t\t\t\t\t\t// #{i} #{data.Comment}\n")
	dat_file.printf("\t\t#{data.Distance},\n")
	dat_file.printf("\t\t{\n")
	dat_file.printf("\t\t\t#{data.Angle.X},#{data.Angle.Y},#{data.Angle.Z}\n")
	dat_file.printf("\t\t},\n")
	dat_file.printf("\t\t#{data.ViewType},\n")
	dat_file.printf("\t\t#{data.PerspWay},\n")
	dat_file.printf("\t\t#{data.Clip.Near},\n")
	dat_file.printf("\t\t#{data.Clip.Far},\n")
	dat_file.printf("\t\t{#{data.Shift.X},#{data.Shift.Y},#{data.Shift.Z}}\n")
	dat_file.printf("\t},\n")
}

dat_file.printf("};\n")



