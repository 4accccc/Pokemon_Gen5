
TARGETEXT	= '3dppack'
DEPENDFILE	= "dependfile"
#HEIGHTLIST	= "heightlist"
ARCLIST		= "arcfilelist"

dependfile = File.open(DEPENDFILE,"w")
#heightlist = File.open(HEIGHTLIST,"w")
arclist	= File.open(ARCLIST,"w")

depends = "\nARCDEPENDS	= \\\n"
File.open(ARGV[0]){|file|

	#dependfile.puts ".imd.nsbmd:"
	#dependfile.puts "	g3dcvtr $< -emdl"
	#dependfile.puts "	g3dcvtr $< -etex"
	#dependfile.puts ""

	while line = file.gets
		name = line.chomp
		if name == '' then break end
		if name == '#END' then break end
				#データ連結ルール
		dependfile.puts "#{name}.#{TARGETEXT}: #{name}.nsbmd #{name}.bhc #{name}.3dmd"
		dependfile.puts "	$(BINLINKER) $*.nsbmd $*.nsbtx $*.bhc $*.3dmd $*.#{TARGETEXT} BR"
		dependfile.puts ""
				#高さデータコンバートルール
		dependfile.puts "#{name}.bhc: #{name}h.imd"
		dependfile.puts "	@echo #{name}h > heightlist.lst"
		dependfile.puts "	@echo \\\#END >> heightlist.lst"
		dependfile.puts "	-$(HEIGHTCNV) heightlist.lst < $(HEIGHTRETFILE)"
		dependfile.puts "	ruby pad4byte.rb #{name}h.bhc"
		dependfile.puts ""
				#配置データ生成ルール（今はダミー）
		dependfile.puts "#{name}.3dmd: dummy.3dmd"
		dependfile.puts "	@echo #{name}.3dmdが存在しないためダミーデータを生成します"
		dependfile.puts "	cp dummy.3dmd #{name}.3dmd"

		depends += "	#{name}.#{TARGETEXT} \\\n"

		#heightlist.puts "#{name}h"

		arclist.puts "\"#{name}.#{TARGETEXT}\""
	end
}

dependfile.puts "#{depends}"
dependfile.close

#heightlist.puts "#END"
#heightlist.close

arclist.close


