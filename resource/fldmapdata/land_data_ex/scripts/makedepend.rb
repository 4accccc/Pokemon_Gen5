
OUTDIR		= "output"
SRCDIR		= "srcdata"
TARGETEXT	= '3dppack'
DEPENDFILE	= OUTDIR + "/dependfile"
ARCLIST		= OUTDIR + "/arcfilelist"

dependfile = File.open(DEPENDFILE,"w")
arclist	= File.open(ARCLIST,"w")

depends = "\nARCDEPENDS	= \\\n"
File.open(ARGV[0]){|file|

	while line = file.gets
		name = line.chomp
		if name == '' then break end
		if name == '#END' then break end
				#データ連結ルール
		dependfile.puts "#{OUTDIR}/#{name}.#{TARGETEXT}: #{OUTDIR}/#{name}.nsbmd #{OUTDIR}/#{name}.bhc #{SRCDIR}/#{name}.3dmd"
		dependfile.puts "	$(BINLINKER) $*.nsbmd $*.bhc #{OUTDIR}/#{name}.3dmd $*.#{TARGETEXT} BR"
		dependfile.puts ""
				#imd-->nsbmd生成ルール
		dependfile.puts "#{OUTDIR}/#{name}.nsbmd: #{SRCDIR}/#{name}.imd"
		dependfile.puts "	g3dcvtr #{SRCDIR}/#{name}.imd -emdl -o #{OUTDIR}/#{name}.nsbmd"
		dependfile.puts ""
				#ダミー高さデータ生成ルール
		dependfile.puts "#{SRCDIR}/#{name}h.imd:"
		dependfile.puts "	@echo #{name}.imdが存在しないためダミーデータを生成します"
		dependfile.puts "	cp dummy_file/dummy_h.imd #{SRCDIR}/#{name}h.imd"
		dependfile.puts ""
				#高さデータコンバートルール
		dependfile.puts "#{OUTDIR}/#{name}.bhc: #{SRCDIR}/#{name}h.imd"
		dependfile.puts "	@echo #{SRCDIR}/#{name}h > heightlist.lst"
		dependfile.puts "	@echo \\\#END >> heightlist.lst"
		dependfile.puts "	-$(HEIGHTCNV) heightlist.lst < $(HEIGHTRETFILE)"
		dependfile.puts "	ruby scripts/pad4byte.rb #{SRCDIR}/#{name}h.bhc #{OUTDIR}/#{name}.bhc"
		dependfile.puts "	rm #{SRCDIR}/#{name}h.bhc"
		#dependfile.puts "	mv #{SRCDIR}/#{name}.bhc ./#{OUTDIR}"
		dependfile.puts ""
				#配置データ生成ルール（今はダミー）
		dependfile.puts "#{SRCDIR}/#{name}.3dmd:"
		dependfile.puts "	@echo #{name}.3dmdが存在しないためダミーデータを生成します"
		dependfile.puts "	cp dummy_file/dummy.3dmd #{SRCDIR}/#{name}.3dmd"

		depends += "	#{OUTDIR}/#{name}.#{TARGETEXT} \\\n"

		arclist.puts "\"#{OUTDIR}/#{name}.#{TARGETEXT}\""
	end
}

dependfile.puts "#{depends}"
dependfile.close

arclist.close


