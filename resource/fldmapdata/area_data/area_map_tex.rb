#------------------------------------------------------------------------------
#
#	area_map_texでの依存ルールファイルなどを生成
#
#
#	2008.12.24	tamada GAME FREAK inc.
#
#------------------------------------------------------------------------------

load "area_common.def"


#------------------------------------------------------------------------------
#	コンバート本体
#------------------------------------------------------------------------------

area_tbl_file = File.open(ARGV[0],"r")


##	元imd→マージしたimd→nsbtxのルールと、依存関係を記述
nsbtx_depend_file = File.open(NSBTX_DEPEND_FILENAME, "w")
nsbtx_depend_file.printf("# area_map_tex.rbにより生成されました\n")

##　nnsarcに引き渡すアーカイブ対象ファイルリストを生成
arclist_file = File.open(NNSARC_TEXLIST_FILENAME, "w")

##	narcとnsbtxの依存関係を設定するためのMakefile定義を生成
nnsarc_depend_file = File.open(NNSARC_DEPEND_FILENAME, "w") 
nnsarc_depend_file.printf("# area_map_tex.rbにより生成されました\n")
nnsarc_depend_file.printf("NNSARC_DEPEND_FILES	= \\\n")

#1行読み飛ばし
READTHROUGH_LINES.times{|| area_tbl_file.gets}

area_count = 0
while line = area_tbl_file.gets
	column = line.split "\t"

	area_count += 1

	imd = "#{MARGED_TEXIMD_DIR}/#{column[COL_TEXNAME]}"
	dst = "#{NSBTXFILESDIR}/#{column[COL_TEXNAME]}".sub(/.imd$/,".nsbtx")
	src1 = "#{SRC_TEXIMD_DIR}/#{column[COL_TEXPART1]}"
	src2 = "#{SRC_TEXIMD_DIR}/#{column[COL_TEXPART2]}"
	srcn = "#{SRC_TEXIMD_DIR}/tex_griddmy.imd"
##nsbtx_depend_file
	if column[COL_TEXPART2] == "dummy" then
		nsbtx_depend_file.printf("%s: %s\n", dst, src1)
		nsbtx_depend_file.printf("\t@echo %s → %s\n", src1, dst)
		nsbtx_depend_file.printf("\t@\$(COPY) %s %s\n", src1, imd)
		nsbtx_depend_file.printf("\t@\$(G3DCVTR) %s -o %s -etex\n", imd, dst )
		nsbtx_depend_file.printf("\n")
	else
		nsbtx_depend_file.printf("%s: %s %s %s\n", dst, src1, src2, srcn)
		nsbtx_depend_file.printf("\t@echo %s + %s → %s\n", src1, src2, dst)
		nsbtx_depend_file.printf("\t@-rm %s\n", imd)
		nsbtx_depend_file.printf("\t@\-$(TEX_MAG) -q -o %s %s %s %s\n", imd, src1, src2, srcn)
		nsbtx_depend_file.printf("\t@\$(G3DCVTR) %s -o %s -etex\n", imd, dst )
		nsbtx_depend_file.printf("\n")
	end
##arclist_file
	arclist_file.printf("\"%s\"\n", dst)
##nnsarc_depend_file
	nnsarc_depend_file.printf("\t%s \\\n", dst)
end

nsbtx_depend_file.close
arclist_file.close
nnsarc_depend_file.close

