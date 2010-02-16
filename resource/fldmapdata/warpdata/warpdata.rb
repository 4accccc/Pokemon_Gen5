###############################################################
#
#	ワープデータ用コンバータ
#
#
#	date	2006.04.01
#	author	tamada GAME FREAK inc.
#
#
#
###############################################################

COL_NUMBER  = 0
COL_SYMBOL = 1
COL_ROOM_ID = 2
COL_ROOM_X = 3
COL_ROOM_Z = 4
COL_WARPFLAG = 5
COL_FLD_ID = 6
COL_FLD_X = 7
COL_FLD_Z = 8
COL_ESC_ID = 9
COL_ESC_X = 10
COL_ESC_Z = 11
COL_AUTO_FLG = 12
COL_FLGNAME = 13
###############################################################
#
#
#
###############################################################

class OutputFile
	def initialize fname
		@fp = File.open(fname, "w")
		@name = fname
		@linecount = 0
		putHeader
	end

	def puts *content
		@fp.puts *content
	end

	def printf	*content
		@fp.printf *content
	end

	def putHeader
		@fp.puts "//#{Time.now.ctime}"
		@fp.puts "//#{@name}"
	end

	def putFooter
		@fp.puts "//#{@name}"
	end

	def close
		putFooter
		@fp.close
	end

	def filediff diff_file
		f1 = File.open(@name,"r")
		l1 = f1.read
		f1.close
		f2 = File.open(diff_file,"r")
		l2 = f2.read
		f2.close
		return (l1 != l2)
	end

	def diff_overwrite diff_file
		if FileTest.exist? diff_file then
			if filediff(diff_file) == true then
				#差分があった場合は更新
				File.delete diff_file
				File.rename @name, diff_file
			else
				#差分がなかった場合は自分のファイルを削除
				File.delete @name
			end
		else
			#diff_fileが存在しない場合は自分のファイルをdiff_fileにリネーム
			File.rename @name, diff_file
		end
	end

end

###############################################################
###############################################################
#------------------------------------------------
#------------------------------------------------
class WarpDataFile < OutputFile
	def putHeader
		puts "//=================================="
		puts "//=================================="
		puts "static const WARPDATA WarpData[] = {"
	end

	def putLine	room_id, room_x, room_z, escape, fld_id, fld_x, fld_z, exit_id,exit_x,exit_z,arrive, arrive_id
    if arrive_id =~ /^SYS_FLAG_ARRIVE_/ then
      #a_id = "SYS_FLAG_ARRIVE_" + arrive_id.upcase
    else
      raise Exception, "#{arrive_id}:フラグ名の命名規則でない"
    end
		puts "\t{"
		printf "\t\t%s,\n", arrive_id
		printf "\t\t%-5s,", if escape == true then "TRUE" else "FALSE" end
		printf "%-5s,\n", if arrive == true then "TRUE" else "FALSE" end
		printf "\t\t%-20s,%4d,%4d,\n", "ZONE_ID_" + room_id.upcase, room_x, room_z
		printf "\t\t%-20s,%4d,%4d,\n", "ZONE_ID_" + fld_id.upcase, fld_x, fld_z
		printf "\t\t%-20s,%4d,%4d,\n", "ZONE_ID_" + exit_id.upcase, exit_x, exit_z
		printf "\t},\n"
	end

	def putFooter
		puts "};"
		puts "//=================================="
	end
end

#------------------------------------------------
#------------------------------------------------
class WarpHeaderFile < OutputFile
	def putHeader
		puts "//=================================="
		puts "//=================================="
		puts "#ifndef __WARPDATA_H__"
		puts "#define __WARPDATA_H__"
		puts "\n"
	end

	def putLine	id, number
		printf "#define	WARP_ID_%02s (%d)\n",id.upcase, number.to_i
		@current_number = number
	end

	def putFooter
		puts "\n"
		puts "#define	WARP_ID_MAX		(#{@current_number.to_i + 1})"
		puts "\n"
		puts "#endif	//__WARPDATA_H__"
		puts "//"
	end
end



###############################################################
###############################################################

def convert	infile
	#データファイル（別名で作成）
	datafile = WarpDataFile.new("temp_data.cdat")
	#ヘッダファイル（別名で作成）
	headerfile = WarpHeaderFile.new("temp_data.h")

	line = infile.gets	#2行読み飛ばし
	line = infile.gets

	while line = infile.gets
		if line =~/^[1-9]/ then
			cl = line.split
			datafile.putLine( cl[COL_ROOM_ID],cl[COL_ROOM_X],cl[COL_ROOM_Z],
                       cl[COL_WARPFLAG]=="○",
                       cl[COL_FLD_ID],cl[COL_FLD_X],cl[COL_FLD_Z],
                       cl[COL_ESC_ID],cl[COL_ESC_X],cl[COL_ESC_Z],
                       cl[COL_AUTO_FLG]=="○",
                       cl[COL_FLGNAME] )
			headerfile.putLine(	cl[COL_SYMBOL],cl[COL_NUMBER] )
		else
			break
		end
	end

	datafile.close
	headerfile.close

	#もともとのデータファイルと比較、変化があれば上書きする
	datafile.diff_overwrite "warpdata.cdat"
	#もともとのヘッダファイルと比較、変化があれば上書きする
	headerfile.diff_overwrite "warpdata.h"
end

infile = File.open(ARGV[0], "r")
convert infile
infile.close

