lib_path1 = File.dirname(__FILE__).gsub(/\\/,"/") + "/../ncg2imd/ncg_parser"
lib_path2 = File.dirname(__FILE__).gsub(/\\/,"/") + "/../ncg2imd/ncl_parser"
lib_path3 = File.dirname(__FILE__).gsub(/\\/,"/") + "/../ncg2imd/rewrite_imd"
require lib_path1
require lib_path2
require lib_path3

def GetFileName(pathfile)
	last = pathfile.rindex("/")
	
	file = pathfile.dup
	file[0,last+1] = ""
	p pathfile
	p last
	p file
	return file
end

def GetAddPath(path,word)
	size = path.size
	tmp = path.rindex(word)
  if tmp == nil then
    last = 0
  else
    last = tmp
  end
	del_size = size-last
	add_path = path.dup
	add_path[last,del_size] = ""
	p add_path
	return add_path
end

def FixPalText(inPaletteData, ary)
	i = 0
	while i < 1
		n = 0
		str = ""
		while n < 16
			if (n == 0)||(n==8) then
				str += "\n\t\t"
			end
			col = sprintf("%04x ",inPaletteData[i*16+n])
			str += col
			n+=1
		end
		str += "\n\t"
		ary << str
		i+=1
	end
end

class FIX_TEX_DATA_MAKER
	def initialize(w,h,flg = TRUE)
		line_num = h*8
		if flg == TRUE then
			@LineData = (0...line_num).map{ Array.new }
		else
			@LineData = []
		end
		@TexW = w		#キャラクタサイズ指定
		@TexH = h		#キャラクタサイズ指定
	end

	def PushData(inTex)
		char_no = 0
		char_floor = 0
		
		inTex.each { | char |
			space_flg = 0
			char_line = 0
			dot_count = 0
			line_ofs = char_floor*8
			char.each { | data |
				#2ドット分処理（16色のとき限定）
				dot_count+=2
	
				#現在処理中のラインを計算
				line = line_ofs + char_line
	
				#下位4ビットを取得
				low = (data & 0x0f)
				#上位4ビットを取得
				hi = ((data>>4) & 0x0f)
				#文字列として、データを格納
				new_str = sprintf("%x%x",hi,low)
				#格納スペースのあるデータがあるなら、それを取得してそこにセット
				#ない場合は、新規に作る
				if space_flg == 1 then		#空きアリ
					array = @LineData[line]
					str = array.pop
					str = new_str + str
					array << str
					#スペースが埋まった
					space_flg = 0
				else						#空きナシ
					array = @LineData[line]
					array << new_str
					#スペースができた
					space_flg = 1
				end
	
				if dot_count%8 == 0 then
					#ライン更新
					char_line+=1
				end
			} #end data
			char_no+=1
			#処理キャラが、指定テクスチャのキャラ横サイズをオーバーしていれば処理キャラ階層を更新する
			if (char_no % @TexW == 0) then
				char_floor+=1
			end
		} #end char
	end

	def PushLineData(inLineData)
		@LineData << inLineData
	end

	#反転テクスチャを作成
	def MakeReverseTex
		fix_data_maker = FIX_TEX_DATA_MAKER.new(@TexW, @TexH, FALSE)

		@LineData.each { | line |
			#ラインデータを逆転させて作る
			rev = line.reverse
			new_line = Array.new(line.length,"")
			count = 0
			rev.each { | s |
				s_split = s.scan(/.{1,1}/m)
				s_rev = s_split.reverse
				new_line[count] = s_rev.join
				count+=1
			}
			fix_data_maker.PushLineData(new_line)
		}
		return fix_data_maker
	end

	def MakeText
		str = ""
		count = 0
		@LineData.each { | line |
			str += "\n\t\t\t"
			line.each { | l |
				str += l
				str += " "
			}
		}
		str += "\n\t\t"
		return str
	end
	
	def Dump
		@LineData.each { | line |
			p line
		}
	end
end

p File.dirname(__FILE__).gsub(/\\/,"/")

#コマンド引数
#NCGファイル、出力先、1コマの幅（キャラ）、1コマの高さ（キャラ）、テクスチャ数
ncg_file_name = ARGV.shift
output_path = ARGV.shift
use_dmy_imd = ARGV.shift
one_tex_width = ARGV.shift.to_i
one_tex_height = ARGV.shift.to_i
tex_num_max = ARGV.shift.to_i

ncg_parser = NCG_STRUCT.new
ncg_parser.Pars(ncg_file_name)
#相対パスの部分を抽出
add_path = GetAddPath(ncg_file_name,"/")

char_data = ncg_parser.GetCharData
ncg_width = ncg_parser.GetWidth

tex_data_array = []

#floor:		NCGでマッピングされている1コマのキャラデータの羅列の階層
#			NCGの横サイズを使い切っても、コマが足らない場合は、
#			折り返して、1コマ目の下（1コマ縦64ドットだった場合は縦に64ドット下がった所）からマッピングされる
#			この折り返し回数をfloorとする
#pat:		横並びのコマに対して、何コマ目を処理しているかを示す
#			コマ折り返し（floorの値が増える）が発生した場合、patは0に戻る
#height:	1コマ内の横キャラ列において、何列目を処理しているかを示す
#width:		1コマ内の横キャラ列において、何キャラ目を処理しているかを示す
#byte_par_char:
#			1キャラ何バイトかを示す

byte_par_char = 0
floor = 0
pat = 0

tex_num = 0

if (1) then
	byte_par_char = 32
else
	byte_par_char = 64
end

while tex_num < tex_num_max
	tex_data = []
	floor_ofs = floor*one_tex_height*ncg_width*byte_par_char
	pat_ofs = pat*one_tex_width*byte_par_char
	
	height = 0
	#1コマ処理
	while height < one_tex_height
		height_ofs = height*ncg_width*byte_par_char
		width = 0
		while width < one_tex_width
			char_dat = []
			#キャラ取得
			char_ofs = width*byte_par_char
			shift = 0
			while shift < byte_par_char
				ofs = shift + floor_ofs + height_ofs + pat_ofs + char_ofs
				char_dat << char_data[ofs]
				shift+=1
			end
			width+=1
			tex_data << char_dat
		end
		height+=1
	end
	#floor・patを更新
	pat+=1
	if ((pat+1)*one_tex_width > ncg_width) then
		pat = 0
		floor+=1
	end
	
	tex_num += 1
	tex_data_array << tex_data
end

#テクスチャアレイを解釈してIMDに適用できる形に加工する
maker = []

tex_data_array.each { | tex |
	fix_data_maker = FIX_TEX_DATA_MAKER.new(one_tex_width, one_tex_height)
	fix_data_maker.PushData(tex)
	maker << fix_data_maker
}

#maker.each { | i |  i.Dump}
p add_path + "/" + ncg_parser.GetPalFileName
ncl_parser = NCL_STRUCT.new(1)
ncl_parser.Pars(add_path + "/" + ncg_parser.GetPalFileName)

palette_data = ncl_parser.GetPaletteData
#パレットテキストを整形する
fix_pal_text_ary = Array.new(0)
FixPalText(palette_data,fix_pal_text_ary)
#テクスチャイメージとパレットデータを書き換える
tex_text_array = []
maker.each { | i |
	tex_text_array << i.MakeText
}

output_file_name = GetFileName( ncg_file_name.sub(/.ncg/,".imd") )

rewrite = REWRITE_TEX.new(tex_text_array,fix_pal_text_ary)

#テンプレートを選択
template = ""
template = use_dmy_imd

rewrite.OutPut(template, output_path + "/" + output_file_name)

