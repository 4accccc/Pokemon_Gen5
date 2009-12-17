
#------------------------------------------------------------------------------
#
#		配置モデルのエリア別テクスチャセットリスト生成
#
#		2009.12.17 iwasawa
#
#------------------------------------------------------------------------------
require "fileutils"

INPUT_FILE	= ARGV[0]			#入力ファイル名
OUTPUT_SYM	= ARGV[1]			#出力ファイルシンボル
OUTPUT_DIR	= ARGV[2]			#出力先ディレクトリ名
OUTPUT_TMPDIR	= ARGV[3]			#テンポラリ出力先ディレクトリ名
OUTPUT_DEP  = ARGV[4]     #出力ファイル依存ファイル名
OUTPUT_ARC  = ARGV[5]     #出力アーカイブリストファイル名

OUTPUT_SYM_UP = OUTPUT_SYM.upcase

book = Array.new
sheet = Array.new
book << sheet

$static_model_num = 0
$static_hash = Hash.new

#bookに対してシートごとに分断して保存
File.open(INPUT_FILE, "r"){|input_file|
	while line = input_file.gets
		column = line.split
		#カラムがない＝別シートと判定
		if column.length < 2 then
			if sheet.length > 1
				sheet = Array.new
				book << sheet
			else
				break
			end
		end
		sheet << line
	end
}

class InputFileError < Exception; end
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class Bmodel
	attr :name
	attr :flag

	def initialize(name,flag)
		@name = name
		@flag = flag
	end
end

def make_all_index(first_page)
	bmodel_list = Array.new
	check_list = Hash.new
	count = 0
	first_page.each{|line|
		if count > 0 then
			column = line.split
			name = column[1].downcase
			flag = if column[6] == "常駐" then true else false end
      if flag == true then
        $static_model_num += 1
        $static_hash[name] = true
      end
			
      if check_list.has_key?(name) then
				raise InputFileError, "#{name}が重複しています"
			end
			check_list[name] = true
			bmodel_list << Bmodel.new(name, flag)
		end
		count += 1
	}
	return bmodel_list
end

bmodel_list = make_all_index(book[0])
bmodel_list.each{|bmodel|
#	puts "#{bmodel.name} #{bmodel.flag == true ? 1 : 0}"
}

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
book.delete_at(0)		#1ページ目削除

dep_list = File.open(OUTPUT_DEP,"w")
arc_list = File.open(OUTPUT_ARC,"w")

dep_list.print("DEP_TEXSET_%s =" % [OUTPUT_SYM_UP])

book.each_index{|sheet_index|
	filesym = sprintf("%s/%s%03d", OUTPUT_DIR, OUTPUT_SYM, sheet_index+1)
	filename = filesym + ".nsbtx"

	arc_list.puts "\"#{filename}\""
	dep_list.print "\t\\\n\t#{filename}"
}

arc_list.print "\n\n"
dep_list.print "\n\n"

book.each_index{|sheet_index|
  index = sheet_index+1
	filename = sprintf("%s/%s%03d.imd", OUTPUT_DIR, OUTPUT_SYM, index )
	listname = sprintf("%s/%s%03d.list", OUTPUT_TMPDIR, OUTPUT_SYM, index )

  dep_sym = "DEP_TEXSET_IMD_%s%03d" % [OUTPUT_SYM_UP,index]
  dep_list.print("#{dep_sym} =")

	#puts "#{filename}"
	File.open(listname, "w"){|outfile|
		entrys = Hash.new
		sheet = book[sheet_index]
		sheet.delete_at(0) #1行目削除
		sheet.delete_at(0) #2行目削除
		
    count = 0
		sheet.each{|line|
			column = line.split
      if $static_hash.has_key?(column[1]) then
        next
      end
			entrys[column[1]] = true
      count += 1
		}
    num = $static_model_num+count
#    print($static_model_num.to_s+","+count.to_s+"\n")
    outfile.puts(num.to_s)

		bmodel_list.each_index{|index|
			bmodel = bmodel_list[index]
			if bmodel.flag == true || entrys.has_key?(bmodel.name) then
        model_path = "imdfiles/"+bmodel.name
				outfile.puts( "imdfiles/"+bmodel.name+".imd" )
	      dep_list.print " \\\n\t$(SRCDIR)/#{bmodel.name}.imd"
				#printf("%3d %3d %-5s %-20s\n", count, index, bmodel.flag, bmodel.name)
			end
		}
	}
	dep_list.print "\n\n"
  dep_list.print( "%s: $(%s) %s\n" % [filename,dep_sym,listname])
  dep_list.print( "\t$(TEXMAG) -q -n -o $@ -S %s\n\n" % [listname])
}
arc_list.close
dep_list.close







