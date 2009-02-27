############################################
#
#	Excelからタブ区切りテキスト生成
#
#	2009.02.27	tamada
#	大野さんが使っていたライブラリを流用。
#
#
#	このファイルと同一ディレクトリにライブラリCXls2Xml.rbと
#	それが利用しているJAVAアーカイブxls2xml.jarが必要です。
#
#	使い方：
#	ruby tab_out.rb	(オプション）　ファイル名
#
#	オプション：
#	-t
#	-tab
#		タブ区切り（デフォルトの動作）
#	-c
#	--comma
#		カンマ区切り
#
############################################

#このファイルが置かれている場所にライブラリもあるのでパスを検索
lib_path = File.dirname(__FILE__).gsub(/\\/,"/") + "/CXls2Xml"
require lib_path
#require 'CXls2Xml'

#エラーを定義
class CommnandLineArgumentError < Exception; end

class XlsTextReader < CXls2Xml
	def initialize
		#@xmldata=""
		@sheets ={}
	end
	def getXmlSheet(xlsName)
		@xmldata = xls2xml(xlsName)
		@sheets = xml2sheet(@xmldata)
	end

	def output(sep)
		contents = ""
		@sheets.each{|sheet|
			sheet.each{|row|
				row.each{|col|
					contents += sprintf("%s%s",col, sep)
				}
				contents += sprintf("\n")
			}
		}
		#なぜか最初に空行が入ってしまうので対処。
		#本来的にはちゃんとデバッグする
		return contents.sub(/^\n/,"")
	end
end

#オプションの解釈処理
case ARGV[0]
when "-tab"		then sep = '	'
when "-t"		then sep = '	'
when "-comma"	then sep = ','
when "-c"		then sep = ','
when /\-.+/		then raise CommnandLineArgumentError, "無効なオプションです"
end

#オプションが無いときのデフォルト設定
if sep != nil then
	ARGV.shift
else
	sep = '	'
end

raise CommnandLineArgumentError, "ファイル名を指定してください" if ARGV[0] == nil
raise FileNotFoundException, "ファイル#{ARGV[0]}が見つかりません" unless FileTest.exists?(ARGV[0])

conv = XlsTextReader.new
conv.getXmlSheet(ARGV[0])
puts "#{conv.output(sep)}"

