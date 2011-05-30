$KCODE = "SJIS"
require 'excel_lib'
require ENV["PROJECT_ROOT"] + 'tools/enum2hash'

#データクラス
class DATA
	def initialize
		@ZoneID = 0
		@ResNum = 0
		@ObjNum = 0
	end

	attr_accessor	:ZoneID,:ResNum,:ObjNum
end

#field_gimmickのenum定義を読み込み
aHash = enum2hash( ENV["PROJECT_PROGDIR"] + 'src/field/field_gimmick_def.h' )


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
  gmk_id = column[1].to_s
  if gmk_id =~ /^[1-9][0-9]*/ then
    data.ResNum = gmk_id.to_i
    STDERR.printf("%s : %d\n", zone_name, gmk_id.to_i )
  elsif aHash.has_key?( gmk_id ) then
    data.ResNum = aHash[ gmk_id ]
    STDERR.printf("%s : %s(%d)\n", zone_name, gmk_id, aHash[ gmk_id ] )
  else
    STDERR.printf("'%s'は未定義のシンボルです!!\n", gmk_id)
    exit(-1)
  end
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

