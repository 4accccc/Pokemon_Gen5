#==============================================================================#
#
#	CodeWarrior for DS用警告フィルタプログラム
#	tamada@gamefreak.co.jp
#	2009.02.12	とりあえず完成
#
#==============================================================================#


#このフィルタでのエラーを定義
class FilterError < StandardError; end


class CWWarningFilter
	def initialize
		@NORMAL_MODE = 0
		@WARN_HEADER_MODE = 1
		@WARN_CONTENT_MODE = 2

		#ここにチェックしたくないエラーを引っ掛ける正規表現を定義
		@ignore_warns = [
			[true, "識別子の再定義", 
				Regexp::new("identifier '[a-zA-Z0-9_]+' redeclared")],
			[true, "暗黙の算術変換", 
				Regexp::new("implicit arithmetic conversion from")],
			[true, "enumへの暗黙の変換",
				Regexp::new("illegal implicit conversion from 'int' to\\s+\#   '\@enum")],
			[true, "暗黙の変換",
				Regexp::new("illegal implicit conversion from ")],
			[true, "未使用の変数/引数",
				Regexp::new(/variable \/ argument '[a-zA-Z0-9_]+' is not used in function/)],
      [false, "対応していないエスケープシーケンス",
        Regexp::new(/unknown escape sequence '/) ]
		]
		@counter = Array.new(@ignore_warns.length, 0)
	end

	def filter warn
		raise FilterError, "警告本体がない" if warn == nil
		result = false
		@ignore_warns.each_with_index{|f, idx|
			if warn =~ f[2] then
				@counter[idx] += 1
				result = true if f[0] == true
			end
		}
		return result
	end

	def put_filter_result
		@counter.each_index{|idx|
			if @counter[idx] != 0 then
				puts "#{@ignore_warns[idx][1]}:#{@counter[idx]}"
			end
		}
	end

	def make_output tfile, sfile, tpos, warn
		if tfile == nil then return end
		raise FilterError, "警告本体がない" if warn == nil
		raise FilterError, "場所がない" if tpos == nil

		if filter(warn) == true then return end
		puts "\nFILE:#{tfile}"
		sfile ||= "itself"
		puts "    :#{sfile}"
		puts "#{tpos}"
		puts "#{warn}"
	end

	def warn_filter file

		tfile = nil	#target file
		sfile = nil	#sub target file
		tpos = nil	#target line position
		warn = nil	#warning explain
		mode = @NORMAL_MODE

		while line = file.gets

			case
			when line =~ /^### mwccarm.exe Compiler:/ then
				make_output(tfile, sfile, tpos, warn)
				mode = @WARN_HEADER_MODE
				tpos = nil
				warn = nil

			when line =~ /^\#    File: (.+)$/ then
				tfile = $1
				sfile = nil

			when line =~ /^\#      In: (.+)$/ then
				sfile = $1

			when line =~ /^\#    From: (.+)$/ then
				tfile = $1
				#sfile = ""

			when line =~ /^\# +\d+: (.+)$/ then
				raise FilterError, "ヘッダ無しで中身がある" if mode != @WARN_HEADER_MODE
				mode = @WARN_CONTENT_MODE
				tpos = line

			when line =~ /^\# Warning:/ then
				raise FilterError, "ヘッダ無しで中身がある" if mode != @WARN_CONTENT_MODE
				tpos += line

			when line =~/^\#/ then
				warn = warn ? warn + line : line

			else
			#if line =~ /^[^#]/ then
				make_output(tfile, sfile, tpos, warn)
				mode = @NORMAL_MODE
				tfile = nil
				sfile = nil
				tpos = nil
				warn = nil
			end

		end
		make_output(tfile, sfile, tpos, warn)
	end

end

###	本体
cwf = CWWarningFilter.new

if ARGV[0] == nil then
	cwf.warn_filter $stdin
	cwf.put_filter_result
else
	File.open(ARGV[0]){|file|
		cwf.warn_filter file
		cwf.put_filter_result
	}
end

