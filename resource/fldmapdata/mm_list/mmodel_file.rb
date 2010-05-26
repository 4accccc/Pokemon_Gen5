###############################################################
#
#
#
#
###############################################################

FIELDOBJ_CODE_HEADER	=	"../../fldmmdl/fldmmdl_objcode.h"

#---------------------------------------------------
#---------------------------------------------------
def read_table infile
	mml = Array.new
	area_count = 0
	mml_count = 0
	now_id = nil
	line = infile.gets	#	読み飛ばし
	while line = infile.gets
		#p "#{line}"
		m = /(MMLID_\w+)\W(\d+)\W([0-9A-Z_]+)\b/.match line
    #p m
		if m == nil 
			puts "NOT HIT!"
			break
		else
			areaID = m[1]
			mmlID = m[3]
			unless mmlID =~ /^[0-9A-Z_]+$/
				STDERR.puts "動作モデルIDが正しくありません！：#{mmlID}\n"
				exit 1
			end
			if now_id == nil
				now_id = areaID
				mml[area_count] = Array.new
				mml[area_count][mml_count] = areaID
			elsif now_id != areaID
				area_count += 1
				mml_count = 0
				now_id = areaID
				mml[area_count] = Array.new
				mml[area_count][mml_count] = areaID
			end
			mml_count += 1
			mml[area_count][mml_count] = mmlID
		end
	end
	return mml
end

#---------------------------------------------------
#---------------------------------------------------
def make_asm_text ofile, ar
	#STDERR.puts "#{ofile.path}...#{ar}"
	ofile.puts <<HEADER
#常駐動作モデル定義リスト	#{ofile.path}

	.text

	.include	"#{FIELDOBJ_CODE_HEADER}"

HEADER

  if ar.length > 26 then
    printf("ERROR:管理できる人数を超えています %d\n", ar.length);
    exit -1
  end

  ofile.printf("\t.short\t%d\n", ar.length)

	ar.each{|id|
		ofile.printf("\t.short\t%s\n", id)
	}

	ofile.puts "\n"
	ofile.puts "\t.short\tOBJCODEMAX\n"
  if ar.length % 2 != 0 then
    #４バイトアライメントのためにダミーコードを入れる
    ofile.puts "\t.short\tOBJCODEMAX\n"
  end
	ofile.puts "\n"
end

def MakePrmBinTbl(ary)
  open_file_str = "../../fldmmdl/fldmmdl_mdlparam.bin"
  stat = File.stat(open_file_str)
  bin_file = File.open(open_file_str,"rb")

  p "SIZE:" + stat.size.to_s
  #4バイトはヘッダー
  header = bin_file.read(4)
  p "HEADER" + header
  one_size = 28
  data_num = stat.size / one_size
  p "DATA NUM:" + data_num.to_s
  for i in(0...data_num) do
    #str = "data_" + i.to_s + ".bin"
    #bin = File.open(str,"wb")
    ary << bin_file.read(one_size)
  end
end

def MakeMdlList(ary)
  file = File.open("fldmmdl_list.csv","r")
  #1行とばし
  line = file.gets
  while line = file.gets
    #END検出
    if line =~/^#END/ then
		  break
	  end
    column = line.split ","
    ary << column[7]
  end
  file.close

  file = File.open("fldmmdl_poke_list.csv","r")
  #1行とばし
  line = file.gets
  while line = file.gets
    #END検出
    if line =~/^#END/ then
		  break
	  end
    column = line.split ","
    ary << column[7]
  end
  file.close
  
  file = File.open("fldmmdl_mdl_list.csv","r")
  #1行とばし
  line = file.gets
  while line = file.gets
    #END検出
    if line =~/^#END/ then
		  break
	  end
    column = line.split ","
    ary << column[7]
  end
  file.close
end

def MakeAreaMvMdlPrm(sym, b_ary, m_ary, ar )
  printf("mmdlprm_%s make\n",sym)
  str = sym + ".prm"
  bin = File.open(str,"wb")
  ar.each{|id|
		idx = m_ary.index(id)
    bin.write( b_ary[idx] )
	}
end

#---------------------------------------------------
#---------------------------------------------------
bin_ary = Array.new
MakePrmBinTbl(bin_ary)
mmdl_ary = Array.new
MakeMdlList(mmdl_ary)
#入力ファイルから定義を読み込む
infile = File.open(ARGV[0],"r")
mml = read_table infile
infile.close

#ダミー用定義を追加
mml << ["mmlid_noentry"]


arclist = File.open(ARGV[1],"w")
archead = File.open(ARGV[2],"w")
archead_def = "__#{ARGV[2].upcase.gsub(/\./,"_")}__"

archead.puts <<HEADER
//	動作モデルふりわけ指定ID

\#ifndef	#{archead_def}
\#define #{archead_def}

HEADER

linecount = 0
mml.each{|ar|
	sym = ar[0].downcase

	#定義ごとにファイル生成
	outfile = File.open("#{sym}.s", "w")
	make_asm_text outfile, ar[1..ar.length - 1]
	outfile.close

	#アーカイブ指定ファイルに一行出力
	arclist.puts "\"#{sym}.bin\""

	#
	archead.printf "#define	%-20s %d\n",sym.upcase, linecount

	linecount += 1

  #エリア別動作モデルパラメータバイナリを作成
  MakeAreaMvMdlPrm(sym, bin_ary, mmdl_ary, ar[1..ar.length - 1] )
}

archead.puts "#endif"
arclist.close
archead.close

=begin
mml.each{|value|
	puts "#{value[0]}..."
	(1 .. value.length).each{|n| printf "[%d]%s:",n,value[n]}
	puts "\n"
}
=end
