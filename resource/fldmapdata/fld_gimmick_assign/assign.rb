$KCODE = "SJIS"
require 'excel_lib'

#データクラス
class DATA
	def initialize
		@ZoneID = 0
		@ResNum = 0
		@ObjNum = 0
	end

	attr_accessor	:ZoneID,:ResNum,:ObjNum
end


#ソーンＩＤヘッダーファイルをロードする
zone_file = open(ARGV.shift,"r")
zone_ary = Array.new
#3行読み飛ばし
line = zone_file.gets
line = zone_file.gets
line = zone_file.gets

while line = zone_file.gets
	column = line.split " "
	#「#define」を含む行を検出
	if (column[0] == "#define") then
		#配列に格納する
		zone_ary << column[1]
	end
end

ary = Array.new
check_ary = Array.new
xls_name = ARGV.shift
file = open(ARGV.shift,"wb")

#エクセル読む
excel = Excel.new(false)
book = excel.copy_book(xls_name)
sheet = book.sheets[1]
range = sheet.range("A2:C2").get_cursol

p "Gimmick assgin start"
loop do
  break if range.text[0] == "#END"	#END検出
  column = range.text
  #リアクションコードを検出
	zone_name = "ZONE_ID_" + column[0].to_s
  p zone_name
			
	#重複チェック
  check = check_ary.index(zone_name)
  if check != nil then
    printf( "ERROR::%s alrady read\n",column[0].to_s)
    exit(-1)
  end

  data = DATA.new
  data.ZoneID = zone_ary.index(zone_name)
  data.ResNum = column[1].to_i
  data.ObjNum = column[2].to_i

  ary.clear
  ary << data.ZoneID
  ary << data.ResNum
  ary << data.ObjNum
  pack_str = ary.pack("LS2")	#8byte
  file.write(pack_str)

  range.v_move(1)
end

p "Gimmick assgin end"

